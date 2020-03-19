#include <linux/module.h> // included for all kernel modules
#include <linux/kernel.h> // included for KERN_INFO
#include <linux/init.h>   // included for __init and __exit macros
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Aymen Sekhri");
MODULE_DESCRIPTION("A Simple Hello World module");
dev_t majMin;
const int numOfDevices = 4;
struct device_info
{
	char *data;
	struct cdev chardev;
};

struct device_info *charDevices;

#define DEVICE_BLOCK_SIZE 100

static void device_info_cleanup(void);

int device_open(struct inode *myinode, struct file *fd)
{
	printk(KERN_INFO "MyLinuxModule: device has been opened.\n");
	return 0;
}

int device_release(struct inode *myinode, struct file *fd)
{
	printk(KERN_INFO "MyLinuxModule: device has been released.\n");
	return 0;
}
ssize_t device_read(struct file *fd, char __user *buf, size_t size, loff_t *offset)
{
	
	printk(KERN_INFO "MyLinuxModule: device is being read from.\n");
	if (*offset < DEVICE_BLOCK_SIZE)
	{
		int true_len = (size > DEVICE_BLOCK_SIZE ? DEVICE_BLOCK_SIZE : size);
		copy_to_user(buf, charDevices[MINOR(fd->f_inode->i_rdev)].data, true_len);
		*offset = *offset + true_len;
		return true_len;
	}
	else
	{
		return 0;
	}
}

ssize_t device_write(struct file *fd,const char __user *buf, size_t size, loff_t *offset)
{
	printk(KERN_INFO "MyLinuxModule: device is being written to.\n");
	if (*offset < DEVICE_BLOCK_SIZE)
	{
		int true_len = (size > DEVICE_BLOCK_SIZE ? DEVICE_BLOCK_SIZE : size);
		copy_from_user(charDevices[MINOR(fd->f_inode->i_rdev)].data, buf, true_len);
		*offset = *offset + true_len;
		return true_len;
	}
	else
	{
		return 0;
	}
}

struct file_operations myfops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.llseek = 0,
	.unlocked_ioctl = 0};

int setup_cdevice(int index)
{
	char *device_message = "Hello from the other side\n";
	//initiate the cdev struct
	cdev_init(&charDevices[index].chardev, &myfops);
	//initiate the data
	charDevices[index].data = kmalloc(DEVICE_BLOCK_SIZE, GFP_KERNEL);
	memcpy(charDevices[index].data, device_message, strlen(device_message));

	int err = cdev_add(&charDevices[index].chardev, MKDEV(MAJOR(majMin), MINOR(majMin) + index), numOfDevices);
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
			device_info_cleanup();
			unregister_chrdev_region(majMin, numOfDevices);
			goto freeingMem;
		}
	}

	return 0; // Non-zero return means that the module couldn't be loaded.
freeingMem:
	kfree(charDevices);
	return -1;
}

static void device_info_cleanup()
{
	for (size_t i = 0; i < numOfDevices; i++)
	{
		cdev_del(&charDevices[i].chardev);
		if (charDevices[i].data)
		{
			kfree(charDevices[i].data);
		}
	}
}

static void hello_cleanup(void)
{

	device_info_cleanup();
	kfree(charDevices);
	unregister_chrdev_region(majMin, numOfDevices);
	printk(KERN_INFO "MyLinuxModule: Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);