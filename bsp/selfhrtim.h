#ifndef __SELFHRTIM_H
#define __SELFHRTIM_H

#include "main.h"

// PWM编号
typedef enum {
    PWM_TIMER_A = 0,
    PWM_TIMER_B = 1,
    PWM_TIMER_C = 2,
    PWM_TIMER_D = 3,
} PWM_Timer_t;

void HRTIM_PWM_Init(void);                          // 启动 HRTIM
void HRTIM_SetDuty(PWM_Timer_t timer, float duty);  // 设占空比 0~1

#endif /* __SELFHRTIM_H */
