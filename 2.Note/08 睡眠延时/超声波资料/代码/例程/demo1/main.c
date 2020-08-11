#include "stm32f4xx.h"
#include "sys.h"
#include <stdio.h>


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

void sr04_init(void)
{

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	
	//PB6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	//第6根引脚 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);	


	//PE6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	//第6根引脚 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	//只要是有输出模式的引脚，就会有初始的电平状态，必须要看时序图
	PBout(6)=0;
}


int32_t sr04_get_distance(void)
{
	uint32_t t=0;
	int32_t d=0;
	
	//触发超声波模块进行工作
	PBout(6)=1;
	
	delay_us(10);
	
	PBout(6)=0;
	
	
	//等待信号返回
	//添加超时处理，以防硬件问题（超声波模块的硬件故障、短路现象）让程序一直阻塞等待
	t=0;
	while(PEin(6)==0)
	{
		t++;
		
		delay_us(1);
		
		if(t >= 1000000)
			return -1;
	}
	
	//有信号返回
	//添加超时处理，以防硬件问题（超声波模块的硬件故障、短路现象）让程序一直阻塞等待
	t=0;
	while(PEin(6))
	{
		t++;
		
		delay_us(9);		//超声波传输9us，等同于传输了3mm
		
		
		//若高电平时间持续过长
		if(t >= 1000000)
			return -2;	
	
	}

	//将时间换算为距离
	//3mm 传输时间约为 9us
	
	t=t/2;
	
	
	d= 3*t;


	return d;
}



int main(void)
{
	int32_t d=0;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	//第9 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	PFout(9)=1;
	

	//串口1初始化，波特率115200
	usart1_init(115200);
	
	
	printf("Hello Teacher.Wen\r\n");
	
	sr04_init();
	
	
	while(1)
	{
		d = sr04_get_distance();
		
		//若距离是合法
		if(d>=20  && d<=4000)
		{
			printf("distance=%dmm\r\n",d);
		
		}
		
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
