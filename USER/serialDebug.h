#ifndef SERIALDEBUG_H
#define SERIALDEBUG_H

#include "stm32f10x.h"
#include <stdint.h>

//public functions

void sDebug_init(void);
void sDebug_setEnable(uint8_t enable);
void sDebug_println(char* str);

#endif
