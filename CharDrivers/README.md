# Character Drivers
## Module vs Driver vs Device
#### Module
A executable program that has been compiled and loaded to the kernel via insmod.
#### Driver
A piece of code in the module that uses the device.
#### Device 
Is either a physical or virtual hardware which will be communicated with by drivers.

## Registering a Char Device

#### alloc_chrdev_region
alloc_chrdev_region is used to register a major and minor numbers dynamically for the device.
``` int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);```

#### register_chrdev_region
register_chrdev_region is used to register predefined major and minor numbers.
``` int register_chrdev_region(dev_t first, unsigned int count,char *name);```

### MAJOR ,MINOR and MKDEV Macros
```
MAJOR(dev_t dev);// get major from dev_t
MINOR(dev_t dev);// get minor from dev_t
MKDEV(int major, int minor);//major/minor from dev_t
```

### /proc/devices
This folder contains all the devices defined on the system with their Major number.

### mknod command
__ mknod used to link the character device to the file system into the folder /dev/.
the option "c" is used to indicate that's a character device, and the major and minor numbers should be passed

### Minor and Major numbers
Each device has a unique major number, each device can have multiple minor numbers they will act as some instances of it.







