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

#define CMD_LED_D7 _IOW('L',0,unsigned int)
#define CMD_LED_D8 _IOW('L',1,unsigned int)
#define CMD_LED_D9 _IOW('L',2,unsigned int)
#define CMD_LED_D10 _IOW('L',3,unsigned int)

//动态定时器
static struct timer_list gec6818_timer;
static int flag = 1;
static int count = 50;

static struct gpio leds_gpios[] = {
	{ PAD_GPIO_E+13, GPIOF_OUT_INIT_HIGH, "LED D7" }, /* default to OFF */
	{ PAD_GPIO_C+17, GPIOF_OUT_INIT_HIGH, "LED D8" }, /* default to OFF */
	{ PAD_GPIO_C+8, GPIOF_OUT_INIT_HIGH, "LED D9" }, /* default to OFF */
	{ PAD_GPIO_C+7, GPIOF_OUT_INIT_HIGH, "LED D10" }, /* default to OFF */
};

void gec6818_timer_fun(unsigned long data)
{
	static unsigned int b = 0;
	printk("timer fun run \n");
	b = !b;
	gpio_set_value(PAD_GPIO_E+13,b);
	gpio_set_value(PAD_GPIO_C+17,b);
	gpio_set_value(PAD_GPIO_C+8,b);
	gpio_set_value(PAD_GPIO_C+7,b);

	if(count > 500)flag = 0;
	if(count < 50)flag =1;

	if(flag)
	{
		mod_timer(&gec6818_timer,jiffies+msecs_to_jiffies(count));
		count += 30;
	}
	else
	{
		mod_timer(&gec6818_timer,jiffies+msecs_to_jiffies(count));
		count -= 30;
	}
	

}

static int __init myled_init(void)
{
	int rt;

	printk("<6>""HZ=%d\n",HZ);

	printk("jiffies/HZ = %lu\n",(jiffies - INITIAL_JIFFIES)/HZ);

	gec6818_timer.function = gec6818_timer_fun;
	gec6818_timer.expires = jiffies + HZ;
	gec6818_timer.data = 100;

	//初始化定时器
	init_timer(&gec6818_timer);

	//将动态定时器加入到内核
	add_timer(&gec6818_timer);

	
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
	del_timer(&gec6818_timer);


	return rt;
	
	
	
}


static void __exit myled_exit(void)
{
	//释放一组GPIO
	gpio_free_array(leds_gpios, ARRAY_SIZE(leds_gpios));
	
	//注销混杂设备
	del_timer(&gec6818_timer);
	
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