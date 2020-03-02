#include <linux/module.h> // included for all kernel modules
#include <linux/kernel.h> // included for KERN_INFO
#include <linux/init.h>   // included for __init and __exit macros
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Aymen Sekhri");
MODULE_DESCRIPTION("A Simple Hello World module");
dev_t majMin;
const int numOfDevices = 4;
struct cdev **charDevices;

int device_open(struct inode *myinode, struct file *fd)
{
	return 0;
}

int device_release(struct inode *myinode, struct file *fd)
{
	return 0;
}

ssize_t device_read(struct file* fd, struct kobject * kobj, struct bin_attribute* attr,char *buff, loff_t kk, size_t size){
	return 2;
}

struct file_operations myfops = {
	.owner = THIS_MODULE,
	.read = 0,
	.write = 0,
	.open = device_open,
	.release = device_release,
	.llseek = 0,
	.unlocked_ioctl = 0};

int setup_cdevice(int index)
{
	//initiate the cdev struct
	cdev_init(charDevices[index], &myfops);

	int err = cdev_add(charDevices[index], MKDEV(MAJOR(majMin), MINOR(majMin) + index), numOfDevices);
	if (err)
	{
		printk(KERN_INFO "MyLinuxModule: cdev_add Error : %X\n", err);
		return -1;
	}
	return 0;
}
static int __init hello_init(void)
{
	charDevices = kmalloc(sizeof(charDevices) * numOfDevices, GFP_KERNEL);
	for (size_t i = 0; i < numOfDevices; i++)
	{
		charDevices[i] = kmalloc(sizeof(struct cdev), GFP_KERNEL);
	}
	//register the char device
	int err = alloc_chrdev_region(&majMin, 0, numOfDevices, "myCharDevice"); // register 4 of them
	if (err == 0)
	{
		printk(KERN_INFO "MyLinuxModule: Major Number : %d Minor Number %d\n", MAJOR(majMin), MINOR(majMin));
	}
	else
	{
		printk(KERN_INFO "MyLinuxModule: alloc_chrdev_region Error : %X\n", err);
		goto freeingMem;
	}

	for (size_t i = 0; i < numOfDevices; i++)
	{
		if (setup_cdevice(i))
		{
			unregister_chrdev_region(majMin, numOfDevices);
			goto freeingMem;
		}
	}

	return 0; // Non-zero return means that the module couldn't be loaded.
freeingMem:
	kfree(charDevices);
	return -1;
}

static void hello_cleanup(void)
{
	for (size_t i = 0; i < numOfDevices; i++)
	{
		if (charDevices[i])
		{
			cdev_del(charDevices[i]);
			kfree(charDevices[i]);
		}
	}

	unregister_chrdev_region(majMin, numOfDevices);
	printk(KERN_INFO "MyLinuxModule: Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
