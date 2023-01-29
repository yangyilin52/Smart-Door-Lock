#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>
#include "stm32f10x.h"
#include "bsp_utils.h"

typedef enum{
    Lock_DoorStatus_Closed,
    Lock_DoorStatus_Open
}Lock_DoorStatus;

void Lock_init(void); //初始化门锁机构
void Lock_setLock(uint8_t locked); //设置开锁关锁
Lock_DoorStatus Lock_getDoorStatus(void); //获取是否关门的状态

#endif
