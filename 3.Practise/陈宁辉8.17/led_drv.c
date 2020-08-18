#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/delay.h>
#include <linux/timer.h>	//内核动态定时器
#include <linux/io.h>		//ioremap
#include <linux/gpio.h>
#include <cfg_type.h>

//动态定时器
static struct timer_list gec6818_timer;

static struct gpio leds_gpios[] = {
	{ PAD_GPIO_E+13, GPIOF_OUT_INIT_HIGH, "LED D7" }, /* default to OFF */
	{ PAD_GPIO_C+17, GPIOF_OUT_INIT_HIGH, "LED D8" }, /* default to OFF */
	{ PAD_GPIO_C+8, GPIOF_OUT_INIT_HIGH, "LED D9" }, /* default to OFF */
	{ PAD_GPIO_C+7, GPIOF_OUT_INIT_HIGH, "LED D10" }, /* default to OFF */
};

static int flag = 1;//控制增或减方向
static int count = 50;

void gec6818_timer_fun(unsigned long data)
{
	static unsigned int b=0;
	
	printk("gec6818_timer_fun\n");
	
	b=!b;
	
	gpio_set_value(PAD_GPIO_E+13,b);
	gpio_set_value(PAD_GPIO_C+17,b);
	gpio_set_value(PAD_GPIO_C+8,b);
	gpio_set_value(PAD_GPIO_C+7,b);

	if(count > 500)flag = 0;
	if(count < 50) flag = 1;
	
	if(flag)
	{
		//添加修改超时时间，1000ms的超时
		mod_timer(&gec6818_timer,jiffies+msecs_to_jiffies(count));
		count += 30;
	}
	else
	{
		//添加修改超时时间，1000ms的超时
		mod_timer(&gec6818_timer,jiffies+msecs_to_jiffies(count));
		count -= 30;
	}
		
}

static int __init myled_init(void)
{
	int rt;

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

	//由于内核已经申请过引脚，必须先释放
	gpio_free_array(leds_gpios, ARRAY_SIZE(leds_gpios));
	
	//申请一组GPIO
	rt = gpio_request_array(leds_gpios, ARRAY_SIZE(leds_gpios));
	if (rt < 0)	
	{
		printk(KERN_INFO"gpio_request_array error\n");
		goto err_gpio_request_array;
	}
	
	//成功返回
	return 0;
err_gpio_request_array:
	//从内核删除动态定时器
	del_timer(&gec6818_timer);

	return -1;
}


static void __exit myled_exit(void)
{
	//释放一组GPIO
	gpio_free_array(leds_gpios, ARRAY_SIZE(leds_gpios));

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