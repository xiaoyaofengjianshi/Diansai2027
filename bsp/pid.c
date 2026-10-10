#include "pid.h"

void PI_Init(PI_t *p, float kp, float ki, float dt,
             float out_min, float out_max, float integral_limit)
{
    p->Kp = kp;
    p->Ki = ki;
    p->dt = dt;
    p->out_min = out_min;
    p->out_max = out_max;
    p->integral_limit = integral_limit;
    p->error = 0.0f;
    p->integral = 0.0f;
    p->output = 0.0f;
}

void PI_SetParams(PI_t *p, float kp, float ki)
{
    p->Kp = kp;
    p->Ki = ki;
}

void PI_SetLimits(PI_t *p, float out_min, float out_max, float integral_limit)
{
    p->out_min = out_min;
    p->out_max = out_max;
    p->integral_limit = integral_limit;
}

void PI_Reset(PI_t *p)
{
    p->error = 0.0f;
    p->integral = 0.0f;
    p->output = 0.0f;
}

float PI_Update(PI_t *p, float setpoint, float feedback)
{
    float out;

    p->error = setpoint - feedback;

    /* 积分（dt 缩放）+ 抗饱和限幅 */
    p->integral += p->error * p->dt;
    if (p->integral > p->integral_limit) {
        p->integral = p->integral_limit;
    } else if (p->integral < -p->integral_limit) {
        p->integral = -p->integral_limit;
    }

    /* 输出 = 比例 + 积分 */
    out = p->Kp * p->error + p->Ki * p->integral;

    /* 输出限幅 */
    if (out > p->out_max) {
        out = p->out_max;
    } else if (out < p->out_min) {
        out = p->out_min;
    }

    p->output = out;
    return out;
}

float PI_GetOutput(const PI_t *p)
{
    return p->output;
}
