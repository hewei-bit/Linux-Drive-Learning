#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>		//字符设备
#include <linux/fs.h>		//file_operations
#include <linux/uaccess.h>	//copy_from_user、copy_to_user
#include <linux/device.h>	//class_create、device_create
#include <linux/ioport.h>	//request_mem_region 
#include <linux/io.h>		//ioremap
//声明一个key字符设备的结构体
static struct cdev key_cdev;	

//声明一个key的设备号
static dev_t key_num;

//声明一个key的类指针
static struct class  *key_class;

//声明一个key的设备指针
static struct device  *key_device;

//声明一个key的资源指针
static struct resource *key_resource_a;
static struct resource *key_resource_b;

//声明一个gpioe起始虚拟地址的指针
void __iomem *gpioa_va=NULL;//K2
void __iomem *gpioa_pad_va=NULL;
void __iomem *gpioa_outenb_va=NULL;
void __iomem *gpioa_atlfn0_va=NULL;
void __iomem *gpioa_atlfn1_va=NULL;

void __iomem *gpiob_va=NULL;//K3 K4 K6
void __iomem *gpiob_pad_va=NULL;
void __iomem *gpiob_outenb_va=NULL;
void __iomem *gpiob_atlfn0_va=NULL;
void __iomem *gpiob_atlfn1_va=NULL;


int key_open (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"key_open\n");
	return 0;
}

int key_close (struct inode * inode, struct file *file)
{
	printk(KERN_INFO"key_close\n");
	return 0;
}

ssize_t key_read (struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	int rt;
	
	char key_val=0;
	
	unsigned int pad_a =0;
	unsigned int pad_b = 0;
	
	if(buf == NULL)
		return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
		
	if(len >sizeof key_val)
		len = sizeof key_val;	
	
	//读取按键的电平
	//K2按键的电平，对应key_val的bit0
	//K3按键的电平，对应key_val的bit1	
	//K4按键的电平，对应key_val的bit2
	//K6按键的电平，对应key_val的bit3
	
	pad_a=(*(volatile unsigned int *)gpioa_pad_va);
	pad_b=(*(volatile unsigned int *)gpiob_pad_va);
	
	//若K2按键按下，bit0就设置为1，否则为0
	key_val|=(pad_a&(1<<28))?0:1;
	key_val|=(pad_b&(1<<30))?0:(1 <<1);
	key_val|=(pad_b&(1<<31))?0:(1 << 2);
	key_val|=(pad_b&(1<<9))?0:(1 << 3);

	//添加K3、K4、K6按键的识别	
	rt = copy_to_user(buf,&key_val,sizeof key_val);
	
	//得到实际拷贝的字节数
	len = len - rt;	
	
	//返回实际读取的字节数
	return len;
}

struct file_operations key_fops={
	.owner = THIS_MODULE,
	.open = key_open,
	.release = key_close,
	.read = key_read,
};

