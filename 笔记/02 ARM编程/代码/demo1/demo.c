#define GPIOEOUT		(*(volatile unsigned int *)0xC001E000)
#define GPIOEOUTENB		(*(volatile unsigned int *)0xC001E004)
#define GPIOEALTFN0		(*(volatile unsigned int *)0xC001E020)


void _start(void)
{
	//����GPIOE13ΪGPIOģʽ
	GPIOEALTFN0&=~(3<<26);		//��GPIOEALTFN0[27:26]����
	
	
	//����GPIOE13Ϊ�������
	GPIOEOUTENB|=1<<13;			//��GPIOEOUTENB[13]��1����bit13����Ϊ1
	
	//GPIOE13����͵�ƽ
	GPIOEOUT&=~(1<<13);			//��GPIOEOUT[13]����
	
	while(1);
}