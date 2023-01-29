#include "beep.h"

uint8_t Beep_pinPotential = 0;

//Private Functions

void TIM3_IRQHandler(void){ //BSP Function
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        if(Beep_pinPotential == 0){
            Beep_pinPotential = 1;
            GPIO_WriteBit(GPIOF, GPIO_Pin_7, Bit_SET);
        }
        else if(Beep_pinPotential == 1){
            Beep_pinPotential = 0;
            GPIO_WriteBit(GPIOF, GPIO_Pin_7, Bit_RESET);
        }
	}
}

//Public Functions

void Beep_init(){ //BSP Function
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 1136; //La 440Hz
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    Beep_pinPotential = 1;
    GPIO_WriteBit(GPIOF, GPIO_Pin_7, Bit_SET);
}

void Beep_setEnable(uint8_t enable){ //BSP Function
    if(enable == 0){
        TIM_Cmd(TIM3, DISABLE);
        Beep_pinPotential = 1;
        GPIO_WriteBit(GPIOF, GPIO_Pin_7, Bit_SET);
    }
    else{
        TIM_SetCounter(TIM3, 0);
        Beep_pinPotential = 1;
        GPIO_WriteBit(GPIOF, GPIO_Pin_7, Bit_SET);
        TIM_Cmd(TIM3, ENABLE);
    }
}

void Beep_setFrequency(uint16_t frequency){
    uint32_t t = (uint32_t)(500000.0 / (double)frequency);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = t;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
}

