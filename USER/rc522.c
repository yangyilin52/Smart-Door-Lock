#include "rc522.h"

//Private Functions

void RC522_hwInit(void){ //BSP Function
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    delay_us(20);
}

void RC522_pinCtrl(RC522_Pin pin, uint8_t status){ //BSP Function
    BitAction action = Bit_SET;
    if(status == 0){
        action = Bit_RESET;
    }
    if(pin == RC522_Pin_NSS){
        GPIO_WriteBit(GPIOB, GPIO_Pin_1, action);
    }
    else if(pin == RC522_Pin_RST){
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, action);
    }
}

uint8_t RC522_SPIWriteReadByte(uint8_t txData){ //BSP Function
    //RC522_pinCtrl(RC522_Pin_NSS, 0);  //Attention! RC522 doesn't work if controlling NSS here.
    uint8_t retry = 0;
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){
        retry++;
        if(retry > 200){
            return 0;
        }
    }
    SPI_I2S_SendData(SPI1, txData);
    retry = 0;
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){
        retry++;
        if(retry > 200){
            return 0;
        }
    }
    //RC522_pinCtrl(RC522_Pin_NSS, 1);   //Attention! RC522 doesn't work if controlling NSS here.
    
    return SPI_I2S_ReceiveData(SPI1);
}

/**
  * @brief  读RC522寄存器
  * @param  ucAddress,寄存器地址
  * @retval 寄存器的当前值
  */
uint8_t RC522_readRawRC(uint8_t ucAddress)
{
	uint8_t ucAddr, ucReturn;
	
	ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;	
    
    RC522_pinCtrl(RC522_Pin_NSS, 0);
    
    RC522_SPIWriteReadByte(ucAddr);
	ucReturn = RC522_SPIWriteReadByte(0x00);
    
    RC522_pinCtrl(RC522_Pin_NSS, 1);
    
	return ucReturn;	
}

/**
  * @brief  写RC522寄存器
  * @param  ucAddress,寄存器地址
  * @param  ucValue,写入寄存器的值
  * @retval 无
  */
void RC522_writeRawRC(uint8_t ucAddress, uint8_t ucValue)
{  
	uint8_t ucAddr;
	
	ucAddr = (( ucAddress << 1 ) & 0x7E);	
    
    RC522_pinCtrl(RC522_Pin_NSS, 0);
    
    RC522_SPIWriteReadByte(ucAddr);	
    RC522_SPIWriteReadByte(ucValue);
    
    RC522_pinCtrl(RC522_Pin_NSS, 1);	
}

/**
  * @brief  对RC522寄存器置位
  * @param  ucReg,寄存器地址
  * @param   ucMask,置位值
  * @retval 无
  */
void RC522_setBitMask(uint8_t ucReg, uint8_t ucMask)  
{
  uint8_t ucTemp;

  ucTemp = RC522_readRawRC ( ucReg );
  RC522_writeRawRC ( ucReg, ucTemp | ucMask ); // set bit mask
}

/**
  * @brief  对RC522寄存器清位
  * @param  ucReg,寄存器地址
  * @param  ucMask,清位值
  * @retval 无
  */
void RC522_clearBitMask(uint8_t ucReg, uint8_t ucMask)  
{
  uint8_t ucTemp;

  ucTemp = RC522_readRawRC ( ucReg );
  RC522_writeRawRC ( ucReg, ucTemp & ( ~ ucMask) ); // clear bit mask
}


/**
  * @brief  开启天线
  * @param  无
  * @retval 无
  */
void RC522_pcdAntennaOn()
{
  uint8_t uc;

  uc = RC522_readRawRC ( TxControlReg );
  if ( ! ( uc & 0x03 ) )
   RC522_setBitMask(TxControlReg, 0x03);		
}

/**
  * @brief  关闭天线
  * @param  无
  * @retval 无
  */
void RC522_pcdAntennaOff()
{
  RC522_clearBitMask ( TxControlReg, 0x03 );	
}

/**
  * @brief  用RC522计算CRC16
  * @param  pIndata,计算CRC16的数组
  * @param  ucLen,计算CRC16的数组字节长度
  * @param  pOutData,存放计算结果存放的首地址
  * @retval 无
  */
