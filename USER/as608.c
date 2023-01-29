#include "as608.h"

uint8_t AS608_recvBuff[255];
uint8_t AS608_recvBuffPtr = 0;//the first empty position

//Private Functions

void AS608_sendData(uint8_t* data, uint8_t length){ //BSP Function
    uint8_t i;
    for(i = 0; i < length; i++){
        USART_SendData(USART3, *data);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) != SET);
        data++;
    }
}

void AS608_clearRecvBuff(){
    uint8_t i;
    for(i = 0; i < AS608_recvBuffPtr; i++){
        AS608_recvBuff[i] = 0x00;
    }
    AS608_recvBuffPtr = 0;
}

void USART3_IRQHandler(void){ //BSP Function
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET){
        if(AS608_recvBuffPtr < 255){
            AS608_recvBuff[AS608_recvBuffPtr] = USART_ReceiveData(USART3);
            AS608_recvBuffPtr++;
        }
    }
}


//Public Functions

void AS608_init(){ //BSP Function
    delay_init();
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &USART_InitStructure);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);
    
    USART_GetFlagStatus(USART3, USART_FLAG_TC);//Without it, the first sent byte lost
}

uint8_t AS608_isFingerAvailable(){ //BSP Function
    uint8_t value = 0;
    if(GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_11) == 1){
        delay_ms(5);
        if(GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_11) == 1){
            value = 1;
        }
    }
    return value;
}

uint8_t AS608_PS_GetImage(){
    uint8_t value = 1;
    AS608_clearRecvBuff();
    uint8_t sData[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05};
    AS608_sendData(sData, 12);
    while(AS608_recvBuffPtr < 12);
    uint16_t checkSumCalc = (uint16_t)AS608_recvBuff[6] + ((uint16_t)AS608_recvBuff[7] << 8) + (uint16_t)AS608_recvBuff[8] + (uint16_t)AS608_recvBuff[9];
    uint16_t checkSumRecv = ((uint16_t)AS608_recvBuff[10] << 8) + (uint16_t)AS608_recvBuff[11];
    if(checkSumCalc == checkSumRecv){
        if(AS608_recvBuff[9] == 0x00){
            value = 0;
        }
    }
    return value;
}

uint8_t AS608_PS_GenChar(uint8_t BufferID){
    uint8_t value = 1;
    if(BufferID == 1 || BufferID == 2){
        AS608_clearRecvBuff();
        uint8_t sData[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, BufferID, 0x00, 0x07 + BufferID};
        AS608_sendData(sData, 13);
        while(AS608_recvBuffPtr < 12);
        uint16_t checkSumCalc = (uint16_t)AS608_recvBuff[6] + ((uint16_t)AS608_recvBuff[7] << 8) + (uint16_t)AS608_recvBuff[8] + (uint16_t)AS608_recvBuff[9];
        uint16_t checkSumRecv = ((uint16_t)AS608_recvBuff[10] << 8) + (uint16_t)AS608_recvBuff[11];
        if(checkSumCalc == checkSumRecv){
            if(AS608_recvBuff[9] == 0x00){
                value = 0;
            }
        }
    }
    return value;
}

uint8_t AS608_PS_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, uint16_t* received_PageID, uint16_t* received_MatchScore){
    uint8_t value = 1;
    if(BufferID == 1 || BufferID == 2){
        AS608_clearRecvBuff();
        uint8_t sData[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, BufferID, 
            (uint8_t)(StartPage >> 8), (uint8_t)(StartPage & 0x00FF), 
            (uint8_t)(PageNum >> 8), (uint8_t)(PageNum & 0x00FF),
            (uint8_t)(((uint16_t)0x0D + (uint16_t)BufferID + StartPage + PageNum) >> 8), (uint8_t)(((uint16_t)0x0D + (uint16_t)BufferID + StartPage + PageNum) & 0x00FF)};
        AS608_sendData(sData, 17);
        while(AS608_recvBuffPtr < 16);
        uint16_t checkSumCalc = (uint16_t)AS608_recvBuff[6] + ((uint16_t)AS608_recvBuff[7] << 8) + (uint16_t)AS608_recvBuff[8] + (uint16_t)AS608_recvBuff[9] + ((uint16_t)AS608_recvBuff[10] << 8) + (uint16_t)AS608_recvBuff[11]
            + ((uint16_t)AS608_recvBuff[12] << 8) + (uint16_t)AS608_recvBuff[13];
        uint16_t checkSumRecv = ((uint16_t)AS608_recvBuff[14] << 8) + (uint16_t)AS608_recvBuff[15];
        if(checkSumCalc == checkSumRecv){
            if(AS608_recvBuff[9] == 0x00){
                value = 0;
                (*received_PageID) = ((uint16_t)AS608_recvBuff[10] << 8) + (uint16_t)(AS608_recvBuff[11]);
                (*received_MatchScore) = ((uint16_t)AS608_recvBuff[12] << 8) + (uint16_t)(AS608_recvBuff[13]);
            }
        }
    }
    return value;
}

