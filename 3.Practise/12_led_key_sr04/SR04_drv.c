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

//声明一个SR04字符设备的结构体
static struct cdev SR04_cdev;	

//声明一个SR04的设备号
static dev_t SR04_num;

//声明一个SR04的类指针
static struct class  *SR04_class;

//声明一个SR04的设备指针
static struct device  *SR04_device;

//声明一个SR04的资源指针
static struct resource *SR04_resource;

//声明一个gpiod起始虚拟地址的指针
//GPIOD19   trig
//GPIO15    echo
void __iomem *gpiod_va=NULL;
void __iomem *gpiod_out_va=NULL;
void __iomem *gpiod_outenb_va=NULL;
void __iomem *gpiod_atlfn0_va=NULL;
void __iomem *gpiod_atlfn1_va=NULL;
void __iomem *gpiod_pad_va=NULL;


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
	iowrite32(ioread32(gpiod_out_va)|(1<<19),gpiod_out_va);

	udelay(10);

	iowrite32( ioread32(gpiod_out_va)&(~(1<<19)) ,gpiod_out_va);

	//等待信号返回
	//添加超时处理
	t=0;
	while( (ioread32(gpiod_pad_va)&(1<<15) )== 0 )
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
		while( (ioread32(gpiod_pad_va)&(1<<15)) )
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

ssize_t SR04_write (struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	int rt;
	unsigned long value;
	char kbuf[8];
	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	if(len >sizeof(kbuf))
		len = sizeof (kbuf);

	//拷贝用户空间的数据，类似于memcpy
	//返回没有被成功拷贝的字节数
	rt = copy_from_user(kbuf,buf,len);

	if(rt != 0)
		return -EFAULT;

	value = ioread32(gpiod_out_va);
	value &=~(1<<13);
	iowrite32(value ,gpiod_out_va );

	//返回实际写入数据的长度
	return len;
}

struct file_operations SR04_fops={
	.owner = THIS_MODULE,
	.open = SR04_open,
	.release = SR04_close,
	.write = SR04_write,
	.read = SR04_read,
};

static int __init mySR04_init(void)
{
	int rt;
	int value;

	//构建设备号,主设备号为240，次设备号为0
	SR04_num=MKDEV(240,2);
	
	//申请设备号，申请一个字符设备，若成功，会在/proc/devices文件当中显示mySR04的名字
	rt = register_chrdev_region(SR04_num,1,"mySR04");
	if(rt < 0)
	{
		printk(KERN_INFO"register_chrdev_region error\n");
		goto err_register_chrdev_region;
	}
	
	//字符设备的初始化
	cdev_init(&SR04_cdev,&SR04_fops);
	
	//将字符设备加入到内核
	rt = cdev_add(&SR04_cdev,SR04_num,1);
	if(rt < 0)
	{
		printk(KERN_INFO"cdev_add error\n");
		goto err_cdev_add;
	}
	
	//创建mySR04的设备类,可以在/sys/class目录当中找到
	SR04_class=class_create(THIS_MODULE,"mySR04");
	
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(SR04_class))
	{
		printk(KERN_INFO"class_create error\n");
		//将指针转换为错误码
		rt = PTR_ERR(SR04_class);
		goto err_class_create;
		
	}
	
	//创建mySR04的设备信息:设备号、设备文件名
	SR04_device=device_create(SR04_class, NULL, SR04_num, NULL, "mySR04");
		
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(SR04_device))
	{
		printk(KERN_INFO"device_create error\n");
		//将指针转换为错误码
		rt = PTR_ERR(SR04_device);
		goto err_device_create;
	}	
	
	//申请物理地址空间
	//release_mem_region(0xC001D000,0x28);
	SR04_resource=request_mem_region(0xC001D000,0x28,"gpiod");
	if(SR04_resource == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"request_mem_region error\n");
		
		goto err_request_mem_region_gpiod;
	}
	
	//由于linux需要虚拟地址访问，将物理地址转换为虚拟地址
	gpiod_out_va = ioremap(0xC001D000,0x28);
	if(gpiod_out_va == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"ioremap error\n");
		
		goto err_ioremapd;
	}	
		
	//得到相应的寄存器虚拟地址
	gpiod_outenb_va = gpiod_out_va+0x04;
	gpiod_pad_va = gpiod_out_va+0x18;
	gpiod_atlfn0_va = gpiod_out_va+0x20;
	gpiod_atlfn1_va = gpiod_out_va+0x24;

	//配置GPIOD19为输出模式
	// (*(volatile unsigned int *)gpiod_atlfn0_va)&=~(3<<30);
	value = ioread32(gpiod_atlfn0_va);
	value &=~(3<<30);
	iowrite32(value, gpiod_atlfn0_va );

	// (*(volatile unsigned int *)gpiod_outenb_va)|=1<<19;
	value = ioread32(gpiod_outenb_va);
	value |=(1<<19);
	iowrite32(value , gpiod_outenb_va );

	//配置GPIOD15为接收功能
	// (*(volatile unsigned int *)gpiod_atlfn0_va)&=~(3<<6);	
	value = ioread32(gpiod_atlfn0_va);
	value &=~(3<<6);
	iowrite32(value , gpiod_atlfn0_va );

	// (*(volatile unsigned int *)gpiod_outenb_va)|=1<<15;	
	value = ioread32(gpiod_outenb_va);
	value &=~(1<<15);
	iowrite32(value , gpiod_outenb_va );

	printk(KERN_INFO"mySR04_init\n");

	//成功返回
	return 0;
	
err_ioremapd:
	release_mem_region(0xC001D000,0x28);	
err_request_mem_region_gpiod:
	device_destroy(SR04_class,SR04_num);
err_device_create:
	class_destroy(SR04_class);	
err_class_create:
	cdev_del(&SR04_cdev);
err_cdev_add:	
	unregister_chrdev_region(SR04_num,1);
err_register_chrdev_region:
	return rt;
}

static void __exit mySR04_exit(void)
{
	//解除映射
	iounmap(gpiod_out_va);
	//释放内存空间
	release_mem_region(0xC001D000,0x28);
	//设备的销毁
	device_destroy(SR04_class,SR04_num);
	//类的销毁
	class_destroy(SR04_class);
	//删除字符设备
	cdev_del(&SR04_cdev);
	//注销设备号
	unregister_chrdev_region(SR04_num,1);
	
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