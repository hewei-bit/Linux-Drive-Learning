#define GPIOCOUT		(*(volatile unsigned int *)0xC001C000)
#define GPIOCOUTENB		(*(volatile unsigned int *)0xC001C004)
#define GPIOCALTFN0		(*(volatile unsigned int *)0xC001C020)
#define GPIOCALTFN1		(*(volatile unsigned int *)0xC001C024)

#define GPIOEOUT		(*(volatile unsigned int *)0xC001E000)
#define GPIOEOUTENB		(*(volatile unsigned int *)0xC001E004)
#define GPIOEALTFN0		(*(volatile unsigned int *)0xC001E020)

void delay(void)
{
	volatile unsigned int i=0x2000000;
	
	while(i--);	
}


void _start(void)
{
	//����GPIOE13ΪGPIOģʽ
	GPIOEALTFN0&=~(3<<26);		//��GPIOEALTFN0[27:26]����
	
	//����GPIOE13Ϊ�������
	GPIOEOUTENB|=1<<13;			//��GPIOEOUTENB[13]��1����bit13����Ϊ1
	
	//GPIOE13����͵�ƽ
	GPIOEOUT&=~(1<<13);			//��GPIOEOUT[13]����
	
	
	
	//����GPIOC17ΪGPIOģʽ
	GPIOCALTFN1&=~(3<<2);		//��GPIOCALTFN1[3:2]����
	GPIOCALTFN1|=  1<<2;		//��GPIOCALTFN1[2]��1    01
	
	//����GPIOC17Ϊ�������
	GPIOCOUTENB|=1<<17;			//��GPIOCOUTENB[17]��1����bit17����Ϊ1
	
	//GPIOC17����͵�ƽ
	GPIOCOUT&=~(1<<17);			//��GPIOCOUT[17]����	
	
	while(1)
	{
		//��ȫ��
		GPIOEOUT&=~(1<<13);
		GPIOCOUT&=~(1<<17);
		
		//��ʱһ��
		delay();
		
		//��ȫ��
		GPIOEOUT|=(1<<13);
		GPIOCOUT|=(1<<17);
		
		//��ʱһ��
		delay();		
	}
}



