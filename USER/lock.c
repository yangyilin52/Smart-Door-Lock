#include "lock.h"

void Lock_init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    Lock_setLock(1);
}

void Lock_setLock(uint8_t locked){
    BitAction action = Bit_RESET;
    if(locked == 0){
        action = Bit_SET;
    }
    GPIO_WriteBit(GPIOD, GPIO_Pin_7, action);
    delay_ms(100); //To Fix Bug that SPI being Affected by EMF
}

Lock_DoorStatus Lock_getDoorStatus(){
    if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == 0){
        return Lock_DoorStatus_Open;
    }
    else{
        return Lock_DoorStatus_Closed;
    }
}
