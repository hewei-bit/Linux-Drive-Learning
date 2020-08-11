//1.定义寄存器
#define  GPIOEOUT  (*(volatile unsigned int *)0xC001E000)
#define  GPIOCOUT  (*(volatile unsigned int *)0xC001C000)

#define GPIOCOUTENB (*(volatile unsigned int *)0xC001C004)
#define GPIOCALTFN0  (*(volatile unsigned int *)0xC001C020)
#define GPIOCALTFN1  (*(volatile unsigned int *)0xC001C024)

#define GPIOEOUTENB (*(volatile unsigned int *)0xC001E004)
#define GPIOEALTFN0  (*(volatile unsigned int *)0xC001E020)
#define GPIOEALTFN1  (*(volatile unsigned int *)0xC001E024)


//D7->GPIOE13
//D8->GPIOC17
//D9->GPIOC8
//D10->GPIOC7

void delay(void);

void _start(void)
{
	
	//配置GPIOE13为输出模式
	GPIOEALTFN0&=~(3<<26);	//GPIOE13的多功能配置[27:26]清零
	//配置GPIOC17为输出模式
	GPIOCALTFN1&=~(3<<2);
	GPIOCALTFN1|=(1<<2);

	GPIOCALTFN0&=~(3<<14);
	GPIOCALTFN0|=(1<<14);

	GPIOCALTFN0&=~(3<<16);
	GPIOCALTFN0|=(1<<16);
	
	//允许GPIOE13输出电平
	GPIOEOUTENB|=1<<13;
	GPIOCOUTENB|=1<<17;
	GPIOCOUTENB|=1<<8;
	GPIOCOUTENB|=1<<7;
		
	while(1)
	{
		//点亮
		GPIOEOUT&=~(1<<13);
		GPIOCOUT&=~(1<<17);
		GPIOCOUT&=~(1<<8);
		GPIOCOUT&=~(1<<7);
			
		//延时一会
		delay();
		
		//熄灭
		GPIOEOUT|=1<<13;
		GPIOCOUT|=1<<17;
		GPIOCOUT|=1<<8;
		GPIOCOUT|=1<<7;
		
		//延时一会
		delay();
	}
}

void delay(void)
{
	volatile unsigned int i = 0x2000000;
	
	while(i--);
}
