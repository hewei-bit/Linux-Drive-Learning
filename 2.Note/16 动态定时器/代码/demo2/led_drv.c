#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/delay.h>
#include <linux/timer.h>	//内核动态定时器


//动态定时器
static struct timer_list gec6818_timer;


void gec6818_timer_fun(unsigned long data)
{
	
	printk("gec6818_timer_fun\n");
	
	
	//添加修改超时时间，1000ms的超时
	mod_timer(&gec6818_timer,jiffies+msecs_to_jiffies(1000));
	
	
}

static int __init myled_init(void)
{

	printk("<6>""HZ=%d\n",HZ);
	//得到开机的时间
	printk("jiffies/HZ = %lu\n",(jiffies - INITIAL_JIFFIES)/HZ);
	
	gec6818_timer.function = gec6818_timer_fun;
	gec6818_timer.expires = jiffies + HZ;	//1秒的超时时间
	gec6818_timer.data =100;
	
	//初始化动态定时器
	init_timer(&gec6818_timer);
	
	
	//将动态定时器加入到内核
	add_timer(&gec6818_timer);
	
	//成功返回
	return 0;
}


static void __exit myled_exit(void)
{
	//从内核删除动态定时器
	del_timer(&gec6818_timer);
	printk(KERN_WARNING"myled_exit\n");
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