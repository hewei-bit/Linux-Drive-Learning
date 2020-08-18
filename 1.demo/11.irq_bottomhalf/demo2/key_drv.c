#include <linux/module.h>
#include <linux/init.h>
#include <mach/devices.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>

//分配工作
struct work_struct mywork;

int dev_irq;

//工作处理函数
void mywork_handler(struct work_struct *work)
{
	int key_val;
	
	//睡眠10ms
	msleep(10);
	
	if(dev_irq == IRQ_GPIO_A_START + 28)
	{
		//读取管脚的电平
		key_val = gpio_get_value(PAD_GPIO_A+28);
		
		if(key_val == 0)
		{
			printk("k2 pressed\n");
		}		
	}
}

//中断服务函数
irqreturn_t keys_irq_handler(int irq, void * dev)
{
	//保存当前中断号
	dev_irq = irq;
	
	//登记工作
	schedule_work(&mywork);
	
	
	//已经完成中断
	return IRQ_HANDLED;
}


//入口函数
static int __init gec6818_key_init(void)
{
	int rt;
	
	//为K2按键申请中断，也就是为GPIOA28申请中断
	//中断服务函数：keys_irq_handler
	//下降沿触发：IRQF_TRIGGER_FALLING
	//申请中断的名字:gpioa28
	//是否传递参数给中断服务函数：NULL，就不传递
	
	rt = request_irq(IRQ_GPIO_A_START + 28,
		keys_irq_handler,
		IRQF_TRIGGER_FALLING,
		"gpioa28",
		NULL);
	
	if(rt)
	{
		printk("request_irq error\n");
		return rt;
	}
	
	INIT_WORK(&mywork, mywork_handler);
	
	printk("gec6818 key init\n");
	
	return 0;
}


//出口函数
static void __exit gec6818_key_exit(void)
{
	//注销工作队列
	cancel_work_sync(&mywork);
	
	//释放GPIOA28中断
	free_irq(IRQ_GPIO_A_START + 28,NULL);
	printk("gec6818 key exit\n");	
}

//驱动程序的入口:安装驱动的时候，会调用module_init这个函数，这个函数又调用gec6818_key_init
module_init(gec6818_key_init);


//驱动程序的出口:卸载驱动的时候，会调用module_exit这个函数，这个函数又调用gec6818_key_exit
module_exit(gec6818_key_exit);

MODULE_AUTHOR("Teacher.Wen");
MODULE_DESCRIPTION("gec6818 led driver");
MODULE_LICENSE("GPL");			//遵循GPL协议，一定要添加，否则在开发板安装驱动会出问题