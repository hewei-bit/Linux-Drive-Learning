#define GPIOBPAD		(*(volatile unsigned int *)0xC001B018)
#define GPIOBOUTENB		(*(volatile unsigned int *)0xC001B004)
#define GPIOBALTFN0		(*(volatile unsigned int *)0xC001B020)
#define GPIOBALTFN1		(*(volatile unsigned int *)0xC001B024)

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
	
	
	
	//����GPIOB30ΪGPIOģʽ
	GPIOBALTFN1&=~(3<<28);		//��GPIOBALTFN1[29:28]����
	GPIOBALTFN1|=1<<28;			//��GPIOBALTFN1[28]��1����bit28����Ϊ1
	
	//����GPIOB30Ϊ���빦��
	GPIOBOUTENB&=~(1<<30);		//��GPIOBOUTENB[30]����
	
	while(1)
	{
		//���˿�B�ĵ�30�������Ƿ�Ϊ�͵�ƽ
		if((GPIOBPAD & (1<<30)) == 0)
		{
			//����D7��
			GPIOEOUT&=~(1<<13);
			
			
		}
		else
		{
			//Ϩ��D7��
			GPIOEOUT|=(1<<13);

		}
		
		
		
		
		
	}
}