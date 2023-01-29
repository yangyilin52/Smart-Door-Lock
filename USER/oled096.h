#ifndef OLED096_H
#define OLED096_H

#include <stdint.h>
#include "stm32f10x.h"
#include "bsp_utils.h"

typedef enum{
    OLED096_WriteAction_Data,
    OLED096_WriteAction_Cmd
}OLED096_WriteAction;

typedef enum{
    OLED096_Pin_NSS,
    OLED096_Pin_DC,
    OLED096_Pin_RST
}OLED096_Pin;

typedef enum{
    OLED096_Align_Left,
    OLED096_Align_Center,
    OLED096_Align_Right
}OLED096_Align;

//private functions
void OLED096_pinCtrl(OLED096_Pin pin, uint8_t status); //BSP Function
void OLED096_hwInit(void); //BSP Function
uint8_t OLED096_SPIWriteReadByte(uint8_t txData); //BSP Function
void OLED096_writeByte(uint8_t data, OLED096_WriteAction action);

//public functions
void OLED096_reset(void); //重置OLED
void OLED096_init(void); //初始化OLED
void OLED096_clearScreen(void); //OLED清屏
void OLED096_writeLine(uint8_t row, OLED096_Align align, uint8_t reverseColor, char* str); //OLED写一行内容
void OLED096_writeScreen(char* str); //OLED整屏写内容
void OLED096_playTestAnimateOnce(void); //OLED播放动画（用于测试）
void OLED096_setContrast(uint8_t contrast); //设置OLED对比度

#endif
