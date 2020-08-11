#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>		//字符设备
#include <linux/fs.h>		//file_operations
#include <linux/uaccess.h>	//copy_from_user、copy_to_user
#include <linux/device.h>	//class_create、device_create


//声明一个led字符设备的结构体
static struct cdev led_cdev;	

//声明一个led的设备号
static dev_t led_num;

//声明一个led的类指针
static struct class  *led_class;

//声明一个led的设备指针
static struct device  *led_device;


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


ssize_t led_read (struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	int rt;
	
	char kbuf[8]={1,2,3,4,5,6,7,8};
	
	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	
	if(len >sizeof kbuf)
		len = sizeof kbuf;	
	
	
	rt = copy_to_user(buf,kbuf,sizeof kbuf);
	
	//得到实际拷贝的字节数
	len = len - rt;	
	
	
	//返回实际读取的字节数
	return len;
}



ssize_t led_write (struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	int rt;
	char kbuf[16]={0};
	
	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	
	if(len >sizeof kbuf)
		len = sizeof kbuf;

	//拷贝用户空间的数据，类似于memcpy
	//返回没有被成功拷贝的字节数
	rt = copy_from_user(kbuf,buf,len);
	
	//得到实际拷贝的字节数
	//len=16
	//rt=2	
	len = len - rt;
	
	printk(KERN_INFO"kbuf:%s\n",kbuf);
	
	
	//返回实际写入数据的长度
	return len;
}

struct file_operations led_fops={
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
	.write = led_write,
	.read = led_read,
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
	
	//创建myled的设备类,可以在/sys/class目录当中找到
	led_class=class_create(THIS_MODULE,"myled");
	
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(led_class))
	{
		printk(KERN_INFO"class_create error\n");
		
		//将指针转换为错误码
		rt = PTR_ERR(led_class);
		
		goto err_class_create;
		
		
	}
	
	//创建myled的设备信息:设备号、设备文件名
	led_device=device_create(led_class, NULL, led_num, NULL, "myled");
		
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(led_device))
	{
		printk(KERN_INFO"device_create error\n");
		
		//将指针转换为错误码
		rt = PTR_ERR(led_device);
		
		goto err_device_create;
		
		
	}	
	printk(KERN_INFO"myled_init\n");

	//成功返回
	return 0;
	
err_device_create:
	class_destroy(led_class);	
err_class_create:
	cdev_del(&led_cdev);
err_cdev_add:	
	unregister_chrdev_region(led_num,1);
	
err_register_chrdev_region:

	return rt;
}


static void __exit myled_exit(void)
{
	//设备的销毁
	device_destroy(led_class,led_num);
	
	//类的销毁
	class_destroy(led_class);
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