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

static struct gpio beep_gpios[] = {
	{ PAD_GPIO_C+14, GPIOF_OUT_INIT_LOW, "BEEP" }, /* default to OFF */
};

int beep_open (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"beep_open\n");
	return 0;
}

int beep_close (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"beep_close\n");
	return 0;
}

ssize_t beep_write (struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	int rt;
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
		case 1:
		{
			if(kbuf[1] == 1)
				//  iowrite32(ioread32(gpioc_out_va)|((1<<14)),gpioc_out_va);	
				gpio_set_value(PAD_GPIO_C+14,1);
			else if(kbuf[1] == 0)
				//  iowrite32(ioread32(gpioc_out_va)& (~((1<<14))),gpioc_out_va);	
					gpio_set_value(PAD_GPIO_C+14,0);
			else
				return -EINVAL;
		}
	 }
	len=len-rt;
	//返回实际写入数据的长度
	return len;
}

struct file_operations beep_fops={
	.owner = THIS_MODULE,
	.open = beep_open,
	.release = beep_close,
	.write  = beep_write,
};

static struct miscdevice beep_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "mybeep", //设备名称,/dev/myled
	.fops = &beep_fops,//文件操作集
};



static int __init mybeep_init(void)
{
	int rt;

	//混杂设备的注册
	rt = misc_register(&beep_miscdev);
	
	if(rt < 0)
	{
		printk(KERN_INFO"misc_register error\n");
		
		goto err_misc_register;
		
	}
	
	//由于内核已经申请过引脚，必须先释放
	gpio_free_array(beep_gpios, ARRAY_SIZE(beep_gpios));
	
	//申请一组GPIO
	rt = gpio_request_array(beep_gpios, ARRAY_SIZE(beep_gpios));
	if (rt < 0)	
	{
		printk(KERN_INFO"gpio_request_array error\n");
		goto err_gpio_request_array;
	}
	
	printk(KERN_INFO"mybeep_init\n");

	//成功返回
	return 0;

	
err_gpio_request_array:
	misc_deregister(&beep_miscdev);

err_misc_register:	
	return rt;
	
}

static void __exit mybeep_exit(void)
{
	//释放一组GPIO
	gpio_free_array(beep_gpios, ARRAY_SIZE(beep_gpios));
	
	//注销混杂设备
	misc_deregister(&beep_miscdev);
	
	
	printk(KERN_INFO"mybeep_exit\n");
}

//加载驱动：当使用insmod beep_drv.ko的时候，会调用module_init函数，module_init函数会回调mybeep_init函数
module_init(mybeep_init);

//卸载驱动：当使用rmmod beep_drv的时候，会调用module_exit函数，module_exit函数会回调mybeep_exit函数
module_exit(mybeep_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Teacher.Wen");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is beep driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");