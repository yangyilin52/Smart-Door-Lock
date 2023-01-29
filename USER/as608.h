#ifndef AS608_H
#define AS608_H

#include <stdint.h>
#include "stm32f10x.h"
#include "bsp_utils.h"

//Private Functions
void AS608_sendData(uint8_t* data, uint8_t length);
void AS608_clearRecvBuff(void);

//Public Functions
void AS608_init(void); //初始化AS608
uint8_t AS608_isFingerAvailable(void); //通过WAK判定是否有手指按在传感器上
uint8_t AS608_PS_GetImage(void); //发送接收PS_GetImage指令
uint8_t AS608_PS_GenChar(uint8_t BufferID); //发送接收PS_GenChar指令
uint8_t AS608_PS_Search(uint8_t BufferID, uint16_t StartPage, 
    uint16_t PageNum, uint16_t* received_PageID, uint16_t* received_MatchScore); //发送接收PS_Search指令
uint8_t AS608_PS_RegModel(void); //发送接收PS_RegModel指令
uint8_t AS608_PS_StoreChar(uint8_t BufferID, uint16_t PageID); //发送接收PS_StoreChar指令
uint8_t AS608_PS_DeleteChar(uint16_t PageID, uint16_t N); //发送接收PS_DeleteChar指令 
uint8_t AS608_PS_Empty(void); //发送接收PS_Empty指令

#endif
