/* 按键实现 */
#include "Key.h"

#define KEY_PRESSED             1
#define KEY_UNPRESSED           0

#define KEY_TIME_DOUBLE         10
#define KEY_TIME_LONG           100
#define KEY_TIME_REPEAT         5

uint8_t Key_Flag[KEY_COUNT];

/* 使用时更改此处引脚即可 */
uint8_t Key_GetState(uint8_t n)
{
    GPIO_PinState state = GPIO_PIN_SET;
    
    switch(n) {
        case KEY1_INDEX: state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0); break;
        case KEY2_INDEX: state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1); break;
        case KEY3_INDEX: state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2); break;
        case KEY4_INDEX: state = HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_10); break;
    }
    
    return (state == GPIO_PIN_RESET) ? KEY_PRESSED : KEY_UNPRESSED;
}


uint8_t Key_Check(uint8_t n, uint8_t Flag)
{
	if (Key_Flag[n] & Flag)
	{
		if (Flag != KEY_HOLD)
		{
			Key_Flag[n] &= ~Flag;
		}
		return 1;
	}
	return 0;
}

void Key_Tick(void)
{
    static uint8_t Count, i;
    static uint8_t CurrState[KEY_COUNT], PrevState[KEY_COUNT];
    static uint8_t S[KEY_COUNT];
    static uint16_t Time[KEY_COUNT];
    
    
    Count ++;
    if (Count >= 20) 
    {
        Count = 0;
        
        for (i = 0; i < KEY_COUNT; i ++)
        {
            if (Time[i] > 0)
            {
                Time[i] --;
            }

            PrevState[i] = CurrState[i];
            CurrState[i] = Key_GetState(i);
            
            if (CurrState[i] == KEY_PRESSED) { Key_Flag[i] |= KEY_HOLD; }
            else                             { Key_Flag[i] &= ~KEY_HOLD; }
            
            if (CurrState[i] == KEY_PRESSED && PrevState[i] == KEY_UNPRESSED)  { Key_Flag[i] |= KEY_DOWN; }
            if (CurrState[i] == KEY_UNPRESSED && PrevState[i] == KEY_PRESSED)  { Key_Flag[i] |= KEY_UP; }
            
            if (S[i] == 0)
            {
                if (CurrState[i] == KEY_PRESSED)
                {
                    Time[i] = KEY_TIME_LONG;
                    S[i] = 1;
                }
            }
            else if (S[i] == 1)
            {
                if (CurrState[i] == KEY_UNPRESSED)
                {
                    Time[i] = KEY_TIME_DOUBLE;
                    S[i] = 2;
                }
                else if (Time[i] == 0)
                {
                    Time[i] = KEY_TIME_REPEAT;
                    Key_Flag[i] |= KEY_LONG;
                    S[i] = 4;
                }
            }
            else if (S[i] == 2)
            {
                if (CurrState[i] == KEY_PRESSED)
                {
                    Key_Flag[i] |= KEY_DOUBLE;
                    S[i] = 3;
                }
                else if (Time[i] == 0)
                {
                    Key_Flag[i] |= KEY_SINGLE;
                    S[i] = 0;
                }
            }
            else if (S[i] == 3)
            {
                if (CurrState[i] == KEY_UNPRESSED)
                {
                    Time[i] = 0; 
                    S[i] = 0;
                }
            }
            else if (S[i] == 4)
            {
                if (CurrState[i] == KEY_UNPRESSED)
                {
                    Time[i] = 0; 
                    S[i] = 0;
                }
                else if (Time[i] == 0)
                {
                    Time[i] = KEY_TIME_REPEAT;
                    Key_Flag[i] |= KEY_REPEAT;
                    S[i] = 4;
                }
            }
        }
    }
}
