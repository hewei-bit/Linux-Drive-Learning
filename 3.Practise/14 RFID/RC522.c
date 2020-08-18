#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include "MFRC522.h"

uint8_t rc522_send_byte(uint8_t byte)
{
	int32_t i=0;
	uint8_t d=0;
	
	
	for(i=7; i>=0; i--)
	{
		//对byte每个bit进行判断
		if(byte & (1<<i))
		{
			//MOSI引脚输出高电平
			gpio_set_value(rc522_MOSI, 1);
		}
		else
		{
		
			//MOSI引脚输出低电平
			gpio_set_value(rc522_MOSI, 0);	
		
		}
		
		//时钟线输出低电平
		gpio_set_value(rc522_SCK, 0);
		
		//延时一会，MOSI引脚已经发送到对方
		udelay(1);
		
	
		//时钟线输出高电平
		gpio_set_value(rc522_SCK, 1);
		
		//延时一会
		udelay(1);	
	
		//读取MISO引脚电平
		if(gpio_get_value(rc522_MISO))
			d|=1<<i;
	
	}
	
	
	return d;
}
void SPI3_Send(unsigned char val)  
{ 
	rc522_send_byte(val);
}
//
unsigned char SPI3_Receive(void)  
{ 

	return rc522_send_byte(0x00);
}
//功能描述向MFRC522的某一寄存器写一个字节数据
//输入参数addr--寄存器地址val--要写入的值
void Write_MFRC522(unsigned char addr, unsigned char val) 
{
	//地址格式0XXXXXX0  
	MFRC522_CS(0);   
	SPI3_Send((addr<<1)&0x7E);  
	SPI3_Send(val);    
	MFRC522_CS(1); 
}
//功能描述从MFRC522的某一寄存器读一个字节数据
//输入参数addr--寄存器地址
//返 回 值返回读取到的一个字节数据 
unsigned char Read_MFRC522(unsigned char addr) 
{  
	unsigned char val;
	//地址格式1XXXXXX0   
	MFRC522_CS(0);     
	SPI3_Send(((addr<<1)&0x7E)|0x80);   
	val=SPI3_Receive();    
	MFRC522_CS(1); 
	//   
	return val;  
}
//下面两个函数只对能读写位有效
//功能描述置RC522寄存器位
//输入参数reg--寄存器地址;mask--置位值
void SetBitMask(unsigned char reg, unsigned char mask)   
{     
	unsigned char tmp=0;
	//     
	tmp=Read_MFRC522(reg);     
	Write_MFRC522(reg,tmp|mask);  // set bit mask 
}
//功能描述清RC522寄存器位
//输入参数reg--寄存器地址;mask--清位值
void ClearBitMask(unsigned char reg, unsigned char mask)   
{     
	unsigned char tmp=0;
	//     
	tmp=Read_MFRC522(reg);     
	Write_MFRC522(reg,tmp&(~mask));  //clear bit mask 
}
//功能描述开启天线,每次启动或关闭天线发射之间应至少有1ms的间隔
void AntennaOn(void) 
{  
	unsigned char temp;
	//   
	temp=Read_MFRC522(TxControlReg);  
	if ((temp&0x03)==0)  
	{   
		SetBitMask(TxControlReg,0x03);  
	}
}
//功能描述关闭天线,每次启动或关闭天线发射之间应至少有1ms的间隔
void AntennaOff(void) 
{  
	ClearBitMask(TxControlReg,0x03);
}
//功能描述复位MFRC522
void MFRC522_Reset(void) 
{ 
	//外复位可以不用
	MFRC522_Rst(1);
	udelay(1);
	MFRC522_Rst(0);
	udelay(1);
	MFRC522_Rst(1);
	udelay(1);
	//内复位   
	Write_MFRC522(CommandReg, PCD_RESETPHASE); 
}
//
void MFRC522_Initializtion(void) 
{
	MFRC522_Reset();         
	//Timer: TPrescaler*TreloadVal/6.78MHz = 0xD3E*0x32/6.78=25ms     
	Write_MFRC522(TModeReg,0x8D);				//TAuto=1为自动计数模式，受通信协议影向。低4位为预分频值的高4位
	//Write_MFRC522(TModeReg,0x1D);				//TAutoRestart=1为自动重载计时，0x0D3E是0.5ms的定时初值//test    
	Write_MFRC522(TPrescalerReg,0x3E); 	//预分频值的低8位     
	Write_MFRC522(TReloadRegL,0x32);		//计数器的低8位                
	Write_MFRC522(TReloadRegH,0x00);		//计数器的高8位       
	Write_MFRC522(TxAutoReg,0x40); 			//100%ASK     
	Write_MFRC522(ModeReg,0x3D); 				//CRC初始值0x6363
	Write_MFRC522(CommandReg,0x00);			//启动MFRC522  
	//Write_MFRC522(RFCfgReg, 0x7F);    //RxGain = 48dB调节卡感应距离      
	AntennaOn();          							//打开天线 
}
//功能描述RC522和ISO14443卡通讯
//输入参数command--MF522命令字
//					sendData--通过RC522发送到卡片的数据
//					sendLen--发送的数据长度
//					BackData--接收到的卡片返回数据
//					BackLen--返回数据的位长度
//返 回 值成功返回MI_O
unsigned char MFRC522_ToCard(unsigned char command, unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned short int *backLen) 
{
	unsigned char  status=MI_ERR;
	unsigned char  irqEn=0x00;
	unsigned char  waitIRq=0x00;
	unsigned char  lastBits;
	unsigned char  n;
	unsigned short int i;
	//根据命预设中断参数
	switch (command)     
	{         
		case PCD_AUTHENT:  		//认证卡密   
			irqEn 	= 0x12;			//    
			waitIRq = 0x10;			//    
			break;
		case PCD_TRANSCEIVE: 	//发送FIFO中数据      
			irqEn 	= 0x77;			//    
			waitIRq = 0x30;			//    
			break;      
		default:    
			break;     
	}
	//
	Write_MFRC522(ComIEnReg, irqEn|0x80);		//允许中断请求     
	ClearBitMask(ComIrqReg, 0x80);  				//清除所有中断请求位               	
	SetBitMask(FIFOLevelReg, 0x80);  				//FlushBuffer=1, FIFO初始化
	Write_MFRC522(CommandReg, PCD_IDLE); 		//使MFRC522空闲   
	//向FIFO中写入数据     
	for (i=0; i<sendLen; i++)
		Write_MFRC522(FIFODataReg, sendData[i]);
	//执行命令
	Write_MFRC522(CommandReg, command);
	//天线发送数据     
	if (command == PCD_TRANSCEIVE)					//如果是卡片通信命令，MFRC522开始向天线发送数据      
		SetBitMask(BitFramingReg, 0x80);  		//StartSend=1,transmission of data starts      
	//等待接收数据完成     
	i = 10000; //i根据时钟频率调整操作M1卡最大等待时间25ms     
	do      
	{        
		n = Read_MFRC522(ComIrqReg);
		//irq_regdata=n;	//test         
		i--;
		//wait_count=i;		//test		     
	}while ((i!=0) && !(n&0x01) && !(n&waitIRq));	//接收完就退出n=0x64
	//停止发送
	ClearBitMask(BitFramingReg, 0x80);   		//StartSend=0
	//如果在25ms内读到卡
	if (i != 0)     
	{            
		if(!(Read_MFRC522(ErrorReg) & 0x1B)) //BufferOvfl Collerr CRCErr ProtecolErr         
		{            
			if (n & irqEn & 0x01)			//                  
				status = MI_NOTAGERR;		//
			//
			if (command == PCD_TRANSCEIVE)             
			{                 
				n = Read_MFRC522(FIFOLevelReg);		//n=0x02                
				lastBits = Read_MFRC522(ControlReg) & 0x07;	//lastBits=0               
				if (lastBits!=0)                         
					*backLen = (n-1)*8 + lastBits; 
				else
					*backLen = n*8;									//backLen=0x10=16
				//
				if (n == 0)                         
				 	n = 1;                        
				if (n > MAX_LEN)         
				 	n = MAX_LEN;
				//
				for (i=0; i<n; i++)                 
					backData[i] = Read_MFRC522(FIFODataReg); 
			}
			//
			status = MI_OK;		
		}
		else
			status = MI_ERR;
	}	
	//
	Write_MFRC522(ControlReg,0x80);				//timer stops     
	Write_MFRC522(CommandReg, PCD_IDLE);	//
	//
	return status;
}
//功能描述寻卡读取卡类型号
//输入参数reqMode--寻卡方式
//					TagType--返回卡片类型
//					0x4400 = Mifare_UltraLight
//					0x0400 = Mifare_One(S50)
//					0x0200 = Mifare_One(S70)
//					0x0800 = Mifare_Pro(X)
//					0x4403 = Mifare_DESFire
//返 回 值成功返回MI_OK	
unsigned char MFRC522_Request(unsigned char reqMode, unsigned char *TagType)
{  
	unsigned char  status;    
	unsigned short int backBits;   //接收到的数据位数
	//   
	Write_MFRC522(BitFramingReg, 0x07);  //TxLastBists = BitFramingReg[2..0]   
	TagType[0] = reqMode;  
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits); 
	// 
	if ((status != MI_OK) || (backBits != 0x10))  
	{       
		status = MI_ERR;
	}
	//  
	return status; 
}
//功能描述防冲突检测读取选中卡片的卡序列号
//输入参数serNum--返回4字节卡序列号,第5字节为校验字节
//返 回 值成功返回MI_OK
unsigned char MFRC522_Anticoll(unsigned char *serNum) 
{     
	unsigned char  status;     
	unsigned char  i;     
	unsigned char  serNumCheck=0;     
	unsigned short int unLen;
	//           
	ClearBitMask(Status2Reg, 0x08);  			//TempSensclear     
	ClearBitMask(CollReg,0x80);   				//ValuesAfterColl  
	Write_MFRC522(BitFramingReg, 0x00);  	//TxLastBists = BitFramingReg[2..0]
	serNum[0] = PICC_ANTICOLL1;     
	serNum[1] = 0x20;     
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
	//      
	if (status == MI_OK)
	{   
		//校验卡序列号   
		for(i=0;i<4;i++)   
			serNumCheck^=serNum[i];
		//
		if(serNumCheck!=serNum[i])        
			status=MI_ERR;
	}
	SetBitMask(CollReg,0x80);  //ValuesAfterColl=1
	//      
	return status;
}
//功能描述用MF522计算CRC
//输入参数pIndata--要读数CRC的数据len--数据长度pOutData--计算的CRC结果
void CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData) 
{     
	unsigned short int i;
	unsigned char  n;
	//      
	ClearBitMask(DivIrqReg, 0x04);   			//CRCIrq = 0     
	SetBitMask(FIFOLevelReg, 0x80);   		//清FIFO指针     
	Write_MFRC522(CommandReg, PCD_IDLE);   
	//向FIFO中写入数据      
	for (i=0; i<len; i++)
		Write_MFRC522(FIFODataReg, *(pIndata+i));
	//开始RCR计算
	Write_MFRC522(CommandReg, PCD_CALCCRC);
	//等待CRC计算完成     
	i = 1000;     
	do      
	{         
		n = Read_MFRC522(DivIrqReg);         
		i--;     
	}while ((i!=0) && !(n&0x04));   //CRCIrq = 1
	//读取CRC计算结果     
	pOutData[0] = Read_MFRC522(CRCResultRegL);     
	pOutData[1] = Read_MFRC522(CRCResultRegH);
	Write_MFRC522(CommandReg, PCD_IDLE);
}
//功能描述选卡读取卡存储器容量
//输入参数serNum--传入卡序列号
//返 回 值成功返回卡容量
unsigned char MFRC522_SelectTag(unsigned char *serNum) 
{     
	unsigned char  i;     
	unsigned char  status;     
	unsigned char  size;     
	unsigned short int recvBits;     
	unsigned char  buffer[9];
	//     
	buffer[0] = PICC_ANTICOLL1;	//防撞码1     
	buffer[1] = 0x70;
	buffer[6] = 0x00;						     
	for (i=0; i<4; i++)					
	{
		buffer[i+2] = *(serNum+i);	//buffer[2]-buffer[5]为卡序列号
		buffer[6]  ^=	*(serNum+i);	//卡校验码
	}
	//
	CalulateCRC(buffer, 7, &buffer[7]);	//buffer[7]-buffer[8]为RCR校验码
	ClearBitMask(Status2Reg,0x08);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
	//
	if ((status == MI_OK) && (recvBits == 0x18))    
		size = buffer[0];     
	else    
		size = 0;
	//	     
	return size; 
}
//功能描述验证卡片密码
//输入参数authMode--密码验证模式
//					0x60 = 验证A密钥
//					0x61 = 验证B密钥
//					BlockAddr--块地址
//					Sectorkey--扇区密码
//					serNum--卡片序列号4字节
//返 回 值成功返回MI_OK
unsigned char MFRC522_Auth(unsigned char authMode, unsigned char BlockAddr, unsigned char *Sectorkey, unsigned char *serNum) 
{     
	unsigned char  status;     
	unsigned short int recvBits;     
	unsigned char  i;  
	unsigned char  buff[12];    
	//验证模式+块地址+扇区密码+卡序列号     
	buff[0] = authMode;		//验证模式     
	buff[1] = BlockAddr;	//块地址     
	for (i=0; i<6; i++)
		buff[i+2] = *(Sectorkey+i);	//扇区密码
	//
	for (i=0; i<4; i++)
		buff[i+8] = *(serNum+i);		//卡序列号
	//
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
	//      
	if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
		status = MI_ERR;
	//
	return status;
}
//功能描述读块数据
//输入参数blockAddr--块地址;recvData--读出的块数据
//返 回 值成功返回MI_OK
unsigned char MFRC522_Read(unsigned char blockAddr, unsigned char *recvData) 
{     
	unsigned char  status;     
	unsigned short int unLen;
	//      
	recvData[0] = PICC_READ;     
	recvData[1] = blockAddr;     
	CalulateCRC(recvData,2, &recvData[2]);     
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
	//
	if ((status != MI_OK) || (unLen != 0x90))
		status = MI_ERR;
	//
	return status;
}
//功能描述写块数据
//输入参数blockAddr--块地址;writeData--向块写16字节数据
//返 回 值成功返回MI_OK
unsigned char MFRC522_Write(unsigned char blockAddr, unsigned char *writeData) 
{     
	unsigned char  status;     
	unsigned short int recvBits;     
	unsigned char  i;  
	unsigned char  buff[18];
	//           
	buff[0] = PICC_WRITE;     
	buff[1] = blockAddr;     
	CalulateCRC(buff, 2, &buff[2]);     
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
	//
	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
		status = MI_ERR;
	//
	if (status == MI_OK)     
	{         
		for (i=0; i<16; i++)  //向FIFO写16Byte数据                     
			buff[i] = *(writeData+i);
		//                     
		CalulateCRC(buff, 16, &buff[16]);         
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);           
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))               
			status = MI_ERR;         
	}          
	return status;
}
//功能描述命令卡片进入休眠状态
void MFRC522_Halt(void) 
{    
	unsigned short int unLen;     
	unsigned char  buff[4];
	//       
	buff[0] = PICC_HALT;     
	buff[1] = 0;     
	CalulateCRC(buff, 2, &buff[2]);       
	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);
}
//
