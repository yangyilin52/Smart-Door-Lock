#include "w25qxx.h"

//4KBytes为一个Sector
//16个扇区为一个Block
//W25Q128
//容量为16M字节，共有128个Block，4096个Sector

//Private Functions

void W25QXX_pinCtrl(W25QXX_Pin pin, uint8_t status){ //BSP Function
    BitAction action = Bit_SET;
    if(status == 0){
        action = Bit_RESET;
    }
    if(pin == W25QXX_Pin_NSS){
        GPIO_WriteBit(GPIOB, GPIO_Pin_12, action);
    }
}

uint8_t W25QXX_SPIWriteReadByte(uint8_t txData){ //BSP Function
    uint8_t retry = 0;
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET){
        retry++;
        if(retry > 200){
            return 0;
        }
    }
    SPI_I2S_SendData(SPI2, txData);
    retry = 0;
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET){
        retry++;
        if(retry > 200){
            return 0;
        }
    }
    return SPI_I2S_ReceiveData(SPI2);
}

//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0，状态寄存器保护位，配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护位置
//WEL:写使能锁定
//BUSY:忙标志位(1,忙;0,空闲)
//默认:0x00
uint8_t W25QXX_readSR(void)   
{  
	uint8_t byte=0;   
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);      //使能器件
	W25QXX_SPIWriteReadByte(W25X_ReadStatusReg); //发送读取状态寄存器指令
	byte=W25QXX_SPIWriteReadByte(0Xff);          //读取一个字节
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);      //取消片选   
	return byte;   
} 


//写W25QXX状态寄存器
//ֻ只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void W25QXX_writeSR(uint8_t sr)   
{   
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);      //使能器件
	W25QXX_SPIWriteReadByte(W25X_WriteStatusReg);//发送写取状态寄存器命令
	W25QXX_SPIWriteReadByte(sr);               	//写入一个字节
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);      //取消片选    	      
}


//W25QXX写使能
//将WEL置位 
void W25QXX_writeEnable(void)   
{
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);    	//使能器件
    W25QXX_SPIWriteReadByte(W25X_WriteEnable); 	//发送写使能
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);     	//取消片选  	      
} 


//W25QXX写禁止
//将WEL清零
void W25QXX_writeDisable(void)   
{  
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);      //使能器件
    W25QXX_SPIWriteReadByte(W25X_WriteDisable);  //发送写禁止指令 
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);      //取消片选     	      
}

//等待空闲
void W25QXX_waitBusy(void)   
{   
	while((W25QXX_readSR()&0x01)==0x01);  		//等待BUSY位清空
}

//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0xFF，否则在非0xFF处写入的数据将失败！
//具有自动换页功能
//在指定地址开始写入指定长度的数据，但是要确保地址不越界！
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QXX_write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数	 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		W25QXX_writePage(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
}

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256)，该数不应该超过该页的剩余字节数！！！	 
void W25QXX_writePage(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
 	uint16_t i;  
    W25QXX_writeEnable();                  	//SET WEL 
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);      	//使能器件
    W25QXX_SPIWriteReadByte(W25X_PageProgram);      	//发送写页命令  
    W25QXX_SPIWriteReadByte((uint8_t)((WriteAddr)>>16)); 	//发送24bit地址
    W25QXX_SPIWriteReadByte((uint8_t)((WriteAddr)>>8));   
    W25QXX_SPIWriteReadByte((uint8_t)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)W25QXX_SPIWriteReadByte(pBuffer[i]);//循环写数 
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);      	//取消片选
	W25QXX_waitBusy();					   		//等待写入结束
}




//Public Functions

void W25QXX_init(void){  //BSP Function
    delay_init();
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI2, &SPI_InitStructure);
    SPI_Cmd(SPI2, ENABLE);
    
    W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);
}

