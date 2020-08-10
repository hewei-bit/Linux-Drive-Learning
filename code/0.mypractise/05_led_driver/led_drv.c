#include <linux/init.h>   //__init __exit
#include <linux/kernel.h> //printk
#include <linux/module.h> //module_init module_exit
#include <linux/printk.h>

static int baud = 9600;
static int port[4] = {0,1,2,3};
static char * name = "vcom";

static int port_cnt = 0;

//如果想通过加载驱动修改变量,需要加载module_param
module_param(baud,int,0644);
module_param_array(port,int ,&port_cnt,0644);
module_param(name,charp,0644);


static int __init myled_init(void)
{
    printk("<6>""myled_init\n");

    printk(KERN_INFO"baud = %d\n",baud);
    printk(KERN_INFO"port = %d %d %d %d\n",port[0],port[1],port[2],port[3]);	
	printk(KERN_INFO"name = %s\n",name);		
	printk(KERN_INFO"port_cnt = %d\n",port_cnt);

    // 成功返回 0
    return 0;
}

static void __exit myled_exit(void)
{
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