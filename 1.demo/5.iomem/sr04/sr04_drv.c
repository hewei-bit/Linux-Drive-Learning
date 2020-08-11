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

//声明一个sr04字符设备的结构体
static struct cdev sr04_cdev;	

//声明一个sr04的设备号
static dev_t sr04_num;

//声明一个sr04的类指针
static struct class  *sr04_class;

//声明一个sr04的设备指针
static struct device  *sr04_device;

//声明一个sr04的资源指针
static struct resource *sr04_resource;

//声明一个gpiod起始虚拟地址的指针
void __iomem *gpiod_va=NULL;
void __iomem *gpiod_out_va=NULL;
void __iomem *gpiod_outenb_va=NULL;
void __iomem *gpiod_atlfn0_va=NULL;
void __iomem *gpiod_atlfn1_va=NULL;
void __iomem *gpiod_pad_va=NULL;

int sr04_open (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"sr04_open\n");
	return 0;
}

int sr04_close (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"sr04_close\n");
	return 0;
}

int sr04_get_distance(void)
{
	unsigned int t=0;
	int d=0;
	
	//触发超声波模块进行工作
	iowrite32(ioread32(gpiod_out_va)|(1<<19),gpiod_out_va);
	
	udelay(10);
	
	iowrite32(ioread32(gpiod_out_va)&(~(1<<19)),gpiod_out_va);
	
	
	//等待信号返回
	//添加超时处理，以防硬件问题（超声波模块的硬件故障、短路现象）让程序一直阻塞等待
	t=0;
	while((ioread32(gpiod_pad_va) & (1<<15)) == 0)
	{
		t++;
		
		udelay(1);
		
		if(t  >= 1000000)
			return -1;
	}
	
	//有信号返回
	//添加超时处理，以防硬件问题（超声波模块的硬件故障、短路现象）让程序一直阻塞等待
	t=0;
	while((ioread32(gpiod_pad_va) & (1<<15)))
	{
		t++;
		
		udelay(5);		//超声波传输9us，等同于传输了3mm
		
		
		//若高电平时间持续过长
		if(t >= 1000000)
			return -2;	
	
	}

	//将时间换算为距离
	//3mm 传输时间约为 9us
	
	t=t/2;
	
	d= 3*t;

	return d;
}


ssize_t sr04_read (struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	int rt;
	int distance;
	
	char sr04_val=0;
	
	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
	
	if(len >sizeof sr04_val)
		len = sizeof sr04_val;	
	
	//读取超声波测距
	distance=sr04_get_distance();
	
	rt = copy_to_user(buf,&distance,sizeof distance);
	
	//得到实际拷贝的字节数
	len = len - rt;	
	
	//返回实际读取的字节数
	return len;
}





struct file_operations sr04_fops={
	.owner = THIS_MODULE,
	.open = sr04_open,
	.release = sr04_close,
	.read = sr04_read,
};

