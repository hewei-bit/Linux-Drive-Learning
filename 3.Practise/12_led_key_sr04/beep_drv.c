#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>		//字符设备
#include <linux/fs.h>		//file_operations
#include <linux/uaccess.h>	//copy_from_user、copy_to_user
#include <linux/device.h>	//class_create、device_create
#include <linux/ioport.h>	//request_mem_region 
#include <linux/io.h>		//ioremap
//声明一个beep字符设备的结构体
static struct cdev beep_cdev;	

//声明一个beep的设备号
static dev_t beep_num;

//声明一个beep的类指针
static struct class  *beep_class;

//声明一个beep的设备指针
static struct device  *beep_device;

//声明一个beep的资源指针
//static struct resource *beep_resource;

//声明一个gpioc起始虚拟地址的指针

void __iomem *gpioc_va=NULL;//D8 D9 D10
void __iomem *gpioc_out_va=NULL;
void __iomem *gpioc_outenb_va=NULL;
void __iomem *gpioc_atlfn0_va=NULL;
void __iomem *gpioc_atlfn1_va=NULL;

int beep_open (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"beep_open\n");
	return 0;
}



int beep_close (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"beep_close\n");
	
	
	return 0;
}

ssize_t beep_write (struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	int rt;
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
		case 1:
		{
			if(kbuf[1] == 1)
				 iowrite32(ioread32(gpioc_out_va)|((1<<14)),gpioc_out_va);	
			else if(kbuf[1] == 0)
				 iowrite32(ioread32(gpioc_out_va)& (~((1<<14))),gpioc_out_va);	
			else
				return -EINVAL;
		}
	 }
	len=len-rt;
	//返回实际写入数据的长度
	return len;
}

struct file_operations beep_fops={
	.owner = THIS_MODULE,
	.open = beep_open,
	.release = beep_close,
	.write  = beep_write,
};

static int __init mybeep_init(void)
{
	int rt;

	//构建设备号,主设备号为240，次设备号为3
	beep_num=MKDEV(240,5);
	
	
	//申请设备号，申请一个字符设备，若成功，会在/proc/devices文件当中显示mybeep的名字
	rt = register_chrdev_region(beep_num,1,"mybeep");
	
	if(rt < 0)
	{
		
		printk(KERN_INFO"register_chrdev_region error\n");
		
		goto err_register_chrdev_region;
		
	}
	
	//字符设备的初始化
	cdev_init(&beep_cdev,&beep_fops);
	
	//将字符设备加入到内核
	rt = cdev_add(&beep_cdev,beep_num,1);
	
	if(rt < 0)
	{
		
		printk(KERN_INFO"cdev_add error\n");
		
		goto err_cdev_add;
		
	}
	
	//创建mybeep的设备类,可以在/sys/class目录当中找到
	beep_class=class_create(THIS_MODULE,"mybeep");
	
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(beep_class))
	{
		printk(KERN_INFO"class_create error\n");
		
		//将指针转换为错误码
		rt = PTR_ERR(beep_class);
		
		goto err_class_create;
		
	}
	
	//创建mybeep的设备信息:设备号、设备文件名
	beep_device=device_create(beep_class, NULL, beep_num, NULL, "mybeep");
		
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(beep_device))
	{
		printk(KERN_INFO"device_create error\n");
		
		//将指针转换为错误码
		rt = PTR_ERR(beep_device);
		
		goto err_device_create;
	}	
	
	
	//申请物理地址空间
//	beep_resource=request_mem_region(0xC001C000,0x28,"gpioc");
	
//	if(beep_resource == NULL)
//	{
//		rt = -ENOMEM;
//		
//		printk(KERN_INFO"request_mem_region error\n");
//		
//		goto err_request_mem_region_gpioc;
//	}

	//由于linux需要虚拟地址访问，将物理地址转换为虚拟地址
	gpioc_va=ioremap(0xC001C000,0x28);
	
	if(gpioc_va == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"ioremap error\n");
		
		goto err_ioremap_gpioc;
	}	
	
	//得到相应的寄存器虚拟地址
	gpioc_out_va    = gpioc_va+0x00;
	gpioc_outenb_va = gpioc_va+0x04;
	gpioc_atlfn0_va = gpioc_va+0x20;

	//配置gpioc14为GPIO模式		
    iowrite32(ioread32(gpioc_atlfn0_va)|(1<<28),gpioc_atlfn0_va);	

	//配置gpioc14为输出功能	
    iowrite32(ioread32(gpioc_outenb_va)|((1<<14)),gpioc_outenb_va);	
	
	//配置gpioc14为低电平
	 iowrite32(ioread32(gpioc_out_va)& (~((1<<14))),gpioc_out_va);	
	
	printk(KERN_INFO"mybeep_init\n");

	//成功返回
	return 0;
	
err_ioremap_gpioc:
	release_mem_region(0xC001C000,0x28);
	
// err_request_mem_region_gpioc:
// 	device_destroy(beep_class,beep_num);	
	
err_device_create:
	class_destroy(beep_class);	
	
err_class_create:
	cdev_del(&beep_cdev);
	
err_cdev_add:	
	unregister_chrdev_region(beep_num,1);
	
err_register_chrdev_region:

	return rt;
	
	
	
}


static void __exit mybeep_exit(void)
{
	//解除映射

	iounmap(gpioc_out_va);
	
	//释放内存空间
	release_mem_region(0xC001C000,0x28);
	
	//设备的销毁
	device_destroy(beep_class,beep_num);
	
	//类的销毁
	class_destroy(beep_class);
	//删除字符设备
	cdev_del(&beep_cdev);
	
	//注销设备号
	unregister_chrdev_region(beep_num,1);
	
	
	printk(KERN_INFO"mybeep_exit\n");
}

//加载驱动：当使用insmod beep_drv.ko的时候，会调用module_init函数，module_init函数会回调mybeep_init函数
module_init(mybeep_init);

//卸载驱动：当使用rmmod beep_drv的时候，会调用module_exit函数，module_exit函数会回调mybeep_exit函数
module_exit(mybeep_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Teacher.Wen");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is beep driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");