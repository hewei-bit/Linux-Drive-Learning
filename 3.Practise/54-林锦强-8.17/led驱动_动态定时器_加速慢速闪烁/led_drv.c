#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/jiffies.h>
#include <linux/timer.h>


#define CMD_LED7 _IOW('l', 0, unsigned int)

//动态定时器
static struct timer_list led_twinkle_timer;

//定时时间记录
static int time_change = HZ;
//加速减速标志位 0为减速 1为加速
static int time_flag = 1;
//亮灭灯标志位 0为灭 1为亮
static int led_flag = 0;

//定义一个led的gpio配置结构体数组
struct gpio gpio_led[4] = {
	{ PAD_GPIO_E+13, GPIOF_OUT_INIT_HIGH, "LED D7" }, /* default to OFF */
	{ PAD_GPIO_C+17, GPIOF_OUT_INIT_HIGH, "LED D8" }, /* default to OFF */
	{ PAD_GPIO_C+8, GPIOF_OUT_INIT_HIGH, "LED D9" }, /* default to OFF */
	{ PAD_GPIO_C+7, GPIOF_OUT_INIT_HIGH, "LED D10" }, /* default to OFF */
};


static int led_open(struct inode * inode, struct file *file)
{
	//将动态定时器加入到内核
	add_timer(&led_twinkle_timer);
	
	printk("<6>""led_open\n");

	return 0;
}


static int led_close(struct inode * inode, struct file * file)
{
	//从内核中删除动态定时器
	del_timer(&led_twinkle_timer);
	
	//LED7灭
	gpio_set_value(PAD_GPIO_E+13, 1);
	//LED8灭
	gpio_set_value(PAD_GPIO_C+17, 1);
	//LED9灭
	gpio_set_value(PAD_GPIO_C+8, 1);
	//LED10灭
	gpio_set_value(PAD_GPIO_C+7, 1);
	
	printk("<6>""led_close\n");
	
	return 0;
}



//文件操作符结构体
struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
};


//定义一个混杂设备结构体
static struct miscdevice led_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //指定了ADC的次设备号为131，也可以MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "myled", //设备名称,/dev/adc
	.fops = &led_fops,//文件操作集
};

//计时器结束后处理函数
void led_twinkle_handler(unsigned long data)
{
	//速度低于0.2s，开始减速
	if(time_change < 200){
		time_flag = 0;
	}
	//速度高于1s，开始加速
	else if(time_change > 800){
		time_flag = 1;
	}
	
	//加速
	if(time_flag == 1)
	{
		//每次变化0.1s
		time_change -= 50;
	}
	//减速
	if(time_flag == 0)
	{
		time_change += 50;
	}
	
	printk("<6>""time_change = %d\n", time_change);
	
	//亮灭灯
	gpio_set_value(PAD_GPIO_E+13, led_flag);
	led_flag = led_flag?0:1;
	
	//添加修改超时时间，1000ms的超时
	mod_timer(&led_twinkle_timer, jiffies+msecs_to_jiffies(time_change));
	
}


static int __init myled_init(void)
{
	int rt;
	
	//混杂设备的注册
	rt = misc_register(&led_miscdev);
	if(rt != 0){
		
		printk("<6>""misc_register error\n");
		
		goto err_misc_register;
	}

	
	//释放内核占用的引脚
	gpio_free_array(gpio_led, 4);
	
	//申请GPIOE13 C17 C8 C7引脚
	rt = gpio_request_array(gpio_led, 4);
	
	if(rt < 0)
	{
		printk(KERN_INFO"gpio_request error\n");
		
		goto err_gpio_request;
	}

	time_change = HZ;
	
	led_twinkle_timer.function = led_twinkle_handler;
	//先慢速
	led_twinkle_timer.expires = time_change+jiffies;
	led_twinkle_timer.data = 0;
	
	//初始化动态定时器
	init_timer(&led_twinkle_timer);
	
	
	
	printk("<6>""Lam is inited\n");


	return 0;

	
err_gpio_request:
	//混杂设备的注销
	misc_deregister(&led_miscdev);

err_misc_register:
	return rt;
}


static void __exit myled_exit(void)
{
	
	//释放内核占用的引脚
	gpio_free_array(gpio_led, 4);
	
	//混杂设备的注销
	misc_deregister(&led_miscdev);
	
	
	printk(KERN_INFO"Lam is exited\n");

}

//加载驱动：当使用insmod led_drv.ko的时候，会调用module_init函数，module_init函数会回调myled_init函数
module_init(myled_init);

//卸载驱动：当使用rmmod led_drv的时候，会调用module_exit函数，module_exit函数会回调myled_exit函数
module_exit(myled_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Lam");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is the led driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");