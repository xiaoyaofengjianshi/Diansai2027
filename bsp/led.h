#ifndef __LED_H_
#define __LED_H_

#include "main.h"

#define LED1 GPIOC,GPIO_PIN_15
#define LED2 GPIOC,GPIO_PIN_14
#define LED3 GPIOC,GPIO_PIN_13
#define LED4 GPIOC,GPIO_PIN_12
#define led_on GPIO_PIN_SET
#define led_off GPIO_PIN_RESET

void LED_ON(uint8_t num);
void LED_ON_All(void);
void LED_OFF_All(void);
void LED_OFF(uint8_t num);

#endif
