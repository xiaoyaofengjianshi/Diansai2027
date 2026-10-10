#include "selfhrtim.h"
#include "hrtim.h"      

#define HRTIM_PERIOD     54400.0f   // cubemx-period 
#define CD_FIXED_DUTY    0.60f      // CD固定 60% 占空比 

/* PWM_Timer_t → HRTIM 定时器索引 的映射 */
static const uint32_t timer_index[] = {
    HRTIM_TIMERINDEX_TIMER_A,
    HRTIM_TIMERINDEX_TIMER_B,
    HRTIM_TIMERINDEX_TIMER_C,
    HRTIM_TIMERINDEX_TIMER_D,
};

void HRTIM_PWM_Init(void)
{
    /* C/D 固定 60% 占空比 */
    HRTIM_SetDuty(PWM_TIMER_C, CD_FIXED_DUTY);
    HRTIM_SetDuty(PWM_TIMER_D, CD_FIXED_DUTY);

    /* 启动主计数器 + A/B/C/D 计数器 */
    HAL_HRTIM_WaveformCountStart(&hhrtim1,
        HRTIM_TIMERID_MASTER  |
        HRTIM_TIMERID_TIMER_A |
        HRTIM_TIMERID_TIMER_B |
        HRTIM_TIMERID_TIMER_C |
        HRTIM_TIMERID_TIMER_D);

    /* 使能输出：每个半桥上下管都开，死区自动生成互补信号 */
    HAL_HRTIM_WaveformOutputStart(&hhrtim1,
        HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
        HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2 |
        HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2 |
        HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
}

void HRTIM_SetDuty(PWM_Timer_t timer, float duty)
{
    uint32_t cmp;

    if (timer > PWM_TIMER_D) {
        return;
    }
    if (duty < 0.0f) {
        duty = 0.0f;
    }
    if (duty > 1.0f) {
        duty = 1.0f;
    }

    cmp = (uint32_t)(duty * HRTIM_PERIOD);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, timer_index[timer], HRTIM_COMPAREUNIT_1, cmp);
}
