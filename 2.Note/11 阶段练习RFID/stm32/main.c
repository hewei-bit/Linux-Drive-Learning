#include "stm32f4xx.h"
#include "sys.h"
#include <stdio.h>
#include "MFRC522.h" 

//GPIO��ʼ���Ľṹ��
static GPIO_InitTypeDef  		GPIO_InitStructure;


static USART_InitTypeDef 		USART_InitStructure;

static NVIC_InitTypeDef 		NVIC_InitStructure;

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
	
	
	//����Ƿ������
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	
	
	USART_SendData(USART1, ch);

	//����Ҫ���͵��ַ�
	return ch;
}

void _sys_exit(int return_code) {
label: goto label; /* endless loop */
}

//ʹ��ϵͳʱ�ӵ�8��Ƶ��Ϊϵͳ��ʱ����ʱ��Դ
void delay_us(uint32_t n)
{

	SysTick->CTRL = 0; 		  // �ر�ϵͳ��ʱ��

	SysTick->LOAD = n*21-1; //n΢��

	SysTick->VAL = 0; 		// ���count��־λ
	SysTick->CTRL = 1; 		// ʹ��ϵͳ��ʱ��������ʱ��ԴΪϵͳʱ��168MHz/8=21MHz
	while ((SysTick->CTRL & 0x00010000)==0);// �ȴ�count��־λ��λ		

	SysTick->CTRL = 0; 		// �ر�ϵͳ��ʱ��
}

void delay_ms(uint32_t n)
{
	

	while(n--)
	{
		//1ms����ʱ
		SysTick->CTRL = 0; 		  // �ر�ϵͳ��ʱ��
		
		SysTick->LOAD = 168000-1; // Count from 167999 to 0 (168000 cycles)
		
		SysTick->VAL = 0; 		// ���count��־λ
		SysTick->CTRL = 5; 		// ʹ��ϵͳ��ʱ��������ʱ��ԴΪϵͳʱ��168MHz
		while ((SysTick->CTRL & 0x00010000)==0);// �ȴ�count��־λ��λ
	}
	
	SysTick->CTRL = 0; 		// �ر�ϵͳ��ʱ��
	
}


void usart1_init(uint32_t baud)
{

	//����1Ӳ��ʱ��ʹ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	
	//�˿�AӲ��ʱ��ʹ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	
	
	//����PA9��PA10Ϊ���ù���ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;	//��9 10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//���ù���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//���ŵ��ٶ����Ϊ100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//û��ʹ���ڲ���������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	
	
	//PA9��PA10���ӵ�����1��Ӳ��
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);	
	
	//����1Ӳ�������������ʡ�����λ��ֹͣλ��У��λ.....
	USART_InitStructure.USART_BaudRate = baud;						//������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//1λֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;				//����ҪУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//����Ҫ������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1,&USART_InitStructure);
	
	
	//ʹ�ܴ���1Ӳ������
	USART_Cmd(USART1,ENABLE);


	//����һ���ֽں󣬾ʹ����ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);


	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
//MFRC522������
u8  mfrc552pidbuf[18];
u8  card_pydebuf[2];
u8  card_numberbuf[5];
u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
u8  card_readbuf[18];

//MFRC522���Ժ���
void MFRC522Test(void)
{
	u8 i,status,card_size;
	//
	status=MFRC522_Request(0x52, card_pydebuf);			//Ѱ��
	//
	if(status==0)		//���������
	{
		status=MFRC522_Anticoll(card_numberbuf);			//��ײ����			
		card_size=MFRC522_SelectTag(card_numberbuf);	//ѡ��
		status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//�鿨
		status=MFRC522_Write(4, card_writebuf);				//д����д��ҪС�ģ��ر��Ǹ����Ŀ�3��
		status=MFRC522_Read(4, card_readbuf);					//����
		//MFRC522_Halt();															//ʹ����������״̬
		//��������ʾ

		printf("card_pydebuf:%x %x\r\n",card_pydebuf[0],card_pydebuf[1]);
		
		//�����к���ʾ�����һ�ֽ�Ϊ����У����
		printf("card_numberbuf:%x %x %x %x %x\r\n",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
		
		//��������ʾ����λΪKbits
		printf("card_size:%x Kbits\r\n",card_size);
		
		
		//����״̬��ʾ������Ϊ0
		
		printf("status:%x\r\n",status);
		
		
		//��һ�����������ʾ
		printf("card_readbuf:");
		for(i=0;i<18;i++)		//��������ʾ
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
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;	//��8 9 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//���ŵ��ٶ����Ϊ100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//û��ʹ���ڲ���������
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	PFout(8)=0;	
	PFout(9)=1;
	

	//����1��ʼ����������115200
	usart1_init(115200);
	
	
	printf("This is spi rc522 test\r\n");
	
	MFRC522_Initializtion();			//��ʼ��MFRC522
	
	
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
	
	//����־λ
	if(SET == USART_GetITStatus(USART1,USART_IT_RXNE))
	{
		//��������
		d=USART_ReceiveData(USART1);
		
		
		if(d == 'a')PFout(9)=0;
		if(d == 'A')PFout(9)=1;		
		
		
		//��ձ�־
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}



}