uint8_t AS608_PS_RegModel(){
    uint8_t value = 1;
    AS608_clearRecvBuff();
    uint8_t sData[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x05, 0x00, 0x09};
    AS608_sendData(sData, 12);
    while(AS608_recvBuffPtr < 12);
    uint16_t checkSumCalc = (uint16_t)AS608_recvBuff[6] + ((uint16_t)AS608_recvBuff[7] << 8) + (uint16_t)AS608_recvBuff[8] + (uint16_t)AS608_recvBuff[9];
    uint16_t checkSumRecv = ((uint16_t)AS608_recvBuff[10] << 8) + (uint16_t)AS608_recvBuff[11];
    if(checkSumCalc == checkSumRecv){
        if(AS608_recvBuff[9] == 0x00){
            value = 0;
        }
    }
    return value;
}

uint8_t AS608_PS_StoreChar(uint8_t BufferID, uint16_t PageID){
    uint8_t value = 1;
    if(BufferID == 0 || BufferID == 1){
        AS608_clearRecvBuff();
        uint8_t sData[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x06, BufferID, (uint8_t)(PageID >> 8), (uint8_t)(PageID & 0x00FF), 
        (uint8_t)(((uint16_t)0x0D + (uint16_t)BufferID + PageID) >> 8), (uint8_t)(((uint16_t)0x0D + (uint16_t)BufferID + PageID) & 0x00FF)};
        AS608_sendData(sData, 15);
        while(AS608_recvBuffPtr < 12);
        uint16_t checkSumCalc = (uint16_t)AS608_recvBuff[6] + ((uint16_t)AS608_recvBuff[7] << 8) + (uint16_t)AS608_recvBuff[8] + (uint16_t)AS608_recvBuff[9];
        uint16_t checkSumRecv = ((uint16_t)AS608_recvBuff[10] << 8) + (uint16_t)AS608_recvBuff[11];
        if(checkSumCalc == checkSumRecv){
            if(AS608_recvBuff[9] == 0x00){
                value = 0;
            }
        }
    }
    return value;
}

uint8_t AS608_PS_DeleteChar(uint16_t PageID, uint16_t N){
    uint8_t value = 1;
    AS608_clearRecvBuff();
    uint8_t sData[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C, (uint8_t)(PageID >> 8), (uint8_t)(PageID & 0x00FF), (uint8_t)(N >> 8), (uint8_t)(N & 0x00FF), 
        (uint8_t)(((uint16_t)0x14 + PageID + N) >> 8), (uint8_t)(((uint16_t)0x14 + PageID + N) & 0x00FF)};
    AS608_sendData(sData, 16);
    while(AS608_recvBuffPtr < 12);
    uint16_t checkSumCalc = (uint16_t)AS608_recvBuff[6] + ((uint16_t)AS608_recvBuff[7] << 8) + (uint16_t)AS608_recvBuff[8] + (uint16_t)AS608_recvBuff[9];
    uint16_t checkSumRecv = ((uint16_t)AS608_recvBuff[10] << 8) + (uint16_t)AS608_recvBuff[11];
    if(checkSumCalc == checkSumRecv){
        if(AS608_recvBuff[9] == 0x00){
            value = 0;
        }
    }
    return value;
}

uint8_t AS608_PS_Empty(){
    uint8_t value = 1;
    AS608_clearRecvBuff();
    uint8_t sData[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0d, 0x00, 0x11};
    AS608_sendData(sData, 12);
    while(AS608_recvBuffPtr < 12);
    uint16_t checkSumCalc = (uint16_t)AS608_recvBuff[6] + ((uint16_t)AS608_recvBuff[7] << 8) + (uint16_t)AS608_recvBuff[8] + (uint16_t)AS608_recvBuff[9];
    uint16_t checkSumRecv = ((uint16_t)AS608_recvBuff[10] << 8) + (uint16_t)AS608_recvBuff[11];
    if(checkSumCalc == checkSumRecv){
        if(AS608_recvBuff[9] == 0x00){
            value = 0;
        }
    }
    return value;
}
