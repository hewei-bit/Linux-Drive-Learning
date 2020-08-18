#include <linux/types.h>  
#include <linux/delay.h>
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
#include "oled.h" 
#include "bmp.h"

extern void OLED_Init(void);
extern void OLED_Clear(void);
extern void OLED_WR_Byte(unsigned dat,unsigned cmd);
extern void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);



static struct gpio oled_gpios[] ={
{ PAD_GPIO_B+28,GPIOF_OUT_INIT_LOW,"oled2"},
{ PAD_GPIO_B+29,GPIOF_OUT_INIT_LOW,"oled3"},
};




//设备打开接口

int oled_open (struct inode * inode, struct file *file)
{
	// OLED_Init();			//初始化OLED  
	// OLED_Clear(); 
	// OLED_DrawBMP(0,0,128,8,BMP1);
	printk(KERN_INFO"oled_open\n");

	
	return 0;
}

//设备关闭
int oled_close (struct inode * inode, struct file *file)
{
	
	
	printk(KERN_INFO"oled_close\n");
	
	
	return 0;
}

ssize_t oled_write (struct file *file,const char __user *buf, size_t len, loff_t *offset)
{
	OLED_Init();			//初始化OLED  
	OLED_Clear()  	; 
	
	//显示BMP图片
	//x=0,y=0
	//图片大小：宽128像素点，高8行
	OLED_DrawBMP(0,0,128,8,BMP1);


	mdelay(3000);

			
		//从左到右移动
		OLED_WR_Byte(0x2E,OLED_CMD);        //关闭滚动
		OLED_WR_Byte(0x26,OLED_CMD);        //水平向左或者右滚动 26/27
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x00,OLED_CMD);        //起始页 0
		OLED_WR_Byte(0x07,OLED_CMD);        //滚动时间间隔
		OLED_WR_Byte(0x07,OLED_CMD);        //终止页 7
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0xFF,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x2F,OLED_CMD);        //开启滚动
			
		mdelay(5000);
			
		//从右到左移动
		OLED_WR_Byte(0x2E,OLED_CMD);        //关闭滚动
		OLED_WR_Byte(0x27,OLED_CMD);        //水平向左或者右滚动 26/27
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x00,OLED_CMD);        //起始页 0
		OLED_WR_Byte(0x07,OLED_CMD);        //滚动时间间隔
		OLED_WR_Byte(0x07,OLED_CMD);        //终止页 7
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0xFF,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x2F,OLED_CMD);        //开启滚动	

		mdelay(5000);	

		//设置垂直和水平滚动的
		OLED_WR_Byte(0x2e,OLED_CMD);        //关闭滚动
		OLED_WR_Byte(0x29,OLED_CMD);        //水平垂直和水平滚动左右 29/2a
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x00,OLED_CMD);        //起始页 0
		OLED_WR_Byte(0x07,OLED_CMD);        //滚动时间间隔
		OLED_WR_Byte(0x07,OLED_CMD);        //终止页 1
		OLED_WR_Byte(0x01,OLED_CMD);        //垂直滚动偏移量
		OLED_WR_Byte(0x2F,OLED_CMD);        //开启滚动

		mdelay(5000);	

		//设置垂直和水平滚动的
		OLED_WR_Byte(0x2e,OLED_CMD);        //关闭滚动
		OLED_WR_Byte(0x2A,OLED_CMD);        //水平垂直和水平滚动左右 29/2a
		OLED_WR_Byte(0x00,OLED_CMD);        //虚拟字节
		OLED_WR_Byte(0x00,OLED_CMD);        //起始页 0
		OLED_WR_Byte(0x07,OLED_CMD);        //滚动时间间隔
		OLED_WR_Byte(0x07,OLED_CMD);        //终止页 1
		OLED_WR_Byte(0x01,OLED_CMD);        //垂直滚动偏移量
		OLED_WR_Byte(0x2F,OLED_CMD);        //开启滚动

		mdelay(5000);	
	
	

	return 0;
}

// read(struct file *file, char __user *userbuf, size_t bytes, loff_t *off)

ssize_t oled_read (struct file *file, char __user *buf, size_t len, loff_t * offs)
{
	int rt;
	char oled_val = 0;

	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	if(len >sizeof oled_val)
		len = sizeof oled_val;



	rt = copy_to_user(buf,&oled_val,sizeof oled_val);
	
	//得到实际拷贝的字节数
	len = len - rt;	
	
	
	//返回实际读取的字节数
	return len;
}


struct file_operations oled_fops={
	 .owner = THIS_MODULE,
	 .open = oled_open,
	 .release = oled_close,
	 .write = oled_write,
	 .read = oled_read,
	
};


static struct miscdevice oled_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "myoled_drv", //设备名称,/dev/myled
	.fops = &oled_fops,//文件操作集
};

static int __init myoled_init(void)
{
	int rt;

	//混杂设备的注册
	rt = misc_register(&oled_miscdev);
	
	if(rt < 0)
	{
		printk(KERN_INFO"misc_register error\n");
		
		goto err_misc_register;
		
	}

	//由于内核已经申请过引脚，必须先释放
	gpio_free_array(oled_gpios, ARRAY_SIZE(oled_gpios));
	
	//申请一组GPIO
	rt = gpio_request_array(oled_gpios, ARRAY_SIZE(oled_gpios));
	if (rt < 0)	
	{
		printk(KERN_INFO"gpio_request_array error\n");
		goto err_gpio_request_array;
	}
	

	OLED_Init();			//初始化OLED  
	OLED_Clear()  	; 
	printk(KERN_INFO"myoled_init\n");


	//成功返回
	return 0;
	

	
err_gpio_request_array:
	misc_deregister(&oled_miscdev);

err_misc_register:	
	return rt;
	
}	

static void __exit myoled_exit(void)
{
	//解除映射
	gpio_free_array(oled_gpios, ARRAY_SIZE(oled_gpios));
	


		
	//注销混杂设备
	misc_deregister(&oled_miscdev);

	
	printk(KERN_INFO"myoled_exit\n");
}



//加载驱动：当使用insmod oled_drv.ko的时候，会调用module_init函数，module_init函数会回调myoled_init函数
module_init(myoled_init);

//卸载驱动：当使用rmmod oled_drv的时候，会调用module_exit函数，module_exit函数会回调myoled_exit函数
module_exit(myoled_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("511");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is oled driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");