#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "stm32f10x.h" 
#include "bsp_utils.h"

//private functions
void Keyboard_setRow(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4); //BSP Function
uint8_t Keyboard_readColumn(uint8_t column); //BSP Function

//public functions
void Keyboard_init(void); //BSP Function 初始化矩阵键盘
uint8_t Keyboard_scan(void); //扫描矩阵键盘并返回结果

#endif
