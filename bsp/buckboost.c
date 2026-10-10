#include "buckboost.h"
#include "control.h"

/* ============================================================================
 *  本版装配
 *
 *  硬件：HRTIM Timer A 和 Timer B 各接一套**独立**的 buck-boost 半桥，
 *  不是同一个四开关变换器的两个桥臂。所以：
 *      topos[0]  接 Timer A，开环，用 KEY1/KEY2 调占空比
 *      topos[1]  接 Timer B，闭环，PI 把反馈稳到 B_TARGET
 *  两路同时运行、互不干扰。
 * ==========================================================================*/

#define TOPO_NUM    2

#define A_INDEX     0
#define B_INDEX     1

#define A_CH_FB     0       /* 开环路不闭环，这个通道只用于显示和遥测 */
#define B_CH_FB     0       /* 闭环反馈通道：必须与实际接线一致 */
#define B_TARGET    5.0f
#define B_KP        0.05f
#define B_KI        0.5f

/* 必须是静态存储期：Control_Bind() 只保存指针，不复制内容 */
static Topology_t topos[TOPO_NUM];

void BuckBoost_Build(void)
{
    Topology_Init(&topos[A_INDEX], "A", PWM_TIMER_A, A_CH_FB, CTRL_OPEN,
                  DUTY_MIN, 0.0f, 0.0f, 0.0f);

    Topology_Init(&topos[B_INDEX], "B", PWM_TIMER_B, B_CH_FB, CTRL_CLOSED,
                  0.0f, B_TARGET, B_KP, B_KI);

    Control_Bind(topos, TOPO_NUM);
}
