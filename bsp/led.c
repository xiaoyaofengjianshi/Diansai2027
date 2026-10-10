#include "led.h"

void LED_ON(uint8_t num){
	if(num == 1){
		HAL_GPIO_WritePin(LED1,led_on);
	}
	else if(num == 2){
		HAL_GPIO_WritePin(LED2,led_on);
	}
	else if(num == 3){
		HAL_GPIO_WritePin(LED3,led_on);
	}
	else if(num == 4){
		HAL_GPIO_WritePin(LED4,led_on);
	}
}

void LED_ON_All(void){
	HAL_GPIO_WritePin(LED4,led_on);
	HAL_GPIO_WritePin(LED3,led_on);
	HAL_GPIO_WritePin(LED2,led_on);
	HAL_GPIO_WritePin(LED1,led_on);
}

void LED_OFF_All(void){
	HAL_GPIO_WritePin(LED4,led_off);
	HAL_GPIO_WritePin(LED3,led_off);
	HAL_GPIO_WritePin(LED2,led_off);
	HAL_GPIO_WritePin(LED1,led_off);
}

void LED_OFF(uint8_t num){
	if(num == 1){
		HAL_GPIO_WritePin(LED1,led_off);
	}
	else if(num == 2){
		HAL_GPIO_WritePin(LED2,led_off);
	}
	else if(num == 3){
		HAL_GPIO_WritePin(LED3,led_off);
	}
	else if(num == 4){
		HAL_GPIO_WritePin(LED4,led_off);
	}
}
