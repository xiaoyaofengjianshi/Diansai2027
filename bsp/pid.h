#ifndef __PID_H
#define __PID_H

#include "main.h"

/* 
 * 这部分大部分为AI参与，我微积分60分TAT
 * 通用 PI 控制器。
 * 一个 PI_t 实例 = 一个独立控制器，可任意创建多个（电流环/电压环/每相一个…）。
 * 积分项按采样周期 dt 缩放，同一组 Kp/Ki 在不同调用频率下表现一致。
 */
typedef struct {
    float Kp;              /* 比例系数 */
    float Ki;              /* 积分系数 */
    float dt;              /* 采样周期 */

    float out_min;         /* 输出下限 */
    float out_max;         /* 输出上限 */
    float integral_limit;  /* 积分限幅，取正值 */

    /* 内部状态 */
    float error;           /* 最近一次误差 */
    float integral;        /* 积分累计 */
    float output;          /* 最近一次输出 */
} PI_t;

void  PI_Init(PI_t *p, float kp, float ki, float dt,
              float out_min, float out_max, float integral_limit);
void  PI_SetParams(PI_t *p, float kp, float ki);
void  PI_SetLimits(PI_t *p, float out_min, float out_max, float integral_limit);
void  PI_Reset(PI_t *p);
float PI_Update(PI_t *p, float setpoint, float feedback);
float PI_GetOutput(const PI_t *p);

#endif /* __PID_H */
