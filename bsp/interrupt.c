#include "interrupt.h"
#include "main.h"
#include "selfadc.h"
#include "control.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM20)   /* 1ms */
    {
        /* 反馈必须在这个节拍里刷新。voltages[] 以前只在主循环里更新，而主循环
         * 被阻塞式 OLED 拖到几十 Hz，控制环读到的是几十毫秒前的值，而且刷新率
         * 随 OLED 的负载抖动。搬到这里之后，滤波窗口变成稳定可预期的 10ms。 */
        Update_All_Voltages(values, voltages);

        Key_Tick();
        Control_Update();          /* 内含按键 + 两路拓扑 + 遥测 */
    }
}