void RC522_caculateCRC(uint8_t * pIndata, uint8_t ucLen, uint8_t * pOutData)
{
  uint8_t uc, ucN;

  RC522_clearBitMask(DivIrqReg,0x04);

  RC522_writeRawRC(CommandReg,PCD_IDLE);

  RC522_setBitMask(FIFOLevelReg,0x80);

  for ( uc = 0; uc < ucLen; uc ++)
    RC522_writeRawRC ( FIFODataReg, * ( pIndata + uc ) );   

  RC522_writeRawRC ( CommandReg, PCD_CALCCRC );

  uc = 0xFF;

  do 
  {
      ucN = RC522_readRawRC ( DivIrqReg );
      uc --;
  } while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );
  
  pOutData [ 0 ] = RC522_readRawRC ( CRCResultRegL );
  pOutData [ 1 ] = RC522_readRawRC ( CRCResultRegM );		
}


/**
  * @brief  通过RC522和ISO14443卡通讯
  * @param  ucCommand,RC522命令字
  * @param  pInData,通过RC522发送到卡片的数据
  * @param  ucInLenByte,发送数据的字节长度
  * @param  pOutData,接收到的卡片返回数据
  * @param  pOutLenBit,返回数据的位长度
  * @retval 状态值=MI_OK,成功
  */
uint8_t RC522_pcdComMF522(uint8_t ucCommand,
                   uint8_t * pInData, 
                   uint8_t ucInLenByte, 
                   uint8_t * pOutData,
                   uint32_t * pOutLenBit)		
{
  uint8_t cStatus = MI_ERR;
  uint8_t ucIrqEn   = 0x00;
  uint8_t ucWaitFor = 0x00;
  uint8_t ucLastBits;
  uint8_t ucN;
  uint32_t ul;

  switch ( ucCommand )
  {
     case PCD_AUTHENT:		  //Mifare认证
        ucIrqEn   = 0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
        ucWaitFor = 0x10;		//认证寻卡等待时候 查询空闲中断标志位
        break;
     
     case PCD_TRANSCEIVE:		//接收发送 发送接收
        ucIrqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        ucWaitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与 空闲中断标志位
        break;
     
     default:
       break;     
  }
  //IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反
  RC522_writeRawRC ( ComIEnReg, ucIrqEn | 0x80 );
  //Set1该位清零时,CommIRqReg的屏蔽位清零
  RC522_clearBitMask ( ComIrqReg, 0x80 );	 
  //写空闲命令
  RC522_writeRawRC ( CommandReg, PCD_IDLE );		 
  
  //置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除
  RC522_setBitMask ( FIFOLevelReg, 0x80 );			

  for ( ul = 0; ul < ucInLenByte; ul ++ )
    RC522_writeRawRC ( FIFODataReg, pInData [ ul ] ); //写数据进FIFOdata
    
  RC522_writeRawRC ( CommandReg, ucCommand );					//写命令


  if ( ucCommand == PCD_TRANSCEIVE )
    
  //StartSend置位启动数据发送 该位与收发命令使用时才有效
    RC522_setBitMask(BitFramingReg,0x80);  				  

  ul = 1000;                             //根据时钟频率调整,操作M1卡最大等待时间25ms

  do 														         //认证 与寻卡等待时间
  {
       ucN = RC522_readRawRC ( ComIrqReg );		 //查询事件中断
       ul --;
  } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );	

  RC522_clearBitMask ( BitFramingReg, 0x80 );	 //清理允许StartSend位

  if ( ul != 0 )
  {
    //读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
    if ( ! ( RC522_readRawRC ( ErrorReg ) & 0x1B ) )	
    {
      cStatus = MI_OK;
      
      if ( ucN & ucIrqEn & 0x01 )				//是否发生定时器中断
        cStatus = MI_NOTAGERR;   
        
      if ( ucCommand == PCD_TRANSCEIVE )
      {
        //读FIFO中保存的字节数
        ucN = RC522_readRawRC ( FIFOLevelReg );		          
        
        //最后接收到的字节的有效位数
        ucLastBits = RC522_readRawRC ( ControlReg ) & 0x07;	
        
        if ( ucLastBits )
          
          //N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
          * pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	
        else
          * pOutLenBit = ucN * 8;      //最后接收到的字节整个字节有效
        
        if ( ucN == 0 )		
          ucN = 1;    
        
        if ( ucN > MAXRLEN )
          ucN = MAXRLEN;   
        
        for ( ul = 0; ul < ucN; ul ++ )
          pOutData [ ul ] = RC522_readRawRC ( FIFODataReg );   
        
        }        
    }   
    else
      cStatus = MI_ERR;       
  }

  RC522_setBitMask ( ControlReg, 0x80 );           // stop timer now
  RC522_writeRawRC ( CommandReg, PCD_IDLE ); 
   
  return cStatus;
}



/**
  * @brief  设置RC522的工作方式
  * @param  ucType,工作方式
  * @retval 无
  */
