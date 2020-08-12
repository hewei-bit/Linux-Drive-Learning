#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>		//字符设备
#include <linux/fs.h>		//file_operations
#include <linux/uaccess.h>	//copy_from_user、copy_to_user
#include <linux/device.h>	//class_create、device_create
#include <linux/ioport.h>	//request_mem_region 
#include <linux/io.h>		//ioremap
#include <linux/gpio.h>		//gpio_request、gpio_set_value.............
#include <cfg_type.h>		//引脚编号:PAD_GPIO_E、.....
#include <linux/miscdevice.h>

static struct gpio leds_gpios[] = {
	{ PAD_GPIO_E+13, GPIOF_OUT_INIT_HIGH, "LED D7" }, /* default to OFF */
	{ PAD_GPIO_C+17, GPIOF_OUT_INIT_HIGH, "LED D8" }, /* default to OFF */
	{ PAD_GPIO_C+8, GPIOF_OUT_INIT_HIGH, "LED D9" }, /* default to OFF */
	{ PAD_GPIO_C+7, GPIOF_OUT_INIT_HIGH, "LED D10" }, /* default to OFF */
};

int led_open (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"led_open\n");

	return 0;
}

int led_close (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"led_close\n");

	return 0;
}


ssize_t led_write (struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	int rt;
	unsigned long value;
	char kbuf[2]={0};
	
	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	if(len >sizeof kbuf)
		len = sizeof kbuf;

	//拷贝用户空间的数据，类似于memcpy
	//返回没有被成功拷贝的字节数
	rt = copy_from_user(kbuf,buf,len);

	if(rt != 0)
		return -EFAULT;
	
	switch(kbuf[0])
	{
		case 7://D7灯的亮与灭
		{
			gpio_set_value(PAD_GPIO_E+13,!kbuf[1]);
		}
		break;
		case 8://D8灯的亮与灭
		{
			gpio_set_value(PAD_GPIO_C+17,!kbuf[1]);	
		}
		break;
		case 9://D9灯的亮与灭
		{
			gpio_set_value(PAD_GPIO_C+8,!kbuf[1]);
		}
		break;
		case 10://D10灯的亮与灭
		{
			gpio_set_value(PAD_GPIO_C+7,!kbuf[1]);
		}
		break;

		default: return -EINVAL;	break;
	}
	
	//返回实际写入数据的长度
	return len;
}

struct file_operations led_fops={
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
	.write = led_write,
};

static struct miscdevice led_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "myled", //设备名称,/dev/myled
	.fops = &led_fops,//文件操作集
};

static int __init myled_init(void)
{
	int rt;

	//混杂设备的注册
	rt = misc_register(&led_miscdev);
	
	if(rt < 0)
	{
		printk(KERN_INFO"misc_register error\n");
		
		goto err_misc_register;
		
	}
	
	//由于内核已经申请过引脚，必须先释放
	gpio_free_array(leds_gpios, ARRAY_SIZE(leds_gpios));
	
	//申请一组GPIO
	rt = gpio_request_array(leds_gpios, ARRAY_SIZE(leds_gpios));
	if (rt < 0)	
	{
		printk(KERN_INFO"gpio_request_array error\n");
		goto err_gpio_request_array;
	}
	
	printk(KERN_INFO"myled_init\n");

	//成功返回
	return 0;

err_gpio_request_array:
	misc_deregister(&led_miscdev);

err_misc_register:	
	return rt;
}

static void __exit myled_exit(void)
{
	//释放一组GPIO
	gpio_free_array(leds_gpios, ARRAY_SIZE(leds_gpios));
	
	//注销混杂设备
	misc_deregister(&led_miscdev);
	
	printk(KERN_INFO"myled_exit\n");
}

//加载驱动：当使用insmod led_drv.ko的时候，会调用module_init函数，module_init函数会回调myled_init函数
module_init(myled_init);

//卸载驱动：当使用rmmod led_drv的时候，会调用module_exit函数，module_exit函数会回调myled_exit函数
module_exit(myled_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Teacher.Wen");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is led driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");