#include <linux/init.h>   //__init __exit
#include <linux/kernel.h> //printk
#include <linux/module.h> //module_init module_exit
#include <linux/printk.h>

static int __init myled_init(void)
{
    printk("<3>"
           "myled_init\n");
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