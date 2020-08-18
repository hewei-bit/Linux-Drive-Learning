#include "stm32f4xx.h"
#include "sys.h"
#include <stdio.h>
#include <string.h>
#include "oled.h"
#include "bmp.h"

static GPIO_InitTypeDef 		GPIO_InitStructure;
static NVIC_InitTypeDef 		NVIC_InitStructure;
static USART_InitTypeDef 		USART_InitStructure;


#pragma import(__use_no_semihosting_swi)
struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
	
	USART_SendData(USART1,ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	USART_ClearFlag(USART1,USART_FLAG_TXE);
	
	
	return ch;
}
void _sys_exit(int return_code) {

}

void delay_us(uint32_t n)
{
	SysTick->CTRL = 0; 			// Disable SysTick，关闭系统定时器
	SysTick->LOAD = (168*n)-1; // 配置计数值(168*n)-1 ~ 0
	SysTick->VAL  = 0; 		// Clear current value as well as count flag
	SysTick->CTRL = 5; 		// Enable SysTick timer with processor clock
	while ((SysTick->CTRL & 0x10000)==0);// Wait until count flag is set
	SysTick->CTRL = 0; 		// Disable SysTick	
}

void delay_ms(uint32_t n)
{
	while(n--)
	{
		SysTick->CTRL = 0; 				// Disable SysTick，关闭系统定时器
		SysTick->LOAD = (168000)-1; 	// 配置计数值(168000)-1 ~ 0
		SysTick->VAL  = 0; 		// Clear current value as well as count flag
		SysTick->CTRL = 5; 		// Enable SysTick timer with processor clock
		while ((SysTick->CTRL & 0x10000)==0);// Wait until count flag is set
	}
	
	SysTick->CTRL = 0; 		// Disable SysTick	

}

void usart1_init(uint32_t baud)
{
	
	//打开PA硬件时钟	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	

	//打开串口1硬件时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	//配置PA9和PA10为复用功能模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;		//第9 10根引脚
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_AF;	//多功能模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推挽输出，增加输出电流能力。
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//高速响应
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//没有使能上下拉电阻

	GPIO_Init(GPIOA,&GPIO_InitStructure);


	//将PA9和PA10引脚连接到串口1的硬件
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);	
	
	
	
	//配置串口1相关参数：波特率、无校验位、8位数据位、1个停止位......
	USART_InitStructure.USART_BaudRate = baud;										//波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8位数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//允许收发数据
	USART_Init(USART1, &USART_InitStructure);
	
	
	//配置串口1的中断触发方法：接收一个字节触发中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	
	//配置串口1的中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	
	//使能串口1工作
	USART_Cmd(USART1, ENABLE);
}


int main(void)
{
	int32_t i=0;

	
	
	//使能(打开)端口F的硬件时钟，就是对端口F供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	//串口1波特率:115200bps
	usart1_init(115200);	

	//初始化GPIO引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		//第9根引脚
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_OUT;	//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推挽输出，增加输出电流能力。
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//高速响应
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//没有使能上下拉电阻

	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	PFout(9)=1;
	
	OLED_Init();			//初始化OLED  
	OLED_Clear()  	; 
	
	//显示BMP图片
	//x=0,y=0
	//图片大小：宽128像素点，高8行
	OLED_DrawBMP(0,0,128,8,BMP1);


	delay_ms(3000);

	while(1) 
	{		
		//从左到右移动
		OLED_WR_Byte(0x2E,OLED_CMD);        //关闭滚动
		OLED_WR_Byte(0x26,OLED_CMD);        //水平向左或者右滚动 26/27
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x00,OLED_CMD);        //起始页 0
		OLED_WR_Byte(0x07,OLED_CMD);        //滚动时间间隔
		OLED_WR_Byte(0x07,OLED_CMD);        //终止页 7
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0xFF,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x2F,OLED_CMD);        //开启滚动
			
		delay_ms(5000);
			
		//从右到左移动
		OLED_WR_Byte(0x2E,OLED_CMD);        //关闭滚动
		OLED_WR_Byte(0x27,OLED_CMD);        //水平向左或者右滚动 26/27
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x00,OLED_CMD);        //起始页 0
		OLED_WR_Byte(0x07,OLED_CMD);        //滚动时间间隔
		OLED_WR_Byte(0x07,OLED_CMD);        //终止页 7
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0xFF,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x2F,OLED_CMD);        //开启滚动	

		delay_ms(5000);	

		//设置垂直和水平滚动的
		OLED_WR_Byte(0x2e,OLED_CMD);        //关闭滚动
		OLED_WR_Byte(0x29,OLED_CMD);        //水平垂直和水平滚动左右 29/2a
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x00,OLED_CMD);        //起始页 0
		OLED_WR_Byte(0x07,OLED_CMD);        //滚动时间间隔
		OLED_WR_Byte(0x07,OLED_CMD);        //终止页 1
		OLED_WR_Byte(0x01,OLED_CMD);        //垂直滚动偏移量
		OLED_WR_Byte(0x2F,OLED_CMD);        //开启滚动

		delay_ms(5000);	

		//设置垂直和水平滚动的
		OLED_WR_Byte(0x2e,OLED_CMD);        //关闭滚动
		OLED_WR_Byte(0x2A,OLED_CMD);        //水平垂直和水平滚动左右 29/2a
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x00,OLED_CMD);        //起始页 0
		OLED_WR_Byte(0x07,OLED_CMD);        //滚动时间间隔
		OLED_WR_Byte(0x07,OLED_CMD);        //终止页 1
		OLED_WR_Byte(0x01,OLED_CMD);        //垂直滚动偏移量
		OLED_WR_Byte(0x2F,OLED_CMD);        //开启滚动

		delay_ms(5000);	
	}	
}



void USART1_IRQHandler(void)
{
	uint8_t d;
	
	//检测标志位
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
	{
		//接收数据
		d=USART_ReceiveData(USART1);
		
		
	
		//清空标志位
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}

}




