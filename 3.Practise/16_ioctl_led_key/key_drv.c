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
#include <linux/miscdevice.h>//misc_registe
#include <linux/ioctl.h>

#define CMD_KEY_GET _IOR('K',0,unsigned int *)


static struct gpio keys_gpios[] = {
	{ PAD_GPIO_B+31, GPIOF_OUT_INIT_HIGH, "KEY K4" }, /* default to OFF */
	{ PAD_GPIO_B+30, GPIOF_OUT_INIT_HIGH, "KEY K3" }, /* default to OFF */
	{ PAD_GPIO_A+28, GPIOF_OUT_INIT_HIGH, "KEY K2" }, /* default to OFF */
	{ PAD_GPIO_B+9, GPIOF_OUT_INIT_HIGH, "KEY K6" }, /* default to OFF */
};

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
	void __user *argp = (void __user *) args;

	unsigned int key_val=0;

	switch(cmd)
	{
		case CMD_KEY_GET:
		{
			key_val|=(gpio_get_value(PAD_GPIO_A+28))?0:1;
			key_val|=(gpio_get_value(PAD_GPIO_B+30))?0:(1 <<1);
			key_val|=(gpio_get_value(PAD_GPIO_B+31))?0:(1 << 2);
			key_val|=(gpio_get_value(PAD_GPIO_B+9))?0:(1 << 3);

			copy_to_user(argp, &key_val, sizeof(key_val));
			
		}break;

		default:
			return -ENOIOCTLCMD;
	}
}


struct file_operations key_fops={
	.owner = THIS_MODULE,
	.open = key_open,
	.release = key_close,
	.unlocked_ioctl = key_ioctl,
};

static struct miscdevice key_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "mykey", //设备名称,/dev/myled
	.fops = &key_fops,//文件操作集
};

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
	
	//由于内核已经申请过引脚，必须先释放
	gpio_free_array(keys_gpios, ARRAY_SIZE(keys_gpios));
	
	//申请一组GPIO
	rt = gpio_request_array(keys_gpios, ARRAY_SIZE(keys_gpios));
	if (rt < 0)	
	{
		printk(KERN_INFO"gpio_request_array error\n");
		goto err_gpio_request_array;
	}
	
	printk(KERN_INFO"mykey_init\n");

	//成功返回
	return 0;
	

err_gpio_request_array:
	misc_deregister(&key_miscdev);

err_misc_register:	
	return rt;
}

static void __exit mykey_exit(void)
{
	//释放一组GPIO
	gpio_free_array(keys_gpios, ARRAY_SIZE(keys_gpios));
	
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
MODULE_DESCRIPTION("This is key driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");