void RC522_M500PcdConfigISOType(uint8_t ucType){
    if ( ucType == 'A'){                     //ISO14443_A
    
        RC522_clearBitMask ( Status2Reg, 0x08 );
		
        RC522_writeRawRC ( ModeReg, 0x3D );         //3F
		
        RC522_writeRawRC ( RxSelReg, 0x86 );        //84
		
        RC522_writeRawRC( RFCfgReg, 0x7F );         //4F
		
        RC522_writeRawRC( TReloadRegL, 30 );        
		
        RC522_writeRawRC ( TReloadRegH, 0 );
		
        RC522_writeRawRC ( TModeReg, 0x8D );
		
        RC522_writeRawRC ( TPrescalerReg, 0x3E );
		
        delay_us ( 10 );
		
        RC522_pcdAntennaOn ();//开天线
    }        
}



//Public Functions

void RC522_init(){
    delay_init();
    
	RC522_hwInit();
    RC522_pcdReset();
}

/**
  * @brief  复位RC522 
  * @param  无
  * @retval 无
  */
void RC522_pcdReset()
{
    RC522_pinCtrl(RC522_Pin_RST, 1);
	RC522_pinCtrl(RC522_Pin_NSS, 1);
	delay_us ( 10 );
	
	RC522_pinCtrl(RC522_Pin_RST, 0);
	delay_us ( 10 );
	RC522_pinCtrl(RC522_Pin_RST, 1);
	delay_us ( 10 );
	
	RC522_writeRawRC ( CommandReg, PCD_RESETPHASE );
	
	while ( RC522_readRawRC ( CommandReg ) & 0x10 );

	delay_us ( 10 );
  
	//定义发送和接收常用模式 和Mifare卡通讯,CRC初始值0x6363
  RC522_writeRawRC ( ModeReg, 0x3D );        
	
  RC522_writeRawRC ( TReloadRegL, 30 );      //16位定时器低位  
	RC522_writeRawRC ( TReloadRegH, 0 );			 //16位定时器高位
	
  RC522_writeRawRC ( TModeReg, 0x8D );			 //定义内部定时器的设置
	
  RC522_writeRawRC ( TPrescalerReg, 0x3E );	 //设置定时器分频系数
	
	RC522_writeRawRC ( TxAutoReg, 0x40 );			 //调制发送信号为100%ASK

    RC522_M500PcdConfigISOType('A');
}

/**
  * @brief  寻卡
  * @param  ucReq_code,寻卡方式 = 0x52, 寻感应区内所有符合14443A标准的卡；
            寻卡方式 = 0x26，寻未进入休眠状态的卡
  * @param  pTagType，卡片类型代码
             = 0x4400, Mifare_UltraLight
             = 0x0400, Mifare_One(S50)
             = 0x0200, Mifare_One(S70)
             = 0x0800, Mifare_Pro(X))
             = 0x4403, Mifare_DESFire
  * @retval 状态值 = MI_OK, 成功
  */
uint8_t RC522_pcdRequest(uint8_t ucReq_code, uint8_t * pTagType)
{
  uint8_t cStatus;  
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;

    //清理指示MIFARECryptol单元接通以及所有卡的数据通信被加密的情况
  RC522_clearBitMask ( Status2Reg, 0x08 );
	//发送的最后一个字节的 七位
  RC522_writeRawRC ( BitFramingReg, 0x07 );
  //TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号
  RC522_setBitMask ( TxControlReg, 0x03 );	

  ucComMF522Buf [ 0 ] = ucReq_code;		//存入 卡片命令字

  cStatus = RC522_pcdComMF522 ( PCD_TRANSCEIVE,	
                          ucComMF522Buf,
                          1, 
                          ucComMF522Buf,
                          & ulLen );	//寻卡 

  if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//寻卡成功返回卡类型
  {    
     * pTagType = ucComMF522Buf [ 0 ];
     * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
  }

  else
   cStatus = MI_ERR;

  return cStatus;	 
}

/**
  * @brief  防冲撞
  * @param  pSnr，卡片序列号，4字节
  * @retval 状态值 = MI_OK，成功
  */
