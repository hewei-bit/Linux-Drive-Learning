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
#include <linux/ioctl.h>
#include <linux/sched.h>

#define CMD_KEY_GET		_IOR('K',0,unsigned int *)

//声明一个等待条件，真和假
static int key_press_flag = 0;

//声明一个等待队列
static wait_queue_head_t key_wq;


static unsigned int key_val=0;

//分配工作
struct work_struct mywork;

int dev_irq;


//工作处理函数
void mywork_handler(struct work_struct *work)
{
	
	
	//读取管脚的电平
	if(dev_irq == IRQ_GPIO_A_START + 28)
	{
		//[可选]延时去抖动
		mdelay(15);
		if(!(gpio_get_value(PAD_GPIO_A + 28)))
		{
			key_val|=0x01<<0;		
			//设置条件为真
			key_press_flag=1;	
			//唤醒队列
			wake_up(&key_wq);
		}
	}
	if(dev_irq == IRQ_GPIO_B_START+9)	
	{
		//[可选]延时去抖动
		mdelay(15);
		if(!(gpio_get_value(PAD_GPIO_B + 9)))
		{
			key_val|=0x01<<1;		
			//设置条件为真
			key_press_flag=1;	
			//唤醒队列
			wake_up(&key_wq);
		}
	}
	if(dev_irq == IRQ_GPIO_B_START+30)	
	{
		//[可选]延时去抖动
		mdelay(15);
		if(!(gpio_get_value(PAD_GPIO_B + 30)))
		{
			key_val|=0x01<<2;		
			//设置条件为真
			key_press_flag=1;	
			//唤醒队列
			wake_up(&key_wq);
		}
	}
	if(dev_irq == IRQ_GPIO_B_START+31)	
	{
		//[可选]延时去抖动
		mdelay(15);
		if(!(gpio_get_value(PAD_GPIO_B + 31)))
		{
			key_val|=0x01<<3;		
			//设置条件为真
			key_press_flag=1;	
			//唤醒队列
			wake_up(&key_wq);
		}
	}
}

//分配初始化tasklet
//DECLARE_TASKLET(mytasklet,mytasklet_handler,0);

int key_open (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"key_open\n");
	return 0;
}



int key_close (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"key_close\n");
	
	
	return 0;
}

long key_ioctl (struct file *filp, unsigned int cmd, unsigned long args)
{
	void __user *arpg = (void __user *)args;
	
	int rt;

	switch(cmd)
	{
		
		case CMD_KEY_GET:
		{
			key_val=0;
			//等待按键的事件，可中断睡眠
			wait_event_interruptible(key_wq,key_press_flag);
			key_press_flag=0;
			
			rt = copy_to_user(arpg,&key_val,sizeof key_val);
			
			key_val=0;
			
			if(rt != 0)
				return -ENOMEM;
			
		}break;
		
		default:
			return -ENOIOCTLCMD;
	}
	
	
	return 0;
}


struct file_operations key_fops={
	.owner = THIS_MODULE,
	.open = key_open,
	.release = key_close,
	.unlocked_ioctl = key_ioctl,
};


static struct miscdevice key_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "mykey", //设备名称,/dev/mykey
	.fops = &key_fops,//文件操作集
};

//中断服务函数
//irq，就是当前触发中断请求的中断号
//dev，就是request_irq传递的参数
irqreturn_t keys_irq_handler(int irq, void *dev)
{
	//mytasklet.data = (unsigned long)irq;

	//登记tasklet
	//tasklet_schedule(&mytasklet);	
	
		//保存当前中断号
	dev_irq = irq;
	
	//登记工作
	schedule_work(&mywork);
	
	return IRQ_HANDLED;//当前中断处理已经完成
}

static int __init mykey_init(void)
{
	int rt;
	
	//混杂设备的注册
	rt = misc_register(&key_miscdev);
	
	if(rt < 0)
	{
		printk(KERN_INFO"misc_register error\n");
		
		goto err_misc_register;
		
	}	
	
	//中断号：gpio_to_irq(PAD_GPIO_A+28)
	//中断服务函数：keys_irq_handler
	//触发方式：下降沿触发，IRQF_TRIGGER_FALLING
	//注册名字："gpioa28"
	//传递参数：NULL
	//rt = request_irq(gpio_to_irq(PAD_GPIO_A+28),keys_irq_handler,IRQF_TRIGGER_FALLING,"gpioa28",NULL);
	rt = request_irq(gpio_to_irq(PAD_GPIO_A+28),keys_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpioa28",NULL);
	
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request1_irq;
		
	}
	rt = request_irq(gpio_to_irq(PAD_GPIO_B+30),keys_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob30",NULL);
	
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request2_irq;
		
	}
	rt = request_irq(gpio_to_irq(PAD_GPIO_B+31),keys_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob31",NULL);
	
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request3_irq;
		
	}
	rt = request_irq(gpio_to_irq(PAD_GPIO_B+9),keys_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob9",NULL);
	
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request4_irq;
		
	}

	INIT_WORK(&mywork, mywork_handler);
	
	//初始化等待队列
	init_waitqueue_head(&key_wq);
	
	
	printk(KERN_INFO"mykey_init\n");

	//成功返回
	return 0;
err_request4_irq:
free_irq(gpio_to_irq(PAD_GPIO_B+31),NULL);
err_request3_irq:
free_irq(gpio_to_irq(PAD_GPIO_B+30),NULL);
err_request2_irq:
free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
err_request1_irq:
misc_deregister(&key_miscdev);
err_misc_register:
	return rt;
}


static void __exit mykey_exit(void)
{
	//释放中断
	free_irq(gpio_to_irq(PAD_GPIO_B+9),NULL);
	free_irq(gpio_to_irq(PAD_GPIO_B+31),NULL);
	free_irq(gpio_to_irq(PAD_GPIO_B+30),NULL);
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
	
	//注销工作队列
	cancel_work_sync(&mywork);
	
	
	//注销混杂设备
	misc_deregister(&key_miscdev);
	
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