#ifndef RC522_H
#define	RC522_H

#include "stm32f10x.h"
#include <stdint.h>
#include "bsp_utils.h"
#include "rc522_config.h"

typedef enum{
    RC522_Pin_NSS,
    RC522_Pin_RST
}RC522_Pin;

//Private Functions
void RC522_hwInit(void); //BSP Function
void RC522_pinCtrl(RC522_Pin pin, uint8_t status); //BSP Function
uint8_t RC522_SPIWriteReadByte(uint8_t txData); //BSP Function
uint8_t RC522_readRawRC(uint8_t ucAddress);
void RC522_writeRawRC(uint8_t ucAddress, uint8_t ucValue);
void RC522_setBitMask(uint8_t ucReg, uint8_t ucMask)  ;
void RC522_clearBitMask(uint8_t ucReg, uint8_t ucMask) ;
void RC522_pcdAntennaOn(void);
void RC522_pcdAntennaOff(void);
void RC522_M500PcdConfigISOType(uint8_t ucType);
uint8_t RC522_pcdComMF522(uint8_t ucCommand, uint8_t* pInData, uint8_t ucInLenByte, uint8_t* pOutData, uint32_t* pOutLenBit);
void RC522_caculateCRC(uint8_t* pIndata, uint8_t ucLen, uint8_t* pOutData);

//Public Functions
void RC522_init(void); //初始化RC522
void RC522_pcdReset(void); //重置RC522
uint8_t RC522_pcdRequest(uint8_t ucReq_code, uint8_t* pTagType); //RC522寻卡
uint8_t RC522_pcdAnticoll(uint8_t* pSnr); //RC522防冲撞
uint8_t RC522_pcdSelect(uint8_t* pSnr); //RC522选择卡
uint8_t RC522_pcdAuthState(uint8_t ucAuth_mode, uint8_t ucAddr, uint8_t* pKey, uint8_t* pSnr); //RC522密码认证
uint8_t RC522_pcdWrite(uint8_t ucAddr, uint8_t* pData); //RC522写数据
uint8_t RC522_pcdRead(uint8_t ucAddr, uint8_t* pData); //RC522读数据
uint8_t RC522_pcdHalt(void); //RC522休眠


//////////////////// EXAMPLES ////////////////////
uint8_t WriteAmount( uint8_t ucAddr, uint32_t pData );
uint8_t ReadAmount( uint8_t ucAddr, uint32_t *pData );

#endif

