#ifndef BSP_UTILS_H
#define BSP_UTILS_H

#include <stdint.h>
#include "stm32f10x.h"

//Public Functions

void delay_init(void);
void delay_us(uint16_t time);  //'time' should be less than 50000us
void delay_ms(uint32_t time);

#endif
