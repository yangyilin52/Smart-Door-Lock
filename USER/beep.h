#ifndef BEEP_H
#define BEEP_H

#include <stdint.h>
#include "stm32f10x.h"

//Public Functions
void Beep_init(void); //BSP Function 初始化蜂鸣器
void Beep_setEnable(uint8_t enable); //BSP Function 设置蜂鸣器是否工作
void Beep_setFrequency(uint16_t frequency); //BSP Function 设置蜂鸣器的声音频率

#endif
