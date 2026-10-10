#include "control.h"
#include "selfadc.h"
#include "key.h"
#include "vofa.h"
#include "OLED.h"

/* ============================================================================
 *  控制框架（底层）—— 与具体拓扑无关
 *
 *  本文件里不应该出现任何具体拓扑的字眼（buck-boost / Timer A / 目标 5V ...）。
 *  这一版用哪几路、什么参数，全部由装配层（buckboost.c）通过 Control_Bind()
 *  交进来。换拓扑时本文件一行都不用改。
 * ==========================================================================*/

/* ------------------------- 拓扑对象 ------------------------- */

/*
 * 积分限幅的取值原则：让 Ki * integral_limit 大致等于占空比窗口的宽度
 * (DUTY_MAX - DUTY_MIN = 0.9)。
 * 取大了（例如 Ki*limit = 1.0 > 0.9）意味着积分项自己就能顶穿整个窗口，
 * 一旦饱和要退很久，恢复时会明显过冲。这里按 Ki 反算，改 Ki 时不必再手调。
 */
#define INT_LIMIT_SPAN   0.9f

/* 前置声明：下面几个 static 函数的定义在 Control_Update 之后 */
static void Control_KeyHandler(void);
static void Control_Telemetry(void);

static float ClampDuty(float d)
{
    if (d < DUTY_MIN) return DUTY_MIN;
    if (d > DUTY_MAX) return DUTY_MAX;
    return d;
}

void Topology_Init(Topology_t *t, const char *name, PWM_Timer_t pwm,
                   uint8_t ch_fb, Ctrl_Mode_t mode,
                   float duty_open, float target, float kp, float ki)
{
    float int_limit;

    t->name      = name;
    t->pwm       = pwm;
    t->ch_fb     = ch_fb;
    t->mode      = mode;
    t->duty_open = ClampDuty(duty_open);
    t->target    = target;
    t->duty      = t->duty_open;
    t->v_fb      = 0.0f;

    /* Ki 为 0 时给个安全值，避免除零 */
    int_limit = (ki > 0.0f) ? (INT_LIMIT_SPAN / ki) : 1.0f;

    /* PI 的输出直接就是占空比，所以限幅用占空比窗口 */
    PI_Init(&t->pi, kp, ki, CTRL_DT, DUTY_MIN, DUTY_MAX, int_limit);
}

void Topology_Update(Topology_t *t)
{
    t->v_fb = ADC_GetVoltage(t->ch_fb);

    if (t->mode == CTRL_CLOSED) {
        t->duty = PI_Update(&t->pi, t->target, t->v_fb);
    } else {
        t->duty = t->duty_open;
    }

    /* 兜底：无论走哪条路径，都绝不把 0 或 1 写下去 */
    t->duty = ClampDuty(t->duty);

    HRTIM_SetDuty(t->pwm, t->duty);
}

void Topology_SetMode(Topology_t *t, Ctrl_Mode_t mode)
{
    if (mode == t->mode) {
        return;
    }

    if (mode == CTRL_CLOSED) {
        /* 清掉残留积分，否则切进来的第一拍会带着上一次闭环的历史，占空比跳变 */
        PI_Reset(&t->pi);
    }

    t->mode = mode;
}

void Topology_SetOpenDuty(Topology_t *t, float duty)
{
    t->duty_open = ClampDuty(duty);
}

void Topology_SetTarget(Topology_t *t, float target)
{
    t->target = target;
}

/* ---------------------------- 框架 ---------------------------- */

/* 装配层交进来的对象表。只保存指针，不复制。 */
static Topology_t *g_list;
static uint8_t     g_count;
static uint8_t     ready;   /* 节拍可能早于 Control_Init，用它挡住 */

void Control_Bind(Topology_t *list, uint8_t count)
{
    if (list == NULL || count == 0) {
        return;
    }
    g_list  = list;
    g_count = count;
}

void Control_Init(void)
{
    uint8_t i;

    /* 启动 HRTIM：C/D 固定 60%，A/B/C/D 计数器和 8 路输出一起开 */
    HRTIM_PWM_Init();

    /* .ioc 里 TA/TB 的 CompareValue 初值是 36.8%，必须立刻覆盖成安全占空比。
     * 如果烧录后在示波器上看到 36.8%，说明这里没生效。 */
    for (i = 0; i < g_count; i++) {
        HRTIM_SetDuty(g_list[i].pwm, DUTY_MIN);
    }

    ready = 1;
}

void Control_Update(void)   /* 1ms 节拍 */
{
    uint8_t i;

    if (!ready) {
        return;
    }

    Control_KeyHandler();

    for (i = 0; i < g_count; i++) {
        Topology_Update(&g_list[i]);
    }

    Control_Telemetry();
}

