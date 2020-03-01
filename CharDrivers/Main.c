
#include <linux/module.h> // included for all kernel modules
#include <linux/kernel.h> // included for KERN_INFO
#include <linux/init.h>   // included for __init and __exit macros
#include <linux/version.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Aymen Sekhri");
MODULE_DESCRIPTION("A Simple Hello World module");

static int __init hello_init(void)
{
    dev_t majMin;
    int err = alloc_chrdev_region(&majMin, 0, 4, "myNewDevice");// register 4 of them
    if (err == 0)
    {
        printk(KERN_INFO "MyLinuxModule: Major Number : %d Minor Number %d\n", MAJOR(majMin),MINOR(majMin));
    }else{
        printk(KERN_INFO "MyLinuxModule: Error : %X\n", err);
    }   
    return 0; // Non-zero return means that the module couldn't be loaded.
}

static void hello_cleanup(void)
{
    printk(KERN_INFO "MyLinuxModule: Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
