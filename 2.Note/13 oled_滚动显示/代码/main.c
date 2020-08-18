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
	SysTick->CTRL = 0; 			// Disable SysTick���ر�ϵͳ��ʱ��
	SysTick->LOAD = (168*n)-1; // ���ü���ֵ(168*n)-1 ~ 0
	SysTick->VAL  = 0; 		// Clear current value as well as count flag
	SysTick->CTRL = 5; 		// Enable SysTick timer with processor clock
	while ((SysTick->CTRL & 0x10000)==0);// Wait until count flag is set
	SysTick->CTRL = 0; 		// Disable SysTick	
}

void delay_ms(uint32_t n)
{
	while(n--)
	{
		SysTick->CTRL = 0; 				// Disable SysTick���ر�ϵͳ��ʱ��
		SysTick->LOAD = (168000)-1; 	// ���ü���ֵ(168000)-1 ~ 0
		SysTick->VAL  = 0; 		// Clear current value as well as count flag
		SysTick->CTRL = 5; 		// Enable SysTick timer with processor clock
		while ((SysTick->CTRL & 0x10000)==0);// Wait until count flag is set
	}
	
	SysTick->CTRL = 0; 		// Disable SysTick	

}

void usart1_init(uint32_t baud)
{
	
	//��PAӲ��ʱ��	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	

	//�򿪴���1Ӳ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	//����PA9��PA10Ϊ���ù���ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;		//��9 10������
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_AF;	//�๦��ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//��������������������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//������Ӧ
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//û��ʹ������������

	GPIO_Init(GPIOA,&GPIO_InitStructure);


	//��PA9��PA10�������ӵ�����1��Ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);	
	
	
	
	//���ô���1��ز����������ʡ���У��λ��8λ����λ��1��ֹͣλ......
	USART_InitStructure.USART_BaudRate = baud;										//������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�����շ�����
	USART_Init(USART1, &USART_InitStructure);
	
	
	//���ô���1���жϴ�������������һ���ֽڴ����ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	
	//���ô���1���ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	
	//ʹ�ܴ���1����
	USART_Cmd(USART1, ENABLE);
}


int main(void)
{
	int32_t i=0;

	
	
	//ʹ��(��)�˿�F��Ӳ��ʱ�ӣ����ǶԶ˿�F����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	//����1������:115200bps
	usart1_init(115200);	

	//��ʼ��GPIO����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		//��9������
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_OUT;	//���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//��������������������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//������Ӧ
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//û��ʹ������������

	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	PFout(9)=1;
	
	OLED_Init();			//��ʼ��OLED  
	OLED_Clear()  	; 
	
	//��ʾBMPͼƬ
	//x=0,y=0
	//ͼƬ��С����128���ص㣬��8��
	OLED_DrawBMP(0,0,128,8,BMP1);


	delay_ms(3000);

	while(1) 
	{		
		//�������ƶ�
		OLED_WR_Byte(0x2E,OLED_CMD);        //�رչ���
		OLED_WR_Byte(0x26,OLED_CMD);        //ˮƽ��������ҹ��� 26/27
		OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
		OLED_WR_Byte(0x00,OLED_CMD);        //��ʼҳ 0
		OLED_WR_Byte(0x07,OLED_CMD);        //����ʱ����
		OLED_WR_Byte(0x07,OLED_CMD);        //��ֹҳ 7
		OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
		OLED_WR_Byte(0xFF,OLED_CMD);        //�����ֽ�
		OLED_WR_Byte(0x2F,OLED_CMD);        //��������
			
		delay_ms(5000);
			
		//���ҵ����ƶ�
		OLED_WR_Byte(0x2E,OLED_CMD);        //�رչ���
		OLED_WR_Byte(0x27,OLED_CMD);        //ˮƽ��������ҹ��� 26/27
		OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
		OLED_WR_Byte(0x00,OLED_CMD);        //��ʼҳ 0
		OLED_WR_Byte(0x07,OLED_CMD);        //����ʱ����
		OLED_WR_Byte(0x07,OLED_CMD);        //��ֹҳ 7
		OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
		OLED_WR_Byte(0xFF,OLED_CMD);        //�����ֽ�
		OLED_WR_Byte(0x2F,OLED_CMD);        //��������	

		delay_ms(5000);	

		//���ô�ֱ��ˮƽ������
		OLED_WR_Byte(0x2e,OLED_CMD);        //�رչ���
		OLED_WR_Byte(0x29,OLED_CMD);        //ˮƽ��ֱ��ˮƽ�������� 29/2a
		OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
		OLED_WR_Byte(0x00,OLED_CMD);        //��ʼҳ 0
		OLED_WR_Byte(0x07,OLED_CMD);        //����ʱ����
		OLED_WR_Byte(0x07,OLED_CMD);        //��ֹҳ 1
		OLED_WR_Byte(0x01,OLED_CMD);        //��ֱ����ƫ����
		OLED_WR_Byte(0x2F,OLED_CMD);        //��������

		delay_ms(5000);	

		//���ô�ֱ��ˮƽ������
		OLED_WR_Byte(0x2e,OLED_CMD);        //�رչ���
		OLED_WR_Byte(0x2A,OLED_CMD);        //ˮƽ��ֱ��ˮƽ�������� 29/2a
		OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
		OLED_WR_Byte(0x00,OLED_CMD);        //��ʼҳ 0
		OLED_WR_Byte(0x07,OLED_CMD);        //����ʱ����
		OLED_WR_Byte(0x07,OLED_CMD);        //��ֹҳ 1
		OLED_WR_Byte(0x01,OLED_CMD);        //��ֱ����ƫ����
		OLED_WR_Byte(0x2F,OLED_CMD);        //��������

		delay_ms(5000);	
	}	
}



void USART1_IRQHandler(void)
{
	uint8_t d;
	
	//����־λ
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
	{
		//��������
		d=USART_ReceiveData(USART1);
		
		
	
		//��ձ�־λ
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}

}




