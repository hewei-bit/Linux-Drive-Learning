#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>		//字符设备
#include <linux/fs.h>		//file_operations
#include <linux/uaccess.h>	//copy_from_user、copy_to_user
#include <linux/device.h>	//class_create、device_create
#include <linux/ioport.h>	//request_mem_region 
#include <linux/io.h>		//ioremap
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/miscdevice.h>//misc_register
#include <linux/ioctl.h>
#include <mach/devices.h>     //中断号
#include <linux/interrupt.h>  //常用的函数接口
#include <linux/delay.h>


void  mytasklet_handler(unsigned long data)
{
	int key_val=0;
	
	//得到中断号
	int irq = (int)data;
	
	if(irq == IRQ_GPIO_A_START+28)	
	{
		
		mdelay(10);
		
		key_val = gpio_get_value(PAD_GPIO_A+28);
		
		printk("k2 %s\n",key_val?"released":"pressed");
	}	
	
}

//静态创建小任务tasklet
DECLARE_TASKLET(mytasklet,mytasklet_handler, 0);


//中断服务函数
//irq，就是当前触发中断请求的中断号
//dev，就是request_irq传递的参数
irqreturn_t keys_irq_handler(int irq, void *dev)
{
	
	//if(irq == gpio_to_irq(PAD_GPIO_A+28))
	
	//私有传递中断号
	mytasklet.data = irq;
	
	//登记tasklet
	tasklet_schedule(&mytasklet);  

	
	return IRQ_HANDLED;//当前中断处理已经完成
}




static int __init mykey_init(void)
{
	int rt;
	
	//中断号：gpio_to_irq(PAD_GPIO_A+28)
	//中断服务函数：keys_irq_handler
	//触发方式：下降沿触发，IRQF_TRIGGER_FALLING
	//注册名字："gpioa28"
	//传递参数：NULL
	//rt = request_irq(gpio_to_irq(PAD_GPIO_A+28),keys_irq_handler,IRQF_TRIGGER_FALLING,"gpioa28",NULL);
	rt = request_irq(IRQ_GPIO_A_START+28,keys_irq_handler,IRQF_TRIGGER_FALLING,"gpioa28",NULL);
	
	
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request_irq;
		
	}

	
	printk(KERN_INFO"mykey_init\n");

	//成功返回
	return 0;
	
err_request_irq:
	return rt;
}


static void __exit mykey_exit(void)
{
	//释放中断
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
	
	printk(KERN_INFO"mykey_exit\n");
}

//加载驱动：当使用insmod key_drv.ko的时候，会调用module_init函数，module_init函数会回调mykey_init函数
module_init(mykey_init);

//卸载驱动：当使用rmmod key_drv的时候，会调用module_exit函数，module_exit函数会回调mykey_exit函数
module_exit(mykey_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Teacher.Wen");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is key irq driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");