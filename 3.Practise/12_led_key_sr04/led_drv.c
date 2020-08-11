#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>		//字符设备
#include <linux/fs.h>		//file_operations
#include <linux/uaccess.h>	//copy_from_user、copy_to_user
#include <linux/device.h>	//class_create、device_create
#include <linux/ioport.h>	//request_mem_region 
#include <linux/io.h>		//ioremap

//声明一个led字符设备的结构体
static struct cdev led_cdev;	

//声明一个led的设备号
static dev_t led_num;

//声明一个led的类指针
static struct class  *led_class;

//声明一个led的设备指针
static struct device  *led_device;

//声明一个led的资源指针
static struct resource *led_resource_e;
static struct resource *led_resource_c;

//声明一个gpioe起始虚拟地址的指针
//D7--->GPIOE13
void __iomem *gpioe_va=NULL;
void __iomem *gpioe_out_va=NULL;
void __iomem *gpioe_outenb_va=NULL;
void __iomem *gpioe_atlfn0_va=NULL;
//D8 --->GPIOC17
// D9 --->GPIOC8
// D10--->GPIOC7	
void __iomem *gpioc_va=NULL;
void __iomem *gpioc_out_va=NULL;
void __iomem *gpioc_outenb_va=NULL;
void __iomem *gpioc_atlfn0_va=NULL;
void __iomem *gpioc_atlfn1_va=NULL;

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
	unsigned long value;
	char kbuf[2]={0};
	
	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	if(len >sizeof kbuf)
		len = sizeof kbuf;

	//拷贝用户空间的数据，类似于memcpy
	//返回没有被成功拷贝的字节数
	rt = copy_from_user(kbuf,buf,len);

	if(rt != 0)
		return -EFAULT;
	
	switch(kbuf[0])
	{
		case 7://D7灯的亮与灭
		{
			if(kbuf[1] == 1)
			{
				iowrite32(ioread32(gpioe_out_va)&(~(1<<13)) ,gpioe_out_va );
			}
			else if(kbuf[1] == 0)
			{
				iowrite32(ioread32(gpioe_out_va)|(1<<13) , gpioe_out_va );
			}
			else
			{
				return -EINVAL;
			}
		}
		break;
		case 8://D8灯的亮与灭
		{
			if(kbuf[1] == 1)
			{
				// (*(volatile unsigned int *)gpioc_out_va)&=~(1<<17);
				value = ioread32(gpioc_out_va);
				value &=~(1<<17);
				iowrite32(value , gpioc_out_va );
			}
			else if(kbuf[1] == 0)
			{
				// (*(volatile unsigned int *)gpioc_out_va)|=(1<<17);
				value = ioread32(gpioc_out_va);
				value |=(1<<17);
				iowrite32(value , gpioc_out_va );
			}
			else
				return -EINVAL;
		}
		break;
		case 9://D9灯的亮与灭
		{
			if(kbuf[1] == 1)
			{
				// (*(volatile unsigned int *)gpioc_out_va)&=~(1<<8);
				value = ioread32(gpioc_out_va);
				value &=~(1<<8);
				iowrite32(value , gpioc_out_va );
			}
				
			else if(kbuf[1] == 0)
			{
				// (*(volatile unsigned int *)gpioc_out_va)|=(1<<8);	
				value = ioread32(gpioc_out_va);
				value |=(1<<8);
				iowrite32(value , gpioc_out_va );			
			}

			else
				return -EINVAL;
		}
		break;
		case 10://D10灯的亮与灭
		{
			if(kbuf[1] == 1)
			{
				// (*(volatile unsigned int *)gpioc_out_va)&=~(1<<7);
				value = ioread32(gpioc_out_va);
				value &=~(1<<7);
				iowrite32(value , gpioc_out_va );
			}
				
			else if(kbuf[1] == 0)
			{
				// (*(volatile unsigned int *)gpioc_out_va)|=(1<<7);
				value = ioread32(gpioc_out_va);
				value |=(1<<7);
				iowrite32(value , gpioc_out_va );
			}
				
			else
				return -EINVAL;
		}
		break;

		default:
			return -EINVAL;
			break;
	}
	
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
	
	//申请物理地址空间
	//release_mem_region(0xC001E000,0x28);
	led_resource_e=request_mem_region(0xC001E000,0x28,"gpioe");
	if(led_resource_e == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"request_mem_region error\n");
		
		goto err_request_mem_region_gpioe;
	}
	
	//由于linux需要虚拟地址访问，将物理地址转换为虚拟地址
	gpioe_out_va=ioremap(0xC001E000,0x28);	
	if(gpioe_out_va == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"ioremap error\n");
		
		goto err_ioremape;
	}	
	
	//得到相应的寄存器虚拟地址
	gpioe_out_va    = gpioe_out_va+0x00;
	gpioe_outenb_va = gpioe_out_va+0x04;
	gpioe_atlfn0_va = gpioe_out_va+0x20;

	//配置GPIOE13为GPIO模式
	(*(volatile unsigned int *)gpioe_atlfn0_va)&=~(3<<26);		
	
	//配置GPIOE13为输出功能
	(*(volatile unsigned int *)gpioe_outenb_va)|=1<<13;
	
	//申请物理地址空间
	//release_mem_region(0xC001C000,0x28);
	led_resource_c=request_mem_region(0xC001C000,0x28,"gpioc");
	
	if(led_resource_c == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"request_mem_region error\n");
		
		goto err_request_mem_region_gpioc;
	}
	
	//由于linux需要虚拟地址访问，将物理地址转换为虚拟地址
	gpioc_out_va=ioremap(0xC001C000,0x28);
	
	if(gpioc_out_va == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"ioremap error\n");
		
		goto err_ioremapc;
	}	
	
	
	//得到相应的寄存器虚拟地址
	gpioc_out_va    = gpioc_out_va+0x00;
	gpioc_outenb_va = gpioc_out_va+0x04;
	gpioc_atlfn0_va = gpioc_out_va+0x20;
	gpioc_atlfn1_va = gpioc_out_va+0x24;

	//配置GPIOC17为GPIO模式
	(*(volatile unsigned int *)gpioc_atlfn1_va)&=~(3<<2);		
	(*(volatile unsigned int *)gpioc_atlfn1_va)|=1<<2;	
	
	//配置GPIOC17为输出功能
	(*(volatile unsigned int *)gpioc_outenb_va)|=1<<17;

	//配置GPIOC8为GPIO模式
	(*(volatile unsigned int *)gpioc_atlfn0_va)&=~(3<<16);		
	(*(volatile unsigned int *)gpioc_atlfn0_va)|=1<<16;	
	
	//配置GPIOC8为输出功能
	(*(volatile unsigned int *)gpioc_outenb_va)|=1<<8;

	//配置GPIOC7为GPIO模式
	(*(volatile unsigned int *)gpioc_atlfn0_va)&=~(3<<14);		
	(*(volatile unsigned int *)gpioc_atlfn0_va)|=1<<14;	
	
	//配置GPIOC7为输出功能
	(*(volatile unsigned int *)gpioc_outenb_va)|=1<<7;
	
	printk(KERN_INFO"myled_init\n");

	//成功返回
	return 0;

	


err_ioremapc:
	release_mem_region(0xC001C000,0x28);	
err_ioremape:
	release_mem_region(0xC001E000,0x28);

err_request_mem_region_gpioc:
	device_destroy(led_class,led_num);
err_request_mem_region_gpioe:
	device_destroy(led_class,led_num);
	
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
	//解除映射
	iounmap(gpioe_out_va);
	iounmap(gpioc_out_va);
	//释放内存空间
	release_mem_region(0xC001E000,0x28);
	release_mem_region(0xC001C000,0x28);
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