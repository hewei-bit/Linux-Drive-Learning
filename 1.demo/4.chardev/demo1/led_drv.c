#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>
#include <linux/fs.h>

//声明一个led字符设备的结构体
static struct cdev led_cdev;	

//声明一个led的设备号
static dev_t led_num;


int led_open (struct inode * inode, struct file *file)
{
	
	printk(KERN_INFO"led_open\n");
	
	
	return 0;
}



int led_close (struct inode * inode, struct file *file)
{
	
	printk(KERN_INFO"led_close\n");
	
	
	return 0;
}



struct file_operations led_fops={
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
};

static int __init myled_init(void)
{
	int rt;

	//构建设备号,主设备号为240，次设备号为0
	led_num=MKDEV(240,0);
	
	
	//申请设备号，申请一个字符设备，若成功，会在/proc/devices文件当中显示myled的名字
	rt = register_chrdev_region(led_num,1,"myled");
	
	if(rt < 0)
	{
		
		printk(KERN_INFO"register_chrdev_region error\n");
		
		goto err_register_chrdev_region;
		
	}
	
	//字符设备的初始化
	cdev_init(&led_cdev,&led_fops);
	
	//将字符设备加入到内核
	rt = cdev_add(&led_cdev,led_num,1);
	
	if(rt < 0)
	{
		
		printk(KERN_INFO"cdev_add error\n");
		
		goto err_cdev_add;
		
	}
		
	
	printk(KERN_INFO"myled_init\n");

	//成功返回
	return 0;
	
err_cdev_add:	
	unregister_chrdev_region(led_num,1);
	
err_register_chrdev_region:

	return rt;
	
	
	
}


static void __exit myled_exit(void)
{
	//删除字符设备
	cdev_del(&led_cdev);
	
	//注销设备号
	unregister_chrdev_region(led_num,1);
	
	
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