static int __init mykey_init(void)
{
	int rt;

	//构建设备号,主设备号为240，次设备号为1
	key_num=MKDEV(240,1);
		
	//申请设备号，申请一个字符设备，若成功，会在/proc/devices文件当中显示mykey的名字
	rt = register_chrdev_region(key_num,1,"mykey");

	if(rt < 0)
	{		
		printk(KERN_INFO"register_chrdev_region error\n");
		goto err_register_chrdev_region;
	}
	
	//字符设备的初始化
	cdev_init(&key_cdev,&key_fops);
	
	//将字符设备加入到内核
	rt = cdev_add(&key_cdev,key_num,1);
	
	if(rt < 0)
	{
		printk(KERN_INFO"cdev_add error\n");
		goto err_cdev_add;
	}
	
	//创建mykey的设备类,可以在/sys/class目录当中找到
	key_class=class_create(THIS_MODULE,"mykey");
	
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(key_class))
	{
		printk(KERN_INFO"class_create error\n");
		
		//将指针转换为错误码
		rt = PTR_ERR(key_class);
		goto err_class_create;
	}
	
	//创建mykey的设备信息:设备号、设备文件名
	key_device=device_create(key_class, NULL, key_num, NULL, "mykey");
		
	//调用IS_ERR函数判断指针的合法性
	if(IS_ERR(key_device))
	{
		printk(KERN_INFO"device_create error\n");
		
		//将指针转换为错误码
		rt = PTR_ERR(key_device);
		
		goto err_device_create;
	}	
	
	
	//申请物理地址空间
	key_resource_a=request_mem_region(0xC001A000,0x28,"gpioa");
	
	if(key_resource_a == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"request_mem_region error\n");
		
		goto err_request_mem_region_gpioa;
	}
	
	//由于linux需要虚拟地址访问，将物理地址转换为虚拟地址
	gpioa_va=ioremap(0xC001A000,0x28);
	
	if(gpioa_va == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"ioremap error\n");
		
		goto err_ioremap_gpioa;
	}	
		
	//得到相应的寄存器虚拟地址
	gpioa_pad_va    = gpioa_va+0x18;
	gpioa_outenb_va = gpioa_va+0x04;
	gpioa_atlfn0_va = gpioa_va+0x20;
	gpioa_atlfn1_va = gpioa_va+0x24;


	//申请物理地址空间
	key_resource_b=request_mem_region(0xC001B000,0x28,"gpiob");
	
	if(key_resource_b == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"request_mem_region error\n");
		
		goto err_request_mem_region_gpiob;
	}
	
	//由于linux需要虚拟地址访问，将物理地址转换为虚拟地址
	gpiob_va=ioremap(0xC001B000,0x28);
	
	if(gpiob_va == NULL)
	{
		rt = -ENOMEM;
		
		printk(KERN_INFO"ioremap error\n");
		
		goto err_ioremap_gpiob;
	}	
		
	//得到相应的寄存器虚拟地址
	gpiob_pad_va    = gpiob_va+0x18;
	gpiob_outenb_va = gpiob_va+0x04;
	gpiob_atlfn0_va = gpiob_va+0x20;
	gpiob_atlfn1_va = gpiob_va+0x24;

	//配置gpioa28为GPIO模式
	(*(volatile unsigned int *)gpioa_atlfn1_va)&=~(3<<24);		
    
	//配置GPIOB30
    (*(volatile unsigned int *)gpiob_atlfn1_va) &=~(3<<28);
    (*(volatile unsigned int *)gpiob_atlfn1_va) |=1<<28;

    //配置GPIO31
    (*(volatile unsigned int *)gpiob_atlfn1_va) &=~(3<<30);
    (*(volatile unsigned int *)gpiob_atlfn1_va) |=1<<30;
    
    //配置GPIO9
    (*(volatile unsigned int *)gpiob_atlfn0_va) &=~(3<<18);

	//配置为输入功能
	(*(volatile unsigned int *)gpioa_outenb_va)&=~(1<<28);	
	(*(volatile unsigned int *)gpiob_outenb_va) &=~(1<<30);
	(*(volatile unsigned int *)gpiob_outenb_va) &=~(1<<31);
	(*(volatile unsigned int *)gpiob_outenb_va) &=~(1<<9);

	printk(KERN_INFO"mykey_init\n");

	//成功返回
	return 0;
	
err_ioremap_gpioa:
	release_mem_region(0xC001A000,0x28);
err_ioremap_gpiob:	
	release_mem_region(0xC001B000,0x28);

err_request_mem_region_gpioa:
	device_destroy(key_class,key_num);
err_request_mem_region_gpiob:
	device_destroy(key_class,key_num);	

err_device_create:
	class_destroy(key_class);	

err_class_create:
	cdev_del(&key_cdev);
	
err_cdev_add:	
	unregister_chrdev_region(key_num,1);
	
err_register_chrdev_region:
	return rt;
}

static void __exit mykey_exit(void)
{
	//解除映射
	iounmap(gpioa_pad_va);
	iounmap(gpiob_pad_va);
	
	//释放内存空间
	release_mem_region(0xC001A000,0x28);
	release_mem_region(0xC001B000,0x28);
	//设备的销毁
	device_destroy(key_class,key_num);	
	//类的销毁
	class_destroy(key_class);
	//删除字符设备
	cdev_del(&key_cdev);
	//注销设备号
	unregister_chrdev_region(key_num,1);
	
	printk(KERN_INFO"mykey_exit\n");
}

//加载驱动：当使用insmod key_drv.ko的时候，会调用module_init函数，module_init函数会回调mykey_init函数
module_init(mykey_init);

//卸载驱动：当使用rmmod key_drv的时候，会调用module_exit函数，module_exit函数会回调mykey_exit函数
module_exit(mykey_exit);

//添加作者信息[可选添加的]
MODULE_AUTHOR("Teacher.Wen");

//添加模块描述[可选添加的]
MODULE_DESCRIPTION("This is key driver");

//添加GPL许可证[必须的]
MODULE_LICENSE("GPL");