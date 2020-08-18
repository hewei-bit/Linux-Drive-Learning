/*
	混杂设备、中断、唤醒、ioctl
*/


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

#define CMD_KEY_GET _IOR('K',0,unsigned int *)
              // 驱动层向应用层传递 // 


//声明一个等待条件，真和假
static int key_press_flag = 0;

//声明一个等待队列
static wait_queue_head_t key_wq;


static unsigned char key_val=0;


static struct gpio keys_gpios[] ={
{ PAD_GPIO_A+28,GPIOF_DIR_IN,"KEY2"},
{ PAD_GPIO_B+30,GPIOF_DIR_IN,"KEY3"},
{ PAD_GPIO_B+31,GPIOF_DIR_IN,"KEY4"},
{ PAD_GPIO_B+9,GPIOF_DIR_IN,"KEY6"},
};

//中断服务函数
//irq，就是当前触发中断请求的中断号
//dev，就是request_irq传递的参数
irqreturn_t keys_irq_handler(int irq, void *dev)
{
	
	key_val = 0;
	key_val |=(gpio_get_value(PAD_GPIO_A+28))?0:1;
	key_val |=(gpio_get_value(PAD_GPIO_B+30))?0:(1<<1);
	key_val |=(gpio_get_value(PAD_GPIO_B+31))?0:(1<<2);
	key_val |=(gpio_get_value(PAD_GPIO_B+9))?0:(1<<3);

	//设置条件为真
	key_press_flag=1;
	
	//唤醒队列
	wake_up(&key_wq);

	return IRQ_HANDLED;//当前中断处理已经完成
}



//设备打开接口

int key_open (struct inode * inode, struct file *file)
{
	
	printk(KERN_INFO"key_open\n");
	
	
	return 0;
}

//设备关闭
int key_close (struct inode * inode, struct file *file)
{
	
	
	printk(KERN_INFO"key_close\n");
	
	
	return 0;
}

ssize_t key_write (struct file *file,const char __user *buf, size_t len, loff_t *offset)
{
	return 0;
}

// read(struct file *file, char __user *userbuf, size_t bytes, loff_t *off)

ssize_t key_read (struct file *file, char __user *buf, size_t len, loff_t * offs)
{
	int rt;
	// char key_val = 0;
	// unsigned int pada = 0;
	// unsigned int padb = 0;
	
	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	
	if(len >sizeof key_val)
		len = sizeof key_val;

	key_val |=(gpio_get_value(PAD_GPIO_A+28))?0:1;
	key_val |=(gpio_get_value(PAD_GPIO_B+30))?0:(1<<1);
	key_val |=(gpio_get_value(PAD_GPIO_B+31))?0:(1<<2);
	key_val |=(gpio_get_value(PAD_GPIO_B+9))?0:(1<<3);
	
	rt = copy_to_user(buf,&key_val,sizeof key_val);
	
	//得到实际拷贝的字节数
	len = len - rt;	
	
	
	//返回实际读取的字节数
	return len;
}

 ////////ioctl//////////
// long key_ioctl (struct file *filp, unsigned int cmd, unsigned long args)
// long key_ioctl(struct file  *filp,  unsigned int cmd,unsigned long args)
long key_ioctl (struct file *filp, unsigned int cmd, unsigned long args)

{
	void __user *arpg = (void __user *)args;

	// unsigned int key_val = 0;
	int rt;
	switch(cmd)
	{
		case CMD_KEY_GET:
		{

			//等待按键的事件，可中断睡眠
			wait_event_interruptible(key_wq,key_press_flag);
			key_press_flag=0;
			rt = copy_to_user(arpg,&key_val,sizeof key_val);
			
			if(rt != 0)
				return -ENOMEM;


		}break;
		default:
			return -ENOIOCTLCMD;
	}
	return 0;
	
}
//////////ioctl//////////

struct file_operations key_fops={
		.owner        = THIS_MODULE,
		.open         = key_open,
		.release      = key_close,
		.write        = key_write,
		.read         = key_read,
		.unlocked_ioctl = key_ioctl,
	
};

static struct miscdevice keys_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "mykey", //设备名称,/dev/myled
	.fops = &key_fops,//文件操作集
};

static int __init mykey_init(void)
{
	int rt;

	//混杂设备的注册
	rt = misc_register(&keys_miscdev);
	
	if(rt < 0)
	{
		printk(KERN_INFO"misc_register error\n");
		
		goto err_misc_register;
		
	}

	//由于内核已经申请过引脚，必须先释放
	gpio_free_array(keys_gpios, ARRAY_SIZE(keys_gpios));
	
	//申请一组GPIO
	rt = gpio_request_array(keys_gpios, ARRAY_SIZE(keys_gpios));
	if (rt < 0)	
	{
		printk(KERN_INFO"gpio_request_array error\n");
		goto err_gpio_request_array;
	}
	
	//中断号：gpio_to_irq(PAD_GPIO_A+28)
	//中断服务函数：keys_irq_handler
	//触发方式：下降沿触发，IRQF_TRIGGER_FALLING
	//注册名字："gpioa28"
	//传递参数：NULL
	rt = request_irq(gpio_to_irq(PAD_GPIO_A+28),keys_irq_handler,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING | IRQF_TRIGGER_FALLING,"gpioa28",NULL);
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request_irq;
		
	}

	rt = request_irq(gpio_to_irq(PAD_GPIO_B+30),keys_irq_handler,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,"gpiob30",NULL);
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request_irq;
		
	}

	rt = request_irq(gpio_to_irq(PAD_GPIO_B+31),keys_irq_handler,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,"gpiob31",NULL);
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request_irq;
		
	}

	rt = request_irq(gpio_to_irq(PAD_GPIO_B+9),keys_irq_handler,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,"gpiob9",NULL);	
	if(rt < 0)
	{
		printk("request_irq error\n");
		
		goto err_request_irq;
		
	}

	//初始化等待队列
	init_waitqueue_head(&key_wq);
	printk(KERN_INFO"mykey_init\n");



	//成功返回
	return 0;
	



err_request_irq:
	return rt;
	
err_gpio_request_array:
	misc_deregister(&keys_miscdev);

err_misc_register:	
	return rt;
	
}	

static void __exit mykey_exit(void)
{
	//释放中断
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
	free_irq(gpio_to_irq(PAD_GPIO_B+30),NULL);
	free_irq(gpio_to_irq(PAD_GPIO_B+31),NULL);
	free_irq(gpio_to_irq(PAD_GPIO_B+9),NULL);
	//解除映射
	gpio_free_array(keys_gpios, ARRAY_SIZE(keys_gpios));



		
	//注销混杂设备
	misc_deregister(&keys_miscdev);

	
	printk(KERN_INFO"mykey_exit\n");
}



//加载驱动：当使用insmod key_drv.ko的时候，会调用module_init函数，module_init函数会回调mykey_init函数
module_init(mykey_init);

//卸载驱动：当使用rmmod key_drv的时候，会调用module_exit函数，module_exit函数会回调mykey_exit函数
module_exit(mykey_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("511");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is key driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");