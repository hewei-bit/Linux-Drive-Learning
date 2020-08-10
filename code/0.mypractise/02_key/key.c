//1.定义寄存器
#define  GPIOEOUT  (*(volatile unsigned int *)0xC001E000)
#define  GPIOCOUT  (*(volatile unsigned int *)0xC001C000)
#define  GPIOAOUT  (*(volatile unsigned int *)0xC001A000)
#define  GPIOBOUT  (*(volatile unsigned int *)0xC001B000)


#define GPIOADETMODE1 (*(volatile unsigned int *)0xC001A00C)
#define GPIOAINTENB (*(volatile unsigned int *)0xC001A010)
#define GPIOAPAD (*(volatile unsigned int *)0xC001A018)

#define GPIOBDETMODE1 (*(volatile unsigned int *)0xC001B00C)
#define GPIOBINTENB (*(volatile unsigned int *)0xC001B010)
#define GPIOBPAD (*(volatile unsigned int *)0xC001B018)

#define GPIOBOUTENB (*(volatile unsigned int *)0xC001B004)
#define GPIOBALTFN0  (*(volatile unsigned int *)0xC001B020)
#define GPIOBALTFN1  (*(volatile unsigned int *)0xC001B024)

#define GPIOAOUTENB (*(volatile unsigned int *)0xC001A004)
#define GPIOAALTFN0  (*(volatile unsigned int *)0xC001A020)
#define GPIOAALTFN1  (*(volatile unsigned int *)0xC001A024)

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

    //配置按键

    //配置GPIOA28输出使能为0
    GPIOAALTFN1 &= ~(3<<24);

    //配置GPIOB30
    GPIOBALTFN1 &=~(3<<28);
    GPIOBALTFN1 |=1<<28;
    //配置GPIO31
    GPIOBALTFN1 &=~(3<<30);
    GPIOBALTFN1 |=1<<30;
    
    //配置GPIO9
    GPIOBALTFN0 &=~(3<<18);

    GPIOAOUTENB &= ~(1<<28);
    GPIOBOUTENB &=~(1<<30);
    GPIOBOUTENB &=~(1<<31);
    GPIOBOUTENB &=~(1<<9);
    
	//配置LED
	//配置GPIOE13为输出模式
	GPIOEALTFN0&=~(3<<26);	//GPIOE13的多功能配置[27:26]清零

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

        if((GPIOBPAD & (1<<30)) == 0)
        {
    	    GPIOEOUT&=~(1<<13);
        }
        else
        {
	    	GPIOEOUT|=1<<13;
        }
        

        if((GPIOAPAD & (1<<28)) == 0)
        {
    	    GPIOCOUT&=~(1<<17);
        }
        else
        {
	    	GPIOCOUT|=1<<17;
        }

		//点亮
        if((GPIOBPAD & (1<<31)) == 0)
        {
	    	GPIOCOUT&=~(1<<8);
        }
        else
        {
	    	GPIOCOUT|=1<<8;
        }

        if((GPIOBPAD & (1<<9)) == 0)
        {
	    	GPIOCOUT&=~(1<<7);
        }
        else
        {
	    	GPIOCOUT|=1<<7;
        }
	}
}



void delay(void)
{
	volatile unsigned int i = 0x2000000;
	
	while(i--);
}
