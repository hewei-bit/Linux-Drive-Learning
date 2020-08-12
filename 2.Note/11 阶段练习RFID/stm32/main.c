#include "stm32f4xx.h"
#include "sys.h"
#include <stdio.h>
#include "MFRC522.h" 

//GPIO初始化的结构体
static GPIO_InitTypeDef  		GPIO_InitStructure;


static USART_InitTypeDef 		USART_InitStructure;

static NVIC_InitTypeDef 		NVIC_InitStructure;

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
	
	
	//检查是否发送完毕
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	
	
	USART_SendData(USART1, ch);

	//返回要发送的字符
	return ch;
}

void _sys_exit(int return_code) {
label: goto label; /* endless loop */
}

//使用系统时钟的8分频作为系统定时器的时钟源
void delay_us(uint32_t n)
{

	SysTick->CTRL = 0; 		  // 关闭系统定时器

	SysTick->LOAD = n*21-1; //n微秒

	SysTick->VAL = 0; 		// 清空count标志位
	SysTick->CTRL = 1; 		// 使能系统定时器工作，时钟源为系统时钟168MHz/8=21MHz
	while ((SysTick->CTRL & 0x00010000)==0);// 等待count标志位置位		

	SysTick->CTRL = 0; 		// 关闭系统定时器
}

void delay_ms(uint32_t n)
{
	

	while(n--)
	{
		//1ms的延时
		SysTick->CTRL = 0; 		  // 关闭系统定时器
		
		SysTick->LOAD = 168000-1; // Count from 167999 to 0 (168000 cycles)
		
		SysTick->VAL = 0; 		// 清空count标志位
		SysTick->CTRL = 5; 		// 使能系统定时器工作，时钟源为系统时钟168MHz
		while ((SysTick->CTRL & 0x00010000)==0);// 等待count标志位置位
	}
	
	SysTick->CTRL = 0; 		// 关闭系统定时器
	
}


void usart1_init(uint32_t baud)
{

	//串口1硬件时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	
	//端口A硬件时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	
	
	//配置PA9和PA10为复用功能模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;	//第9 10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//复用功能模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	
	
	//PA9和PA10连接到串口1的硬件
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);	
	
	//串口1硬件参数：波特率、数据位、停止位、校验位.....
	USART_InitStructure.USART_BaudRate = baud;						//波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//8位数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;				//不需要校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不需要流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1,&USART_InitStructure);
	
	
	//使能串口1硬件工作
	USART_Cmd(USART1,ENABLE);


	//接收一个字节后，就触发中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);


	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
//MFRC522数据区
u8  mfrc552pidbuf[18];
u8  card_pydebuf[2];
u8  card_numberbuf[5];
u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
u8  card_readbuf[18];

//MFRC522测试函数
void MFRC522Test(void)
{
	u8 i,status,card_size;
	//
	status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
	//
	if(status==0)		//如果读到卡
	{
		status=MFRC522_Anticoll(card_numberbuf);			//防撞处理			
		card_size=MFRC522_SelectTag(card_numberbuf);	//选卡
		status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
		status=MFRC522_Write(4, card_writebuf);				//写卡（写卡要小心，特别是各区的块3）
		status=MFRC522_Read(4, card_readbuf);					//读卡
		//MFRC522_Halt();															//使卡进入休眠状态
		//卡类型显示

		printf("card_pydebuf:%x %x\r\n",card_pydebuf[0],card_pydebuf[1]);
		
		//卡序列号显示，最后一字节为卡的校验码
		printf("card_numberbuf:%x %x %x %x %x\r\n",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
		
		//卡容量显示，单位为Kbits
		printf("card_size:%x Kbits\r\n",card_size);
		
		
		//读卡状态显示，正常为0
		
		printf("status:%x\r\n",status);
		
		
		//读一个块的数据显示
		printf("card_readbuf:");
		for(i=0;i<18;i++)		//分两行显示
		{
			printf("%x ",card_readbuf[i]);
		}
		printf("\r\n");
		
		printf("================================\r\n\r\n");
		
		PFout(8)=1;PFout(9)=0;delay_ms(80);
		PFout(8)=0;PFout(9)=1;delay_ms(80);	

		PFout(8)=1;PFout(9)=0;delay_ms(80);
		PFout(8)=0;PFout(9)=1;delay_ms(80);		
	}	
}

int main(void)
{

	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;	//第8 9 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	PFout(8)=0;	
	PFout(9)=1;
	

	//串口1初始化，波特率115200
	usart1_init(115200);
	
	
	printf("This is spi rc522 test\r\n");
	
	MFRC522_Initializtion();			//初始化MFRC522
	
	
	while(1)
	{
		MFRC522_Initializtion();
		MFRC522Test();
		delay_ms(1000);
	}

}

void USART1_IRQHandler(void)
{
	uint8_t d=0;
	
	//检测标志位
	if(SET == USART_GetITStatus(USART1,USART_IT_RXNE))
	{
		//接收数据
		d=USART_ReceiveData(USART1);
		
		
		if(d == 'a')PFout(9)=0;
		if(d == 'A')PFout(9)=1;		
		
		
		//清空标志
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}



}