uint8_t RC522_pcdAnticoll(uint8_t * pSnr)
{
  uint8_t cStatus;
  uint8_t uc, ucSnr_check = 0;
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;
  
  //清理MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
  RC522_clearBitMask ( Status2Reg, 0x08 );
  //清理寄存器 停止收发
  RC522_writeRawRC ( BitFramingReg, 0x00);	
	//清ValuesAfterColl所有接收的位在冲突后被清除
  RC522_clearBitMask ( CollReg, 0x80 );			  
 
  ucComMF522Buf [ 0 ] = 0x93;	          //卡片防冲突命令
  ucComMF522Buf [ 1 ] = 0x20;
 
  cStatus = RC522_pcdComMF522 ( PCD_TRANSCEIVE, 
                          ucComMF522Buf,
                          2, 
                          ucComMF522Buf,
                          & ulLen);      //与卡片通信

  if ( cStatus == MI_OK)		            //通信成功
  {
    for ( uc = 0; uc < 4; uc ++ )
    {
       * ( pSnr + uc )  = ucComMF522Buf [ uc ]; //读出UID
       ucSnr_check ^= ucComMF522Buf [ uc ];
    }
    
    if ( ucSnr_check != ucComMF522Buf [ uc ] )
      cStatus = MI_ERR;    				 
  }
  
  RC522_setBitMask ( CollReg, 0x80 );
      
  return cStatus;		
}

/**
  * @brief  选定卡片
  * @param  pSnr，卡片序列号，4字节
  * @retval 状态值 = MI_OK，成功
  */
uint8_t RC522_pcdSelect(uint8_t * pSnr)
{
  uint8_t ucN;
  uint8_t uc;
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t  ulLen;
  
  
  ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
  ucComMF522Buf [ 1 ] = 0x70;
  ucComMF522Buf [ 6 ] = 0;

  for ( uc = 0; uc < 4; uc ++ )
  {
    ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
    ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
  }
  
  RC522_caculateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );

  RC522_clearBitMask ( Status2Reg, 0x08 );

  ucN = RC522_pcdComMF522 ( PCD_TRANSCEIVE,
                     ucComMF522Buf,
                     9,
                     ucComMF522Buf, 
                     & ulLen );
  
  if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
    ucN = MI_OK;  
  else
    ucN = MI_ERR;    
  
  return ucN;		
}



/**
  * @brief  验证卡片密码
  * @param  ucAuth_mode，密码验证模式 = 0x60，验证A密钥，
            密码验证模式 = 0x61，验证B密钥
  * @param  uint8_t ucAddr，块地址
  * @param  pKey，密码 
  * @param  pSnr，卡片序列号，4字节
  * @retval 状态值 = MI_OK，成功
  */
uint8_t RC522_pcdAuthState(uint8_t ucAuth_mode, 
                    uint8_t ucAddr, 
                    uint8_t * pKey,
                    uint8_t * pSnr)
{
  uint8_t cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
  uint32_t ulLen;
  

  ucComMF522Buf [ 0 ] = ucAuth_mode;
  ucComMF522Buf [ 1 ] = ucAddr;

  for ( uc = 0; uc < 6; uc ++ )
    ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );   

  for ( uc = 0; uc < 6; uc ++ )
    ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );   

  cStatus = RC522_pcdComMF522 ( PCD_AUTHENT,
                          ucComMF522Buf, 
                          12,
                          ucComMF522Buf,
                          & ulLen );

  if ( ( cStatus != MI_OK ) || ( ! ( RC522_readRawRC ( Status2Reg ) & 0x08 ) ) )
    cStatus = MI_ERR;   
    
  return cStatus;
}


/**
  * @brief  写数据到M1卡一块
  * @param  uint8_t ucAddr，块地址
  * @param  pData，写入的数据，16字节
  * @retval 状态值 = MI_OK，成功
  */
uint8_t RC522_pcdWrite(uint8_t ucAddr, uint8_t * pData)
{
  uint8_t cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
  uint32_t ulLen;
   
  
  ucComMF522Buf [ 0 ] = PICC_WRITE;
  ucComMF522Buf [ 1 ] = ucAddr;

  RC522_caculateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

  cStatus = RC522_pcdComMF522 ( PCD_TRANSCEIVE,
                          ucComMF522Buf,
                          4, 
                          ucComMF522Buf,
                          & ulLen );

  if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || 
         ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
    cStatus = MI_ERR;   
      
  if ( cStatus == MI_OK )
  {
    //memcpy(ucComMF522Buf, pData, 16);
    for ( uc = 0; uc < 16; uc ++ )
      ucComMF522Buf [ uc ] = * ( pData + uc );  
    
    RC522_caculateCRC ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

    cStatus = RC522_pcdComMF522 ( PCD_TRANSCEIVE,
                           ucComMF522Buf, 
                           18, 
                           ucComMF522Buf,
                           & ulLen );
    
    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || 
         ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
      cStatus = MI_ERR;   
    
  } 	
  return cStatus;		
}


