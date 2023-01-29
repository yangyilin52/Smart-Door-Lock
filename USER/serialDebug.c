#include "serialDebug.h"

uint8_t sDebug_enable = 1;

//public functions

void sDebug_init(){ //BSP Function
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
    
    USART_GetFlagStatus(USART1, USART_FLAG_TC); //Without it, the first sent byte lost
}

void sDebug_setEnable(uint8_t enable){
    sDebug_enable = enable;
}

void sDebug_println(char* str){ //BSP Function
    if(sDebug_enable != 0){
        while((*str) != '\0'){
            USART_SendData(USART1, *str);
            while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
            str++;
        }
        USART_SendData(USART1, '\r');
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
        USART_SendData(USART1, '\n');
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
    }
}
