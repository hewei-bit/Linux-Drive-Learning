#define GPIOBPAD		(*(volatile unsigned int *)0xC001B018)
#define GPIOBOUTENB		(*(volatile unsigned int *)0xC001B004)
#define GPIOBALTFN0		(*(volatile unsigned int *)0xC001B020)
#define GPIOBALTFN1		(*(volatile unsigned int *)0xC001B024)

#define GPIOEOUT		(*(volatile unsigned int *)0xC001E000)
#define GPIOEOUTENB		(*(volatile unsigned int *)0xC001E004)
#define GPIOEALTFN0		(*(volatile unsigned int *)0xC001E020)


void _start(void)
{
	//配置GPIOE13为GPIO模式
	GPIOEALTFN0&=~(3<<26);		//将GPIOEALTFN0[27:26]清零
	
	
	//配置GPIOE13为输出功能
	GPIOEOUTENB|=1<<13;			//将GPIOEOUTENB[13]置1，将bit13设置为1
	
	//GPIOE13输出低电平
	GPIOEOUT&=~(1<<13);			//将GPIOEOUT[13]清零
	
	
	
	//配置GPIOB30为GPIO模式
	GPIOBALTFN1&=~(3<<28);		//将GPIOBALTFN1[29:28]清零
	GPIOBALTFN1|=1<<28;			//将GPIOBALTFN1[28]置1，将bit28设置为1
	
	//配置GPIOB30为输入功能
	GPIOBOUTENB&=~(1<<30);		//将GPIOBOUTENB[30]清零
	
	while(1)
	{
		//检测端口B的第30根引脚是否为低电平
		if((GPIOBPAD & (1<<30)) == 0)
		{
			//点亮D7灯
			GPIOEOUT&=~(1<<13);
			
			
		}
		else
		{
			//熄灭D7灯
			GPIOEOUT|=(1<<13);

		}
		
		
		
		
		
	}
}