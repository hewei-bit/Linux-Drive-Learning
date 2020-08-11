#include "stm32f4xx.h"
#include "sys.h"
#include <stdio.h>


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

void sr04_init(void)
{

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	
	//PB6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	//��6������ 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//���ŵ��ٶ����Ϊ100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//û��ʹ���ڲ���������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	


	//PE6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	//��6������ 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			//���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//���ŵ��ٶ����Ϊ100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//û��ʹ���ڲ���������
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	//ֻҪ�������ģʽ�����ţ��ͻ��г�ʼ�ĵ�ƽ״̬������Ҫ��ʱ��ͼ
	PBout(6)=0;
}


int32_t sr04_get_distance(void)
{
	uint32_t t=0;
	int32_t d=0;
	
	//����������ģ����й���
	PBout(6)=1;
	
	delay_us(10);
	
	PBout(6)=0;
	
	
	//�ȴ��źŷ���
	//��ӳ�ʱ�����Է�Ӳ�����⣨������ģ���Ӳ�����ϡ���·�����ó���һֱ�����ȴ�
	t=0;
	while(PEin(6)==0)
	{
		t++;
		
		delay_us(1);
		
		if(t >= 1000000)
			return -1;
	}
	
	//���źŷ���
	//��ӳ�ʱ�����Է�Ӳ�����⣨������ģ���Ӳ�����ϡ���·�����ó���һֱ�����ȴ�
	t=0;
	while(PEin(6))
	{
		t++;
		
		delay_us(9);		//����������9us����ͬ�ڴ�����3mm
		
		
		//���ߵ�ƽʱ���������
		if(t >= 1000000)
			return -2;	
	
	}

	//��ʱ�任��Ϊ����
	//3mm ����ʱ��ԼΪ 9us
	
	t=t/2;
	
	
	d= 3*t;


	return d;
}



int main(void)
{
	int32_t d=0;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	//��9 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//���ŵ��ٶ����Ϊ100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//û��ʹ���ڲ���������
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	PFout(9)=1;
	

	//����1��ʼ����������115200
	usart1_init(115200);
	
	
	printf("Hello Teacher.Wen\r\n");
	
	sr04_init();
	
	
	while(1)
	{
		d = sr04_get_distance();
		
		//�������ǺϷ�
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
