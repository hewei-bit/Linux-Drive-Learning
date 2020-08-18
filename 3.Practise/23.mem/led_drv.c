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

#include <linux/slab.h>


char *p = NULL;

char *q = NULL;

static int __init myled_init(void)
{
	//小空间申请，物理地址连续
	p = kmalloc(1024,GFP_KERNEL);

	//申请大空间
	q = vmalloc(100*1024*1024);


	strcpy(p,"This is kmalloc test");	
	strcpy(q."vmalloc ");

	printk(KERN_INFO"myled_init\n");
	printk(KERN_INFO"p = %s \n",p);
	//成功返回
	return 0;

	
err_gpio_request_array:
	del_timer(&gec6818_timer);


	return rt;
	
	
	
}


static void __exit myled_exit(void)
{
	kfree(p);
	kfree(q);
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