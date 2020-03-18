#include <linux/module.h> // included for all kernel modules
#include <linux/kernel.h> // included for KERN_INFO
#include <linux/init.h>   // included for __init and __exit macros
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Aymen Sekhri");
MODULE_DESCRIPTION("A Simple Hello World module");
dev_t majMin;
const int numOfDevices = 4;
struct device_info
{
	char *data;
	int max_data_avaliable;
	struct cdev *chardev;
	struct mutex my_mutex; 
	struct wait_queue_head my_queue;
};

struct device_info *charDevices;

#define DEVICE_BLOCK_SIZE 1000

static void device_info_cleanup(void);
static int my_min(int num1, int num2)
{
	if (num1 < 0 || num2 < 0)
		return 0;
	if (num1 < num2)
		return num1;
	else
		return num2;
}
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
	struct device_info mydevice = charDevices[MINOR(fd->f_inode->i_rdev)];
	if (mutex_lock_interruptible(&mydevice.my_mutex))
		return -ERESTARTSYS;
	while (*offset > mydevice.max_data_avaliable)
	{
		mutex_unlock(&mydevice.my_mutex);
		if (fd->f_flags & O_NONBLOCK) //Non Blocking IO
			return -EAGAIN;
		if (wait_event_interruptible(mydevice.my_queue, *offset <= mydevice.max_data_avaliable))
			return -ERESTARTSYS;
		if (mutex_lock_interruptible(&mydevice.my_mutex))
			return -ERESTARTSYS;
	}
	int true_len = my_min(size, mydevice.max_data_avaliable - *offset);
	copy_to_user(buf, mydevice.data, true_len);
	mutex_unlock(&mydevice.my_mutex);
	*offset += true_len;
	return true_len;
}

ssize_t device_write(struct file *fd, const char __user *buf, size_t size, loff_t *offset)
{
	printk(KERN_INFO "MyLinuxModule: device is being written to.\n");
	struct device_info mydevice = charDevices[MINOR(fd->f_inode->i_rdev)];
	if (*offset > DEVICE_BLOCK_SIZE)//reached Max size 
	{
		return 0;
	}
	int true_len = my_min(size, DEVICE_BLOCK_SIZE - *offset);
	if (mutex_lock_interruptible(&mydevice.my_mutex))
		return -ERESTARTSYS;
	mydevice.max_data_avaliable += true_len;
	copy_from_user(mydevice.data, buf, true_len);
	mutex_unlock(&mydevice.my_mutex);
	*offset += true_len;
	return true_len;
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
	charDevices[index].chardev = kmalloc(sizeof(struct cdev), GFP_KERNEL);
	cdev_init(charDevices[index].chardev, &myfops);
	//initiate the data
	charDevices[index].data = kmalloc(DEVICE_BLOCK_SIZE, GFP_KERNEL);
	memcpy(charDevices[index].data, device_message, strlen(device_message));
	//initiate the semaphore & queue
	mutex_init(&charDevices[index].my_mutex);
	mutex_unlock(&charDevices[index].my_mutex);
	//init_waitqueue_head(&(charDevices[index].my_queue));
	charDevices[index].max_data_avaliable = 0;
	int err = cdev_add(charDevices[index].chardev, MKDEV(MAJOR(majMin), MINOR(majMin) + index), numOfDevices);
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
	int err = alloc_chrdev_region(&majMin, 0, numOfDevices, "char"); // register 4 of them
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
			//TODO: free others too.
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
		if (charDevices[i].chardev)
		{
			cdev_del(charDevices[i].chardev);
			kfree(charDevices[i].chardev);
		}
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