/**
  * @brief  读取M1卡一块数据
  * @param  ucAddr，块地址
  * @param  pData，读出的数据，16字节
  * @retval 状态值 = MI_OK，成功
  */
uint8_t RC522_pcdRead(uint8_t ucAddr, uint8_t * pData)
{
  uint8_t cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;
  
  ucComMF522Buf [ 0 ] = PICC_READ;
  ucComMF522Buf [ 1 ] = ucAddr;

  RC522_caculateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 
  cStatus = RC522_pcdComMF522 ( PCD_TRANSCEIVE,
                          ucComMF522Buf,
                          4, 
                          ucComMF522Buf,
                          & ulLen );

  if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
  {
    for ( uc = 0; uc < 16; uc ++ )
      * ( pData + uc ) = ucComMF522Buf [ uc ];   
  }
  
  else
    cStatus = MI_ERR;   
   
  return cStatus;		
}


/**
  * @brief  命令卡片进入休眠状态
  * @param  无
  * @retval 状态值 = MI_OK，成功
  */
uint8_t RC522_pcdHalt()
{
	uint8_t ucComMF522Buf [ MAXRLEN ]; 
	uint32_t  ulLen;
  

  ucComMF522Buf [ 0 ] = PICC_HALT;
  ucComMF522Buf [ 1 ] = 0;
	
  RC522_caculateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 	RC522_pcdComMF522 ( PCD_TRANSCEIVE,
                ucComMF522Buf,
                4, 
                ucComMF522Buf, 
                & ulLen );

  return MI_OK;	
}


///////////////////  EXAMPLES ///////////////////


/////////////////////////////////////////////////////////////////////
//功    能：写入钱包金额
//参数说明：ucAddr[IN]：块地址
//          pData：写入的金额
//返    回：状态值 = MI_OK，成功
/////////////////////////////////////////////////////////////////////
uint8_t WriteAmount( uint8_t ucAddr, uint32_t pData )
{
	uint8_t status;
	uint8_t ucComMF522Buf[16];
	ucComMF522Buf[0] = (pData&((uint32_t)0x000000ff));
	ucComMF522Buf[1] = (pData&((uint32_t)0x0000ff00))>>8;
	ucComMF522Buf[2] = (pData&((uint32_t)0x00ff0000))>>16;
	ucComMF522Buf[3] = (pData&((uint32_t)0xff000000))>>24;	
	
	ucComMF522Buf[4] = ~(pData&((uint32_t)0x000000ff));
	ucComMF522Buf[5] = ~(pData&((uint32_t)0x0000ff00))>>8;
	ucComMF522Buf[6] = ~(pData&((uint32_t)0x00ff0000))>>16;
	ucComMF522Buf[7] = ~(pData&((uint32_t)0xff000000))>>24;	
	
	ucComMF522Buf[8] = (pData&((uint32_t)0x000000ff));
	ucComMF522Buf[9] = (pData&((uint32_t)0x0000ff00))>>8;
	ucComMF522Buf[10] = (pData&((uint32_t)0x00ff0000))>>16;
	ucComMF522Buf[11] = (pData&((uint32_t)0xff000000))>>24;	
	
	ucComMF522Buf[12] = ucAddr;
	ucComMF522Buf[13] = ~ucAddr;
	ucComMF522Buf[14] = ucAddr;
	ucComMF522Buf[15] = ~ucAddr;
  status = RC522_pcdWrite(ucAddr,ucComMF522Buf);
	return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：读取钱包金额
//参数说明：ucAddr[IN]：块地址
//          *pData：读取的金额
//返    回：状态值 = MI_OK，成功
/////////////////////////////////////////////////////////////////////
uint8_t ReadAmount( uint8_t ucAddr, uint32_t *pData )
{
	
	uint8_t status = MI_ERR;
	uint8_t j;
	uint8_t ucComMF522Buf[16];
  status = RC522_pcdRead(ucAddr,ucComMF522Buf);
	if(status != MI_OK)
		return status;
	for(j=0;j<4;j++)
	{
		if((ucComMF522Buf[j] != ucComMF522Buf[j+8]) && (ucComMF522Buf[j] != ~ucComMF522Buf[j+4]))
		break;
	}
	if(j == 4)
	{
		  status = MI_OK;
			*pData = ucComMF522Buf[0] + (ucComMF522Buf[1]<<8) + (ucComMF522Buf[2]<<16) + (ucComMF522Buf[3]<<24);
	}
	else
	{
		status = MI_ERR;
		*pData = 0;
	}
  return status;	
}

