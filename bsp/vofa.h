#ifndef __VOFA_H
#define __VOFA_H

#include "main.h"

/* 目前只发前两个去vofa */
#define VOFA_CH_NUM   2

void VOFA_Send_Floats(float *data, uint16_t channel_count);

#endif /* __VOFA_H */
