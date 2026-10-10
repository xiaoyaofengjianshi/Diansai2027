#ifndef __SELFADC_H
#define __SELFADC_H

#include "main.h"



#define ADC_CHANNEL_NUM      8
#define FILTER_WINDOW_SIZE   10  

#define STEP_THRESHOLD       28  
#define STEP_CONFIRM_CNT     2   


extern uint16_t values[ADC_CHANNEL_NUM];
extern float voltages[ADC_CHANNEL_NUM];

void Update_All_Voltages(uint16_t *adc_array, float *volt_array);
float ADC_GetVoltage(uint8_t ch);   /* 取通道 ch 换算后的电压 */

#endif /* __SELFADC_H */
