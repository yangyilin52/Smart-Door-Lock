#include "keyboard.h"

//private functions

void Keyboard_setRow(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4){ //BSP Function
    BitAction row1A, row2A, row3A, row4A;
    row1A = row2A = row3A = row4A = Bit_SET;
    if(row1 == 0){
        row1A = Bit_RESET;
    }
    if(row2 == 0){
        row2A = Bit_RESET;
    }
    if(row3 == 0){
        row3A = Bit_RESET;
    }
    if(row4 == 0){
        row4A = Bit_RESET;
    }
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, row1A);
    GPIO_WriteBit(GPIOA, GPIO_Pin_11, row2A);
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, row3A);
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, row4A);
}

uint8_t Keyboard_readColumn(uint8_t column){ //BSP Function
    uint8_t value = 0;
    if(column == 1){
        value = GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6);
    }
    else if(column == 2){
        value = GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_7);
    }
    else if(column == 3){
        value = GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_8);
    }
    return value;
}

//public functions

void Keyboard_init(){ //BSP Function
    delay_init();
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
}

uint8_t Keyboard_scan(){
    uint8_t value = 255;
    
    Keyboard_setRow(0, 1, 1, 1);
    if(Keyboard_readColumn(1) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(1) == 0){
            value = 0;
            while(Keyboard_readColumn(1) != 1);
            return value;
        }
    }
    else if(Keyboard_readColumn(2) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(2) == 0){
            value = 1;
            while(Keyboard_readColumn(2) != 1);
            return value;
        }
    }
    else if(Keyboard_readColumn(3) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(3) == 0){
            value = 2;
            while(Keyboard_readColumn(3) != 1);
            return value;
        }
    }
    
    Keyboard_setRow(1, 0, 1, 1);
    if(Keyboard_readColumn(1) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(1) == 0){
            value = 3;
            while(Keyboard_readColumn(1) != 1);
            return value;
        }
    }
    else if(Keyboard_readColumn(2) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(2) == 0){
            value = 4;
            while(Keyboard_readColumn(2) != 1);
            return value;
        }
    }
    else if(Keyboard_readColumn(3) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(3) == 0){
            value = 5;
            while(Keyboard_readColumn(3) != 1);
            return value;
        }
    }
    
    Keyboard_setRow(1, 1, 0, 1);
    if(Keyboard_readColumn(1) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(1) == 0){
            value = 6;
            while(Keyboard_readColumn(1) != 1);
            return value;
        }
    }
    else if(Keyboard_readColumn(2) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(2) == 0){
            value = 7;
            while(Keyboard_readColumn(2) != 1);
            return value;
        }
    }
    else if(Keyboard_readColumn(3) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(3) == 0){
            value = 8;
            while(Keyboard_readColumn(3) != 1);
            return value;
        }
    }
    
    Keyboard_setRow(1, 1, 1, 0);
    if(Keyboard_readColumn(1) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(1) == 0){
            value = 9;
            while(Keyboard_readColumn(1) != 1);
            return value;
        }
    }
    else if(Keyboard_readColumn(2) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(2) == 0){
            value = 10;
            while(Keyboard_readColumn(2) != 1);
            return value;
        }
    }
    else if(Keyboard_readColumn(3) == 0){
        delay_ms(5);
        if(Keyboard_readColumn(3) == 0){
            value = 11;
            while(Keyboard_readColumn(3) != 1);
            return value;
        }
    }
    return value;
}
