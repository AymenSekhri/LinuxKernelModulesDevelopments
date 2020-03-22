#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h> // included for __init and __exit macros
#include <linux/kernel.h> // included for KERN_INFO
#include <linux/module.h> // included for all kernel modules
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/wait.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Aymen Sekhri");
MODULE_DESCRIPTION("A Simple Hello World module");
dev_t majMin;
const int numOfDevices = 4;
struct device_info {
    char* data;
    int max_data_avaliable;
    struct cdev chardev;
    struct mutex my_mutex;
    struct wait_queue_head my_queue;
};

struct device_info* charDevices;

#define DEVICE_BLOCK_SIZE 1000

static void device_info_cleanup(void);
int setup_cdevice(int index);

int device_open(struct inode* myinode, struct file* fd)
{
    printk(KERN_INFO "MyLinuxModule: device has been opened,     fd=%X.\n", fd);
    return 0;
}

int device_release(struct inode* myinode, struct file* fd)
{
    printk(KERN_INFO "MyLinuxModule: device has been released,   fd=%X.\n", fd);
    return 0;
}
ssize_t device_read(struct file* fd, char __user* buf, size_t size,
    loff_t* offset)
{
    printk(KERN_INFO "MyLinuxModule: device is being read from,  fd=%X.\n", fd);
    struct device_info mydevice = charDevices[MINOR(fd->f_inode->i_rdev)];
    if (mutex_lock_interruptible(&mydevice.my_mutex))
        return -ERESTARTSYS;
    while (*offset >= mydevice.max_data_avaliable) {
		printk(KERN_INFO "MyLinuxModule: device is blocking...,      fd=%X.\n", fd);
        mutex_unlock(&mydevice.my_mutex);
        /*if (fd->f_flags & O_NONBLOCK) // Non Blocking IO
            return -EAGAIN;*/
        if (wait_event_interruptible(mydevice.my_queue,
                *offset < mydevice.max_data_avaliable))
            return -ERESTARTSYS;
        if (mutex_lock_interruptible(&mydevice.my_mutex))
            return -ERESTARTSYS;
    }
    int true_len = min(size, mydevice.max_data_avaliable - *offset);
    printk(KERN_INFO "MyLinuxModule: true_len=%d, size=%d, left=%d fd=%X.\n",size, mydevice.max_data_avaliable - *offset, fd);
    copy_to_user(buf, mydevice.data, true_len);
    mutex_unlock(&mydevice.my_mutex);
    *offset += true_len;
    return true_len;
}

ssize_t device_write(struct file* fd, const char __user* buf, size_t size,
    loff_t* offset)
{
    printk(KERN_INFO "MyLinuxModule: device is being written to, fd=%X.\n", fd);
    struct device_info mydevice = charDevices[MINOR(fd->f_inode->i_rdev)];
    if (*offset > DEVICE_BLOCK_SIZE) // reached Max size
    {
        return 0;
    }
    int true_len = min(size, DEVICE_BLOCK_SIZE - *offset);
    if (mutex_lock_interruptible(&mydevice.my_mutex))
        return -ERESTARTSYS;
    copy_from_user(mydevice.data, buf, true_len);
    mydevice.max_data_avaliable += true_len;
    mutex_unlock(&mydevice.my_mutex);
	printk(KERN_INFO "MyLinuxModule: my_queue=%X.\n", &mydevice.my_queue);
	wake_up_interruptible(&mydevice.my_queue);
    *offset += true_len;
    return true_len;
}

struct file_operations myfops = { .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
    .llseek = 0,
    .unlocked_ioctl = 0 };

static int __init hello_init(void)
{
    charDevices = kmalloc(sizeof(*charDevices) * numOfDevices, GFP_KERNEL);
    if (!charDevices) {
        return -ENOMEM;
    }
    // register the char device
    int err = alloc_chrdev_region(&majMin, 0, numOfDevices,
        "char");
    if (err == 0) {
        printk(KERN_INFO "MyLinuxModule: Major Number : %d Minor Number %d\n",
            MAJOR(majMin), MINOR(majMin));
    } else {
        printk(KERN_INFO "MyLinuxModule: alloc_chrdev_region Error : %X\n", err);
        goto freeingMem;
    }

    for (size_t i = 0; i < numOfDevices; i++) {
        if (setup_cdevice(i)) {
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
static void hello_cleanup(void)
{
    device_info_cleanup();
    kfree(charDevices);
    unregister_chrdev_region(majMin, numOfDevices);
    printk(KERN_INFO "MyLinuxModule: Cleaning up module.\n");
}
int setup_cdevice(int index)
{
    char* device_message = "Hello from the other side\n";
    // initiate the cdev struct
    cdev_init(&charDevices[index].chardev, &myfops);
    // initiate the data
    charDevices[index].data = kmalloc(DEVICE_BLOCK_SIZE, GFP_KERNEL);
    memcpy(charDevices[index].data, device_message, strlen(device_message));
    // initiate the semaphore & queue
    mutex_init(&charDevices[index].my_mutex);
    init_waitqueue_head(&charDevices[index].my_queue);
    charDevices[index].max_data_avaliable = 0;
    int err = cdev_add(&charDevices[index].chardev,
        MKDEV(MAJOR(majMin), MINOR(majMin) + index), numOfDevices);
    if (err) {
        printk(KERN_INFO "MyLinuxModule: cdev_add Error : %X\n", err);
        return -1;
    }
    return 0;
}
static void device_info_cleanup()
{
    for (size_t i = 0; i < numOfDevices; i++) {
        if (&charDevices[i].chardev) {
            cdev_del(&charDevices[i].chardev);
        }
        if (charDevices[i].data) {
            kfree(charDevices[i].data);
        }
    }
}



module_init(hello_init);
module_exit(hello_cleanup);
