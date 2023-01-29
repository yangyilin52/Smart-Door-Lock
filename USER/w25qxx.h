#ifndef W25QXX_H
#define W25QXX_H		

#include "stm32f10x.h"
#include <stdint.h>
#include "bsp_utils.h"

////////////////////////////////////////////////////////////////////////////
//Chip ID of W25Qxx and NM25Qxx
#define W25Q80 	0XEF13
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17

#define NM25Q80 	0X5213 	
#define NM25Q16 	0X5214
#define NM25Q32 	0X5215
#define NM25Q64 	0X5216
#define NM25Q128	0X5217
#define NM25Q256 	0X5218
////////////////////////////////////////////////////////////////////////////		   
				 
////////////////////////////////////////////////////////////////////////////
//ָ指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
////////////////////////////////////////////////////////////////////////////

typedef enum {
    W25QXX_Pin_NSS
}W25QXX_Pin;

//Private Functions
uint8_t W25QXX_readSR(void);        		//读取状态寄存器
void W25QXX_writeSR(uint8_t sr);  			//写状态寄存器
void W25QXX_writeEnable(void);  		//写使能
void W25QXX_writeDisable(void);		//写保护
void W25QXX_pinCtrl(W25QXX_Pin pin, uint8_t status); //BSP Function
uint8_t W25QXX_SPIWriteReadByte(uint8_t txData); //BSP Function
void W25QXX_waitBusy(void);           	//等待空闲
void W25QXX_write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void W25QXX_writePage(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);

//Public Functions
void W25QXX_init(void); //BSP Function
uint16_t W25QXX_readID(void);  	    		//读取Flash ID
void W25QXX_read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);   //读取Flash
void W25QXX_write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);//写入Flash
void W25QXX_eraseChip(void);    	  	//整片擦除
void W25QXX_eraseSector(uint32_t Dst_Addr);	//扇区擦除
void W25QXX_powerDown(void);        	//进入掉电模式
void W25QXX_wakeUp(void);				//唤醒

#endif
