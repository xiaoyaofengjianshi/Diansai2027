#include "selfadc.h"

uint16_t values[ADC_CHANNEL_NUM];
float voltages[ADC_CHANNEL_NUM];

//  更新实际电压值
static float Convert_Single_Channel(uint16_t adc_raw, uint8_t ch_index)
{

    static uint16_t adc_buffers[ADC_CHANNEL_NUM][FILTER_WINDOW_SIZE] = {0};
    static int buffer_indices[ADC_CHANNEL_NUM] = {0};
    static uint32_t adc_sums[ADC_CHANNEL_NUM] = {0};
    static int data_counts[ADC_CHANNEL_NUM] = {0};
    

    static uint8_t step_counters[ADC_CHANNEL_NUM] = {0};


    if (data_counts[ch_index] == 0) 
    {
        for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
            adc_buffers[ch_index][i] = adc_raw;
        }
        adc_sums[ch_index] = (uint32_t)adc_raw * FILTER_WINDOW_SIZE;
        data_counts[ch_index] = FILTER_WINDOW_SIZE;
        buffer_indices[ch_index] = 0;
        step_counters[ch_index] = 0;
    }

    else 
    {
        uint16_t current_avg = adc_sums[ch_index] / data_counts[ch_index];
        int diff = (int)adc_raw - (int)current_avg;
        if (diff < 0) diff = -diff;

        if (diff > STEP_THRESHOLD) 
        {
            step_counters[ch_index]++;
            
            if (step_counters[ch_index] >= STEP_CONFIRM_CNT) 
            {
                for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
                    adc_buffers[ch_index][i] = adc_raw;
                }
                adc_sums[ch_index] = (uint32_t)adc_raw * FILTER_WINDOW_SIZE;
                buffer_indices[ch_index] = 0;
                step_counters[ch_index] = 0;
            }
        } 
        else 
        {
            step_counters[ch_index] = 0; 
        }

        int idx = buffer_indices[ch_index];
        adc_sums[ch_index] -= adc_buffers[ch_index][idx];
        adc_buffers[ch_index][idx] = adc_raw;
        adc_sums[ch_index] += adc_raw;
        buffer_indices[ch_index] = (idx + 1) % FILTER_WINDOW_SIZE;
    }

    float adc_average = (float)adc_sums[ch_index] / (float)data_counts[ch_index];

    static const float k_table[ADC_CHANNEL_NUM] = {
        0.0279f, 
        0.0277f, 
        0.0272f, 
        0.01802952f, 
        0.01802952f,
        0.01802952f, 
        0.01802952f, 
        0.01802952f 
    };

    static const float b_table[ADC_CHANNEL_NUM] = {
        -45.27f,   
        -44.7752f,  
        -44.477f,  
        -29.0227f,  
        -29.0227f, 
        -29.0227f, 
        -29.0227f,  
        -29.0227f    
    };


    float voltage = (k_table[ch_index] * adc_average) + b_table[ch_index];
    if (voltage < 0.0f) {
        voltage = 0.0f;
    }

    return voltage;
}


void Update_All_Voltages(uint16_t *adc_array, float *volt_array)
{
    if (adc_array == NULL || volt_array == NULL) {
        return;
    }


    for (uint8_t i = 0; i < ADC_CHANNEL_NUM; i++)
    {
        volt_array[i] = Convert_Single_Channel(adc_array[i], i);
    }
}

float ADC_GetVoltage(uint8_t ch)
{
    if (ch >= ADC_CHANNEL_NUM) {
        return 0.0f;
    }
    return voltages[ch];
}
