#include <linux/init.h>   //__init __exit
#include <linux/kernel.h> //printk
#include <linux/module.h> //module_init module_exit...

int myadd(int a, int b)
{
    return (a + b);
}

static int __init sum_init(void)
{

    printk("<6>"
           "sum_init\n");

    //成功返回
    return 0;
}

static void __exit sum_exit(void)
{
    printk(KERN_WARNING "sum_exit\n");
}