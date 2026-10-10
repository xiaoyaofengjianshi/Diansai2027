#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"
#include "pid.h"
#include "selfhrtim.h"

/* ============================================================================
 *  控制框架（底层）—— 与具体拓扑无关
 *  ---------------------------------------------------------------------------
 *  它只知道"有一组拓扑对象"，不知道它们是什么拓扑、接在哪几个引脚上。
 *  要换拓扑（buck-boost -> boost / buck / 整流），改的是装配层
 *  （buckboost.c 那一类文件），本文件一行都不用动。
 * ==========================================================================*/

/* 控制周期：必须与实际调用 Topology_Update 的周期一致（当前是 TIM20 的 1ms 节拍）。
 * PI 的积分项按 dt 缩放，改了调用频率就要同步改这里。 */
#define CTRL_DT     0.001f

/* 占空比窗口
 * 下限 0.05：HRTIM 的 Set 源是 MASTERPER、Reset 源是 TIMCMP1，占空比就是"高边导通时间"。
 *   D 趋近 0 会让 Set/Reset 两个边沿重合、被死区单元(300 tick 约 220ns)吞掉；
 *   这个窗口同时是自举电容的充电时间。
 * 上限 0.95：buck-boost 的 Vout/Vin = D/(1-D)，增益在 D->1 时爆炸
 *   (dG/dD = 1/(1-D)^2，D=0.95 时已是 400)，而且低边只剩 2us 给自举充电。
 *   若上电后发现高边在高占空比时掉驱动，把上限降到 0.90。
 */
#define DUTY_MIN    0.05f
#define DUTY_MAX    0.95f

/* 按键单步调整量 */
#define DUTY_STEP   0.02f

typedef enum {
    CTRL_OPEN   = 0,   /* 开环：固定占空比，不读反馈 */
    CTRL_CLOSED = 1,   /* 闭环：PI 电压环 */
} Ctrl_Mode_t;

/* 一个 Topology_t = 一路独立变换器
 *   = 一个 HRTIM 定时器 + 一个 ADC 反馈通道 + 一个模式。
 *
 * buck / boost / 反相或非反相 buck-boost 的控制律完全相同（都是
 * "读反馈 -> 算占空比 -> 写 HRTIM"），所以它们共用这一个类型。
 * 将来要加"参考值随时间变化"的拓扑（逆变器需要正弦参考）时，
 * 在 Topology_Update 里加一个参考生成钩子即可，不要上函数指针虚表。
 */
typedef struct {
    const char  *name;      /* 显示用，1~2 个字符 */
    PWM_Timer_t  pwm;       /* 该路占用的 HRTIM 定时器 */
    uint8_t      ch_fb;     /* 反馈电压的 ADC 通道号 0..7 */
    Ctrl_Mode_t  mode;      /* 开环 / 闭环 */

    float duty_open;        /* 开环占空比，按键可调，恒在 [DUTY_MIN, DUTY_MAX] */
    float duty;             /* 本拍实际写下去的占空比（只读） */
    float target;           /* 闭环设定值 [V] */
    float v_fb;             /* 最近一次反馈电压 [V]（只读） */

    PI_t  pi;
} Topology_t;

/* ---------------------------- 拓扑对象 ---------------------------- */

void Topology_Init(Topology_t *t, const char *name, PWM_Timer_t pwm,
                   uint8_t ch_fb, Ctrl_Mode_t mode,
                   float duty_open, float target, float kp, float ki);

/* 每个控制周期调用一次（当前 1ms）：读反馈 -> 按模式算占空比 -> 写 HRTIM */
void Topology_Update(Topology_t *t);

void Topology_SetMode(Topology_t *t, Ctrl_Mode_t mode);
void Topology_SetOpenDuty(Topology_t *t, float duty);   /* 内部限幅 */
void Topology_SetTarget(Topology_t *t, float target);

/* ------------------------------ 框架 ------------------------------ */

/* 装配层调用：把这一版的拓扑对象表交给框架。
 * 必须在 Control_Init() 之前调用。list 必须是静态存储期的数组
 * （框架只保存指针，不复制内容）。 */
void Control_Bind(Topology_t *list, uint8_t count);

void Control_Init(void);      /* 启动 HRTIM，把所有已注册的路置到安全占空比 */
void Control_Update(void);    /* 1ms 节拍调用：按键 + 遍历更新 + 遥测 */
void Control_Display(void);   /* 主循环调用：OLED。绝不能在中断里调（阻塞式 I2C） */

/* 工具：把某一路的开环占空比加减 delta（内部限幅） */
void Control_Nudge(Topology_t *t, float delta);

#endif /* __CONTROL_H */