//读取芯片ID
//返回值如下:			   
//0XEF13,表示芯片型号为W25Q80
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 	  
uint16_t W25QXX_readID(void)
{
	uint16_t Temp = 0;	  
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);
	W25QXX_SPIWriteReadByte(0x90);//发送读ID指令   
	W25QXX_SPIWriteReadByte(0x00); 	    
	W25QXX_SPIWriteReadByte(0x00); 	    
	W25QXX_SPIWriteReadByte(0x00); 	 			   
	Temp|=W25QXX_SPIWriteReadByte(0xFF)<<8;  
	Temp|=W25QXX_SPIWriteReadByte(0xFF);	 
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);
	return Temp;
}


//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void W25QXX_read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
 	uint16_t i;   										    
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);       	//使能器件 
    W25QXX_SPIWriteReadByte(W25X_ReadData);         	//发送读取命令  
    W25QXX_SPIWriteReadByte((uint8_t)((ReadAddr)>>16));  	//发送24bit地址 
    W25QXX_SPIWriteReadByte((uint8_t)((ReadAddr)>>8));   
    W25QXX_SPIWriteReadByte((uint8_t)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=W25QXX_SPIWriteReadByte(0XFF);   	//循环读数
    }
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);      
}


//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作！
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535) 
uint8_t W25QXX_BUFFER[4096];		 
void W25QXX_write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
	uint8_t * W25QXX_BUF;	  
   	W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小  
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		W25QXX_read(W25QXX_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//需要擦除 	  
		}
		if(i<secremain)//需要擦除
		{
			W25QXX_eraseSector(secpos);		//擦除这个扇区
			for(i=0;i<secremain;i++)	   		//复制
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			W25QXX_write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区

            }else W25QXX_write_NoCheck(pBuffer,WriteAddr,secremain);//写以已经删除了的，直接写入扇区剩余区间 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 

		   	pBuffer+=secremain;  				//指针偏移
			WriteAddr+=secremain;				//写地址偏移 
		   	NumByteToWrite-=secremain;			//字节数递减
			if(NumByteToWrite>4096)secremain=4096;//下一个扇区还是写不完
			else secremain=NumByteToWrite;		//下一个扇区可以写完了
		}	 
	};	 
}


//擦除整个芯片	  
//等待时间超长...
void W25QXX_eraseChip(void)   
{                                   
    W25QXX_writeEnable();                 	 	//SET WEL 
    W25QXX_waitBusy();   
  	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);        	//使能器件 
    W25QXX_SPIWriteReadByte(W25X_ChipErase);        	//发送片擦除命令
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);        	//取消片选   	      
	W25QXX_waitBusy();   				   		//等待芯片擦除结束
}


//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个扇区的最小时间:150ms
void W25QXX_eraseSector(uint32_t Dst_Addr)   
{  
	//监视Flash擦除情况   
 	Dst_Addr*=4096;
    W25QXX_writeEnable();                  	//SET WEL 	 
    W25QXX_waitBusy();   
  	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);       	//使能器件
    W25QXX_SPIWriteReadByte(W25X_SectorErase);      	//发送扇区擦除指令
    W25QXX_SPIWriteReadByte((uint8_t)((Dst_Addr)>>16));  	//发送24bit地址  
    W25QXX_SPIWriteReadByte((uint8_t)((Dst_Addr)>>8));   
    W25QXX_SPIWriteReadByte((uint8_t)Dst_Addr);  
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);      	//取消片选   	      
    W25QXX_waitBusy();   				   		//等待擦除完成
}


//进入掉电模式
void W25QXX_powerDown(void)   
{ 
  	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);     	 	//使能器件 
    W25QXX_SPIWriteReadByte(W25X_PowerDown);        //发送掉电指令
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);      	//取消片选     	      
    delay_us(3);                               //等待TPD
}


//唤醒
void W25QXX_wakeUp(void)   
{  
  	W25QXX_pinCtrl(W25QXX_Pin_NSS, 0);      	//使能器件 
    W25QXX_SPIWriteReadByte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
	W25QXX_pinCtrl(W25QXX_Pin_NSS, 1);      	//取消片选     	      
    delay_us(3);                            	//等待TRES1
}   
