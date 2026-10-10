#include "vofa.h"
#include "usart.h"          
#include <string.h>        

/* JustFloat 协议：每个通道 4 字节小端 float，帧尾 0x00 0x00 0x80 0x7F */
#define VOFA_MAX_CH       8
#define VOFA_TAIL_BYTES   4

static uint8_t vofa_tx_buf[VOFA_MAX_CH * 4 + VOFA_TAIL_BYTES];

void VOFA_Send_Floats(float *data, uint16_t channel_count)
{
    uint16_t i;
    uint16_t len;

    if (data == NULL || channel_count == 0 || channel_count > VOFA_MAX_CH) {
        return;
    }

    /* 上一帧还没发完就直接丢了不要了 */
    if (huart5.gState != HAL_UART_STATE_READY) {
        return;
    }

    len = channel_count * 4;
    for (i = 0; i < channel_count; i++) {
        memcpy(&vofa_tx_buf[i * 4], &data[i], 4);
    }
    vofa_tx_buf[len + 0] = 0x00;
    vofa_tx_buf[len + 1] = 0x00;
    vofa_tx_buf[len + 2] = 0x80;
    vofa_tx_buf[len + 3] = 0x7F;

    HAL_UART_Transmit_IT(&huart5, vofa_tx_buf, len + VOFA_TAIL_BYTES);
}
