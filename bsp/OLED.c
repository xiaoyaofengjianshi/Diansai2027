
#include "OLED.h"
#include "OLED_Font.h"
#include "i2c.h" 

/* OLED 的 I2C 地址，通常为 0x78 (0x3C << 1) */
#define OLED_ADDRESS 0x78

/**
  * @brief  向 OLED 写入一个命令
  */
void OLED_WriteCommand(uint8_t Command)
{
    // 使用 HAL 硬件 I2C 写入：0x00 表示后面紧跟的是命令
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, &Command, 1, HAL_MAX_DELAY);
}

/**
  * @brief  向 OLED 写入一个数据
  */
void OLED_WriteData(uint8_t Data)
{
    // 使用 HAL 硬件 I2C 写入：0x40 表示后面紧跟的是数据
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDRESS, 0x40, I2C_MEMADD_SIZE_8BIT, &Data, 1, HAL_MAX_DELAY);
}

/**
  * @brief  设置光标位置
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                    // 设置页地址 (0~7)
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));    // 设置列高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));           // 设置列低4位
}

/**
  * @brief  清屏
  */
void OLED_Clear(void)
{  
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for(i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

/**
  * @brief  显示字符
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
    }
}

/**
  * @brief  显示字符串
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/* --- 辅助数学函数 --- */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--) Result *= X;
    return Result;
}

/* --- 数字显示系列函数 (逻辑保持不变) --- */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)							
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint32_t Number1;
    if (Number >= 0) { OLED_ShowChar(Line, Column, '+'); Number1 = Number; }
    else { OLED_ShowChar(Line, Column, '-'); Number1 = -Number; }
    for (uint8_t i = 0; i < Length; i++)							
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)							
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10) OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        else OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
    }
}

void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    for (uint8_t i = 0; i < Length; i++)							
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

/**
  * @brief  OLED初始化
  */
void OLED_Init(void)
{
    HAL_Delay(100); // 上电延时，使用 HAL 库标准毫秒延时

    // SSD1306 初始化序列
    OLED_WriteCommand(0xAE); // 关闭显示
    OLED_WriteCommand(0xD5); OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3); OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0xA1); // 左右方向
    OLED_WriteCommand(0xC8); // 上下方向
    OLED_WriteCommand(0xDA); OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9); OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB); OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0x8D); OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF); // 开启显示
		
    OLED_Clear(); 
}
