# Character Drivers
## Module vs Driver vs Device
#### Module
A executable program that has been compiled and loaded to the kernel via *insmod*.
#### Driver
A piece of code in the module that uses the device.
#### Device 
Is either a physical or virtual hardware which will be communicated by its drivers.

## Registering a Char Device

### Minor and Major numbers
Each device has a unique major number, each device can have multiple minor numbers they will act as some instances of it.

### alloc_chrdev_region
``` int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);```<br>
alloc_chrdev_region is used to register a major and minor numbers dynamically for the device.

### register_chrdev_region
``` int register_chrdev_region(dev_t first, unsigned int count,char *name);```<br>
register_chrdev_region is used to register predefined major and minor numbers.


### MAJOR ,MINOR and MKDEV Macros
```
MAJOR(dev_t dev);// get major from dev_t
MINOR(dev_t dev);// get minor from dev_t
MKDEV(int major, int minor);//major/minor from dev_t
```

### /proc/devices
This folder contains all the devices defined on the system with their Major number.

### *mknod* Command
*mknod* used to link the character device to the file system into the folder /dev/.<br>
the option "c" is used to indicate that's a character device, and the major and minor numbers should be passed.

### *file* Structure

*file* is a file descriptor used to identify the handle returned by open system call.<br>
it contains information like *file_operations* structure, read/write mode, private_data and dentry.<br>
dentry is contains pointer to *inode* structure.<br>

### *inode* Structure
contains some information, includes dev_t (which are the major and minor versions of the device) and *cdev* which identifies the character device.

### *cdev* Structure
most important field of it is file_operations which is a structure to identify the function called for files system calls (open,read, write ...).
### *cdev_init*
```void cdev_init(struct cdev *cdev, struct file_operations *fops);```<br>
this function is used to initialize the allocated cdev structure and link file_operations structure to it.<br>
idk why cant we link it directly by ``` cdev->file_operations = myops; ``` maybe it does something other than just linking myops.<br>
Note: *cdev* should be allocated beforehand or use *cdev_alloc*;<br>
Note2 : the *cdev* structure should be freed in the module clean up function.<br>

### *cdev_add*
``` int cdev_add(struct cdev *dev, dev_t num, unsigned int count);```<br>
this function is used to register cdev structure in the kernel.

### *cdev_del*
``` void cdev_del(struct cdev *dev); ``` 
this function is used to unregister cdev structure in the kernel.
