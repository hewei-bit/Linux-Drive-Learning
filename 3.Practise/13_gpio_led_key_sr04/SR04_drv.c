#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>		//字符设备
#include <linux/fs.h>		//file_operations
#include <linux/uaccess.h>	//copy_from_user、copy_to_user
#include <linux/device.h>	//class_create、device_create
#include <linux/ioport.h>	//request_mem_region 
#include <linux/io.h>		//ioremap
#include <linux/delay.h>	//msleep,mdelay,udelay.....
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/miscdevice.h>//misc_register

//声明一个gpiod起始虚拟地址的指针
//GPIOD19   trig
//GPIO15    echo

static struct gpio SR04_gpios[] = {
	{ PAD_GPIO_D+19, GPIOF_OUT_INIT_HIGH, "TRIG" }, /* default to OFF */
	{ PAD_GPIO_D+15, GPIOF_OUT_INIT_HIGH, "ECHO" }, /* default to OFF */
};


int SR04_open (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"SR04_open\n");
	return 0;
}

int SR04_close (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"SR04_close\n");
	return 0;
}

int SR04_get_distance(void)
{
	unsigned int t=0;
	int d=0;
	//触发超声波模块
	gpio_set_value(PAD_GPIO_D+19,1);
	udelay(10);
	gpio_set_value(PAD_GPIO_D+19,0);

	//等待信号返回
	//添加超时处理
	t=0;
	while(gpio_get_value(PAD_GPIO_D+15) == 0 )
	{
		t++;
		udelay(1);
		
		if(t >= 1000000)
		{
			return -1;
		}
	}

	//有信号返回
	//添加超时处理
	t = 0;
	while(gpio_get_value(PAD_GPIO_D+15) )
	{
		t++;
		udelay(9);     //超声波传输9us，等同于传输了3mm
		//若高电平时间持续过长
		if(t >= 1000000)
		{
			return -2;
		}
	}

	//将时间换算距离
	//3mm 传输时间约为 9us
	t = t/2;
	d = 3*t;
	return d;
}

ssize_t SR04_read (struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	int rt;
	int distance;
	char sr04_val;

	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	distance = SR04_get_distance();
	
	if(len >sizeof (sr04_val))
		len = sizeof (sr04_val);	
		
	rt = copy_to_user(buf,&distance,sizeof distance);
	
	//得到实际拷贝的字节数
	len = len - rt;	
		
	//返回实际读取的字节数
	return len;
}


struct file_operations SR04_fops={
	.owner = THIS_MODULE,
	.open = SR04_open,
	.release = SR04_close,
	.read = SR04_read,
};

static struct miscdevice SR04_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "mySR04", //设备名称,/dev/myled
	.fops = &SR04_fops,//文件操作集
};

static int __init mySR04_init(void)
{
	int rt;

	//混杂设备的注册
	rt = misc_register(&SR04_miscdev);
	
	if(rt < 0)
	{
		printk(KERN_INFO"misc_register error\n");
		
		goto err_misc_register;
		
	}
	
	//由于内核已经申请过引脚，必须先释放
	gpio_free_array(SR04_gpios, ARRAY_SIZE(SR04_gpios));
	
	//申请一组GPIO
	rt = gpio_request_array(SR04_gpios, ARRAY_SIZE(SR04_gpios));
	if (rt < 0)	
	{
		printk(KERN_INFO"gpio_request_array error\n");
		goto err_gpio_request_array;
	}
	
	printk(KERN_INFO"mySR04_init\n");

	//成功返回
	return 0;

	
err_gpio_request_array:
	misc_deregister(&SR04_miscdev);

err_misc_register:	
	return rt;
	
	
}

static void __exit mySR04_exit(void)
{
	//释放一组GPIO
	gpio_free_array(SR04_gpios, ARRAY_SIZE(SR04_gpios));
	
	//注销混杂设备
	misc_deregister(&SR04_miscdev);
	
	printk(KERN_INFO"mySR04_exit\n");
}

//加载驱动：当使用insmod SR04_drv.ko的时候，会调用module_init函数，module_init函数会回调mySR04_init函数
module_init(mySR04_init);

//卸载驱动：当使用rmmod SR04_drv的时候，会调用module_exit函数，module_exit函数会回调mySR04_exit函数
module_exit(mySR04_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Master.Wei");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is SR04 driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");