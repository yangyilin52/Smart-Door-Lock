#include "led.h"

Led_Color Led_usingColor = Led_Color_None;
uint16_t Led_timeCount = 0;
uint16_t Led_lightTimeSet = 500;
uint16_t Led_restTimeSet = 500;
uint8_t Led_isOff = 0;

//Private Function

void Led_pinCtrl(uint8_t Red, uint8_t Green, uint8_t Blue){ //BSP Function
    BitAction RedA, GreenA, BlueA;
    RedA = GreenA = BlueA = Bit_SET;
    if(Red == 0){
        RedA = Bit_RESET;
    }
    if(Green == 0){
        GreenA = Bit_RESET;
    }
    if(Blue == 0){
        BlueA = Bit_RESET;
    }
    GPIO_WriteBit(GPIOF, GPIO_Pin_9, RedA);
    GPIO_WriteBit(GPIOF, GPIO_Pin_8, GreenA);
    GPIO_WriteBit(GPIOF, GPIO_Pin_10, BlueA);
}

void TIM4_IRQHandler(void){ //BSP Function
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET){
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        Led_timeCount++;
		if(Led_isOff == 1){
            if(Led_timeCount >= Led_restTimeSet){
                Led_isOff = 0;
                Led_setColor(Led_usingColor);
                Led_timeCount = 0;
            }
        }
        else if(Led_isOff == 0){
            if(Led_timeCount >= Led_lightTimeSet){
                Led_isOff = 1;
                Led_pinCtrl(0, 0, 0);
                Led_timeCount = 0;
            }
        }
	}
}


//Public Function

void Led_init(){ //BSP Function
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 10;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure); //Interrupt every 1ms
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
    
    Led_setColor(Led_Color_None);
}

void Led_setColor(Led_Color color){
    Led_usingColor = color;
    if(color == Led_Color_Blue){
        Led_pinCtrl(0, 0, 1);
    }
    else if(color == Led_Color_Red){
        Led_pinCtrl(1, 0, 0);
    }
    else if(color == Led_Color_Green){
        Led_pinCtrl(0, 1, 0);
    }
    else if(color == Led_Color_Purple){
        Led_pinCtrl(1, 0, 1);
    }
    else if(color == Led_Color_Yellow){
        Led_pinCtrl(1, 1, 0);
    }
    else if(color == Led_Color_SkyBlue){
        Led_pinCtrl(0, 1, 1);
    }
    else if(color == Led_Color_White){
        Led_pinCtrl(1, 1, 1);
    }
    else if(color == Led_Color_None){
        Led_pinCtrl(0, 0, 0);
    }
}

void Led_setBlink(uint8_t enable, uint16_t lightTimeMs, uint16_t restTimeMs){ //BSP Function
    if(enable != 0 && lightTimeMs != 0 && restTimeMs != 0){
        Led_lightTimeSet = lightTimeMs;
        Led_restTimeSet = restTimeMs;
        Led_timeCount = 0;
        Led_isOff = 0;
        Led_setColor(Led_usingColor);
        TIM_SetCounter(TIM4, 0);
        TIM_Cmd(TIM4, ENABLE);
    }
    else if(enable == 0){
        TIM_Cmd(TIM4, DISABLE);
        Led_setColor(Led_usingColor);
    }
}
