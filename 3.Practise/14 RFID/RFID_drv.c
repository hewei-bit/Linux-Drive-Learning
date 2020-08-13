#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include "MFRC522.h"

//MFRC522数据区
unsigned char  mfrc552pidbuf[18];
unsigned char  card_pydebuf[2];
unsigned char  card_numberbuf[5];
unsigned char  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
unsigned char  card_readbuf[18];

static struct gpio rc522_gpios[5] = {
		// 引脚编号     引脚输出模式-初始电平状态为高电平	自定义引脚的名字
		{ rc522_RST, 	GPIOF_OUT_INIT_HIGH, 		"rc522_RST" }, 
		{ rc522_MOSI, 	GPIOF_OUT_INIT_HIGH, 		"rc522_MOSI" }, 
		{ rc522_SCK,  	GPIOF_OUT_INIT_HIGH, 		"rc522_SCK"  }, 
		{ rc522_SDA,  	GPIOF_OUT_INIT_HIGH, 		"rc522_SDA" }, 
		{ rc522_MISO,  	GPIOF_IN, 					"rc522_MISO" }, 
};

static int rc522_open (struct inode *inode, struct file *file)
{
	printk(KERN_INFO"rc522_open\n");
	
	return 0;
}


static int rc522_close (struct inode *inode, struct file *file)
{
	printk(KERN_INFO"rc522_close\n");	
	
	return 0;
}


static ssize_t rc522_write (struct file * file, const char __user *buf, size_t len, loff_t * offset)
{
	char kbuf[16]={0};
	
	int rt=0;
	
	unsigned char status,card_size;
	
	if(buf == NULL)
		return -EINVAL;		
	
	if(len > sizeof (kbuf))
		return -EINVAL;		
	
	rt = copy_from_user(kbuf,buf,len);
	
	MFRC522_Initializtion();	
	
	status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
	
	if(status==0)										//如果读到卡
	{
		status=MFRC522_Anticoll(card_numberbuf);		//防撞处理			
		card_size=MFRC522_SelectTag(card_numberbuf);	//选卡
		status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
		status=MFRC522_Write(4, kbuf);					//写卡
		
		return 0;
	}
	
	return -EAGAIN;
}

static ssize_t rc522_read (struct file *file, char __user *buf, size_t len, loff_t *off)
{
	char kbuf[21] = {0};
	int rt = 0;
	unsigned char status,card_size;
	
	if(buf == NULL)
		return -EINVAL;
	
	//如果len比kbuf大
	len = (len > sizeof kbuf)? sizeof kbuf : len;
	
	MFRC522_Initializtion();
	
	status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
	
	if(status==0)										//如果读到卡
	{
		status=MFRC522_Anticoll(kbuf);					//防撞处理			
		card_size=MFRC522_SelectTag(kbuf);				//选卡
		status=MFRC522_Auth(0x60, 4, card_key0Abuf, kbuf);	//验卡
		
		status=MFRC522_Read(4, (kbuf+5));				//读卡
		rt = copy_to_user(buf, &kbuf, len);
		
		return 0;
	}
	
	return -EAGAIN;
}

static struct file_operations rc522_fops={
	
	.owner 	 = THIS_MODULE,
	.open  	 = rc522_open,
	.write 	 = rc522_write,
	.read 	 = rc522_read,
	.release = rc522_close,
};


static struct miscdevice rc522_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, //可以MISC_DYNAMIC_MINOR,动态分配次设备号
	.name = "myrc522", 			//设备名称,/dev/adc
	.fops = &rc522_fops,		//文件操作集
};

//入口函数
static int __init rc522_init(void)
{
	int rt=0;

	//混杂设备的注册
	rt = misc_register(&rc522_miscdev);
	
	if (rt < 0)
	{
		goto err_misc_register;
	}
	
	//先释放内核占用的引脚资源
	gpio_free_array(rc522_gpios, ARRAY_SIZE(rc522_gpios));
	
	//申请多个引脚资源
	rt = gpio_request_array(rc522_gpios, ARRAY_SIZE(rc522_gpios));
	
	if (rt < 0)
	{
		goto err_gpio_request_array;
	}
	
	printk("rc522 init\n");

	return 0;
	
err_gpio_request_array:
	misc_deregister(&rc522_miscdev);

err_misc_register:
	return rt;
}

//出口函数
static void __exit rc522_exit(void)
{
	//释放引脚资源
	gpio_free_array(rc522_gpios, ARRAY_SIZE(rc522_gpios));
	
	//混杂设备的注销
	misc_deregister(&rc522_miscdev);	
	
	printk("rm rc522\n");
}

//驱动程序的入口：安装驱动的时候，自动调用module_init，module_init继续调用rc522_init函数
module_init(rc522_init);

//驱动程序的出口：卸载驱动的时候，自动调用module_exit，module_exit继续调用rc522_exit函数
module_exit(rc522_exit);


MODULE_AUTHOR("Lin");//作者描述
MODULE_DESCRIPTION("rc522 Device Driver");//模块描述
MODULE_LICENSE("GPL");			//遵循GPL