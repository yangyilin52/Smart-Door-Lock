#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "stm32f10x.h"

typedef enum{
    Led_Color_Red,
    Led_Color_Green,
    Led_Color_Blue,
    Led_Color_Purple,
    Led_Color_SkyBlue,
    Led_Color_Yellow,
    Led_Color_White,
    Led_Color_None
}Led_Color;

//private functions
void Led_pinCtrl(uint8_t Red, uint8_t Green, uint8_t Blue); //BSP Function

//public functions
void Led_init(void); //BSP Function 初始化LED
void Led_setColor(Led_Color color); //设置LED亮灯颜色
void Led_setBlink(uint8_t enable, uint16_t lightTimeMs, uint16_t restTimeMs); //BSP Function 设置LED闪烁规则


#endif