void Control_Nudge(Topology_t *t, float delta)
{
    Topology_SetOpenDuty(t, t->duty_open + delta);   /* 内部限幅 */
}

/* ---------------------------- 内部 ---------------------------- */

/*
 * 默认操作约定：KEY1/KEY2 调第 0 路的开环占空比，KEY3 切第 0 路的模式。
 *
 * 这是一套"够用"的约定，不是通用规律 —— 将来某一版按键含义不同
 * （比如整流要调 PI 参数），改这一段即可，不影响上面任何一行。
 *
 * Key_Check 读取即清标志。绝不能写成 if (A || B)：|| 短路会跳过第二个检查，
 * 那个标志不会被清掉，下一拍会补一次误触发。必须一条 if 一个标志。
 *
 * 用 KEY_DOWN（按下瞬间，20ms 延迟）和 KEY_REPEAT（长按 2s 后 10Hz 连发）：
 * key.c 的状态机每 20ms 才跑一次，所以 KEY_SINGLE 有约 200ms 延迟、
 * KEY_LONG 要按满 2 秒，都不适合连续调占空比。
 * KEY3 用 KEY_SINGLE 是**刻意不对称**的（模式切换很少发生，200ms 不可见），别顺手统一。
 * KEY4 是 PG10 = NRST 引脚，不用。
 */
static void Control_KeyHandler(void)
{
    if (g_count == 0) {
        return;
    }

    if (Key_Check(KEY1_INDEX, KEY_DOWN))   { Control_Nudge(&g_list[0], +DUTY_STEP); }
    if (Key_Check(KEY1_INDEX, KEY_REPEAT)) { Control_Nudge(&g_list[0], +DUTY_STEP); }

    if (Key_Check(KEY2_INDEX, KEY_DOWN))   { Control_Nudge(&g_list[0], -DUTY_STEP); }
    if (Key_Check(KEY2_INDEX, KEY_REPEAT)) { Control_Nudge(&g_list[0], -DUTY_STEP); }

    if (Key_Check(KEY3_INDEX, KEY_SINGLE)) {
        Topology_SetMode(&g_list[0],
            (g_list[0].mode == CTRL_OPEN) ? CTRL_CLOSED : CTRL_OPEN);
    }
}

/*
 * 遥测：每路发 2 个通道（反馈电压、实际占空比），最多发前 TELEM_TOPO 路。
 *
 * 115200 波特率下 8N1 每字节约 87us。4 通道一帧 20 字节 = 1.74ms，
 * 比 1ms 的节拍还长 —— 1kHz 发 4 通道物理上不可能（vofa.c 会直接丢帧）。
 * 所以抽到 100Hz，占用率约 17%。想多发几路就得先提波特率。
 */
#define TELEM_DIV   10      /* 1ms / 10 = 100Hz */
#define TELEM_TOPO  2       /* 最多 2 路 = 4 个通道 */

static void Control_Telemetry(void)
{
    static float   buf[TELEM_TOPO * 2];
    static uint8_t div;
    uint8_t i, n;

    if (++div < TELEM_DIV) {
        return;
    }
    div = 0;

    n = (g_count < TELEM_TOPO) ? g_count : TELEM_TOPO;
    if (n == 0) {
        return;
    }

    for (i = 0; i < n; i++) {
        buf[i * 2 + 0] = g_list[i].v_fb;
        buf[i * 2 + 1] = g_list[i].duty;
    }

    VOFA_Send_Floats(buf, (uint16_t)(n * 2));
}

/* ---------------------------- 显示 ---------------------------- */

/*
 * 主循环调用。OLED 驱动是无缓冲、逐字节阻塞的 I2C 写（一个字符约 2ms），
 * 所以绝不能放进中断。用 OLED_ShowNum 配整数缩放，避免 %f
 * —— 浮点 printf 会让固件膨胀 10~20KB，还可能因 MicroLIB 关掉而链接失败。
 *
 * 每路占一行（OLED 支持 4 行），布局：
 *   A OP D29 V050      <- 名字 / 模式 / 占空比% / 电压(×10)
 */
void Control_Display(void)
{
    uint8_t i;

    for (i = 0; i < g_count && i < 4; i++) {
        OLED_ShowString(i + 1, 1, (char *)g_list[i].name);
        OLED_ShowString(i + 1, 3, (g_list[i].mode == CTRL_OPEN) ? "OP" : "CL");
        OLED_ShowString(i + 1, 6, "D");
        OLED_ShowNum   (i + 1, 7, (uint32_t)(g_list[i].duty * 100.0f), 2);
        OLED_ShowString(i + 1, 10, "V");
        OLED_ShowNum   (i + 1, 11, (uint32_t)(g_list[i].v_fb * 10.0f), 3);
    }
}
