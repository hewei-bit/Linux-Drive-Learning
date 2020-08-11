#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...

static int __init sum_init(void)
{
	
	printk("<6>""sum_init\n");

	//成功返回
	return 0;
}


static void __exit sum_exit(void)
{
	printk(KERN_WARNING"sum_exit\n");
}

//加载驱动：当使用insmod sum_drv.ko的时候，会调用module_init函数，module_init函数会回调sum_init函数
module_init(sum_init);

//卸载驱动：当使用rmmod sum_drv的时候，会调用module_exit函数，module_exit函数会回调sum_exit函数
module_exit(sum_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Teacher.Wen");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is sum driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");