static int __init mysr04_init(void)
{
	int rt;
	
	int v;

	//构建设备号,主设备号为240，次设备号为2
	sr04_num=MKDEV(240,2);
	
	
	//申请设备号，申请一个字符设备，若成功，会在/proc/devices文件当中显示mysr04的名字
	rt = register_chrdev_region(sr04_num,1,"mysr04");
	
	if(rt < 0)
	{
		
		printk(KERN_INFO"register_chrdev_region error\n");
		
		goto err_register_chrdev_region;
		
	}
	
	//字符设备的初始化
	cdev_init(&sr04_cdev,&sr04_fops);
	
	//将字符设备加入到内核
	rt = cdev_add(&sr04_cdev,sr04_num,1);
	
	if(rt < 0)
	{
		
		printk(KERN_INFO"cdev_add error\n");
		
		goto err_cdev_add;
		
	}
	
	//创建mysr04的设备类,可以在/sys/class目录当中找到
	sr04_class=class_create(THIS_MODULE,"mysr04");
	
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(sr04_class))
	{
		printk(KERN_INFO"class_create error\n");
		
		//将指针转换为错误码
		rt = PTR_ERR(sr04_class);
		
		goto err_class_create;
		
		
	}
	
	//创建mysr04的设备信息:设备号、设备文件名
	sr04_device=device_create(sr04_class, NULL, sr04_num, NULL, "mysr04");
		
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(sr04_device))
	{
		printk(KERN_INFO"device_create error\n");
		
		//将指针转换为错误码
		rt = PTR_ERR(sr04_device);
		
		goto err_device_create;
	}	
	
	
	//申请物理地址空间
	sr04_resource=request_mem_region(0xC001D000,0x28,"gpiod");
	
	if(sr04_resource == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"request_mem_region error\n");
		
		goto err_request_mem_region_gpiod;
	}
	
	//由于linux需要虚拟地址访问，将物理地址转换为虚拟地址
	gpiod_va=ioremap(0xC001D000,0x28);
	
	if(gpiod_va == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"ioremap error\n");
		
		goto err_ioremap_gpiod;
	}	
	
	
	//得到相应的寄存器虚拟地址
	gpiod_out_va    = gpiod_va+0x00;
	gpiod_outenb_va = gpiod_va+0x04;
	gpiod_pad_va    = gpiod_va+0x18;
	gpiod_atlfn0_va = gpiod_va+0x20;
	gpiod_atlfn1_va = gpiod_va+0x24;


	//配置gpiod15为GPIO模式
	v = ioread32(gpiod_atlfn0_va);
	v &=~(3<<30);
	iowrite32(v,gpiod_atlfn0_va);

	//配置gpiod19为GPIO模式
	v = ioread32(gpiod_atlfn1_va);
	v &=~(3<<6);
	iowrite32(v,gpiod_atlfn1_va);	
	

	//配置gpiod15为输入功能
	v = ioread32(gpiod_outenb_va);
	v &=~(1<<15);
	iowrite32(v,gpiod_outenb_va);

	//配置gpiod19为输出功能
	v = ioread32(gpiod_outenb_va);
	v |=(1<<19);
	iowrite32(v,gpiod_outenb_va);
	
	//只要是输出引脚，有初始电平状态，根据超声波模块的使用技巧，连接TRIG引脚必须为低电平
	iowrite32(ioread32(gpiod_out_va)&(~(1<<19)),gpiod_out_va);

	
	printk(KERN_INFO"mysr04_init\n");

	//成功返回
	return 0;
	
err_ioremap_gpiod:
	release_mem_region(0xC001D000,0x28);
	
err_request_mem_region_gpiod:
	device_destroy(sr04_class,sr04_num);
	
err_device_create:
	class_destroy(sr04_class);	
	
err_class_create:
	cdev_del(&sr04_cdev);
	
err_cdev_add:	
	unregister_chrdev_region(sr04_num,1);
	
err_register_chrdev_region:

	return rt;
	
	
	
}


static void __exit mysr04_exit(void)
{
	//解除映射

	iounmap(gpiod_pad_va);
	
	//释放内存空间
	release_mem_region(0xC001D000,0x28);
	
	//设备的销毁
	device_destroy(sr04_class,sr04_num);
	
	//类的销毁
	class_destroy(sr04_class);
	//删除字符设备
	cdev_del(&sr04_cdev);
	
	//注销设备号
	unregister_chrdev_region(sr04_num,1);
	
	
	printk(KERN_INFO"mysr04_exit\n");
}

//加载驱动：当使用insmod sr04_drv.ko的时候，会调用module_init函数，module_init函数会回调mysr04_init函数
module_init(mysr04_init);

//卸载驱动：当使用rmmod sr04_drv的时候，会调用module_exit函数，module_exit函数会回调mysr04_exit函数
module_exit(mysr04_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Teacher.Wen");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is sr04 driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");