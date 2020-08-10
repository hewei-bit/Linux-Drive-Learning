#define GPIOEOUT		0xC001E000
#define GPIOEOUTENB		0xC001E004
#define GPIOEALTFN0		0xC001E020

.text							//代码段的开始
.global _start
_start:							//地址标签，也可以作为函数名
	//配置GPIOE13为GPIO模式
	ldr r0,=GPIOEALTFN0			//r0 = 0xC001E020
	ldr r1,[r0]					//r1 = *r0

	ldr r2,=3<<26				//r2 = 3<<26
	mvn r3,r2					//r3 = ~r2

	and r1,r1,r3				//r1 = r1 & r3

	str r1,[r0]					//*r0 = r1
	
	//配置GPIOE13为输出功能
	ldr r0,=GPIOEOUTENB			//r0 = 0xC001E004
	ldr r1,[r0]					//r1 = *r0
	
	ldr r2,=1<<13				//r2 = 1<<13
	
	orr r1,r1,r2				//r1 = r1|r2
	
	str r1,[r0]					//*r0 = r1
	
	//GPIOE13输出低电平
	ldr r0,=GPIOEOUT			//r0 = 0xC001E000
	ldr r1,[r0]					//r1 = *r0

	ldr r2,=1<<13				//r2 = 1<<13
	mvn r3,r2					//r3 = ~r2

	and r1,r1,r3				//r1 = r1 & r3

	str r1,[r0]					//*r0 = r1	
	
loop:							//地址标签

	b   loop					//跳转到loop

.end							//代码段的结束