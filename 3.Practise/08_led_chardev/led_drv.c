#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit...
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

//声明一个led字符设备的结构体
static struct cdev led_cdev;
//声明一个led的设备号
static dev_t led_num;

int led_open(struct inode *inode,struct file *file)
{
    printk(KERN_INFO"led_open \n");
    return 0;
}


int led_close(struct inode *inode,struct file *file)
{
    printk(KERN_INFO"led_close \n");
    return 0;
}

ssize_t led_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    int rt;
    char kbuf[16] = {0};

    if(buf == NULL)
        return -EINVAL;	//判断参数的合法性，若是非法，就返回EINVAL
    
    if(len > sizeof(kbuf))
    {
        len = 16;
    }
	//拷贝用户空间的数据，类似于memcpy
	//返回没有被成功拷贝的字节数
    rt = copy_from_user(kbuf,buf,len);

    len = len-rt;

    printk(KERN_INFO"kbuf:%s\n",kbuf);

    return len;

}

ssize_t led_read(struct file *file, char __user *buf, size_t len, loff_t * offs)
{
    int rt;
    char kbuf[5] =  {'1','2','3','4','\0'};

    if(len > sizeof kbuf)
    {
        return -EINVAL;
    }

    rt = copy_to_user(buf,kbuf,len);

    len = len -rt;
    
    printk(KERN_ERR"gec6818_led_read,__user buf[%s],len[%d]\n",buf,len);	

	return len;
}


struct file_operations led_fops ={
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_close,
    .write = led_write,
    .read = led_read,
};

static int __init myled_init(void)
{
    int rt;
    //构建设备号
    led_num = MKDEV(240,0);
    //申请设备号
    rt = register_chrdev_region(led_num,1,"myled");

    if(rt<0)
    {
        printk("register_chrdev_region error \n");

        goto err_register_chrdev_region;
    }

    //字符设备初始化
    cdev_init(&led_cdev,&led_fops);

    //将字符设备加到内核
    rt = cdev_add(&led_cdev,led_num,1);

    if(rt<0)
    {
        printk("cdev_add error \n");

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

    printk(KERN_ERR "myled_exit \n");
}

//加载驱动
module_init(myled_init);
// 卸载驱动
module_exit(myled_exit);

// 添加作者
MODULE_AUTHOR("master wei");
// 添加模块描述
MODULE_DESCRIPTION("This is  a led driver");
// 添加GPL许可证
MODULE_LICENSE("GPL");