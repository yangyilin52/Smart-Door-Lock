#include "bsp_utils.h"

uint8_t delay_initialized = 0;

//Public Functions

void delay_init(){
    if(delay_initialized == 0){
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
        TIM_TimeBaseInitStructure.TIM_Prescaler = 72;
        TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInitStructure.TIM_Period = 60000;
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
        delay_initialized = 1;
    }
}

void delay_us(uint16_t time){ //'time' should be less than 50000us
    if(time <= 50000){
        TIM_SetCounter(TIM2, 0);
        TIM_Cmd(TIM2, ENABLE);
        while(TIM_GetCounter(TIM2) < time);
        TIM_Cmd(TIM2, DISABLE);
    }
}

void delay_ms(uint32_t time){
    uint32_t i;
    TIM_SetCounter(TIM2, 0);
    TIM_Cmd(TIM2, ENABLE);
    for(i = 0; i < time; i++){
        while(TIM_GetCounter(TIM2) < 1000);
        TIM_SetCounter(TIM2, 0);
    }
    TIM_Cmd(TIM2, DISABLE);
}
