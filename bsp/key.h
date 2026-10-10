/* 迁移时本文件没有需要更改的部分，.h文件也放这里是看着方便 */
#ifndef __KEY_H
#define __KEY_H

#include "main.h" 

#define KEY_COUNT               4

#define KEY1_INDEX              0  
#define KEY2_INDEX              1  
#define KEY3_INDEX              2  
#define KEY4_INDEX              3 

#define KEY_HOLD                0x01
#define KEY_DOWN                0x02
#define KEY_UP                  0x04
#define KEY_SINGLE              0x08
#define KEY_DOUBLE              0x10
#define KEY_LONG                0x20
#define KEY_REPEAT              0x40

void Key_Tick(void);
uint8_t Key_Check(uint8_t n, uint8_t Flag);

#endif
