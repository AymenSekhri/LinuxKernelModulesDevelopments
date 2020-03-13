# Character Drivers
## *ioctl* System Call
This system call is used to configure the device, it definition in user mode is: <br>
```int ioctl(int fd, unsigned long cmd, ...);```<br>
Although it seems it has variable number of arguments but it doesn't, The number is hardcoded in the system call depending on the command type specified in second argument.<br>
Here is the function definition is the kernel:

```
int (*ioctl) (struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg); 
```
The _cmd_ argument represent the command which _ioctl_ will do, In order to not send a right command to a wrong device, this number should be unique for all commands in the system and it is structured like the following:<br>
* **Type**: Should be unique for each device and the same for all commands of a one device, consult _ioctl-number.txt_ to get a number that has not been used by other system drivers. _type_'s size is __IOC_TYPEBITS_ (8 bits in 5.04 Kernel x86)
* **number**: The ordinal (sequential) number, its size is __IOC_NRBITS_ (8 bits in 5.04 Kernel x86).
* **direction**: The direction of data transfer; so it's either __IOC_NONE_, __IOC_READ_ or __IOC_WRITE_, you can combine the last two by OR operation.
* **size**: The size of the cmd arguments, which is in __IOC_SIZEBITS_ macro.
<br><br>
There are few macros to setup the cmd command:
* _**_IO(type,nr)**_: to setup cmd command without arguments.
* _**_IOR(type,nr,datatype)**_: to setup cmd command to read.
* _**_IOW(type,nr,datatype)**_: to setup cmd command to write.
* _**_IOWR(type,nr,datatype)**_: to setup cmd command to write and.
And there is some macros to decode it: _**_IOC_DIR(nr)**_, _**_IOC_TYPE(nr)**_, _**_IOC_NR(nr)**_, and _**_IOC_SIZE(nr)**_.<br>

### Predefined Commands
These commands are defined by the kernel and if you define them for your driver, the driver's function won't be called at all. Here is some:
#### FIOCLEX
Set the close-on-exec flag (File IOctl CLose on EXec). Setting this flag causes the file descriptor to be closed when the calling process executes a new program.
#### FIONCLEX
Clear the close-on-exec flag (File IOctl Not CLos on EXec). The command
restores the common file behavior, undoing what FIOCLEX above does.
#### FIOQSIZE
This command returns the size of a file or directory; when applied to a device
file, however, it yields an ENOTTY error return.

## Accessing User Space Memory From the Kernel
When your driver is executing, the user space process which called the driver is not necessary is still in memory, so direct access to user space pointer may cause some problems (page fault, data from other process, not user data at all). Thus, there are some function used to bring some of the user mode process memory.<br>
The kernel knows which process that called the driver so you don't have to specify it in the following functions.
### *copy_from_user*/*copy_to_user*
```
unsigned long copy_to_user (void __user* to, const void* from, unsigned long	n);
unsigned long copy_from_user (void* to, const void __user* from,unsigned long n);
```
As has been explained before.
### *access_ok*
```int access_ok(int type, const void *addr, unsigned long size);```<br>
This function checks if the user pointer is indeed in user space not the kernel space.<br>
Unlike most kernel functions, access_ok returns a boolean value: 1 for success (access
is OK) and 0 for failure (access is not OK). If it returns false, the driver should usually
return -EFAULT to the caller.<br>
As of a commit in 2019, access_ok() no long has the type argument, so the VERIFY_WRITE versus VERIFY_READ point is moot.
### *__put_user*/*__get_user* 
```
__put_user(datum, ptr)
__get_user(local, ptr)
```
These two function are used to write/read from the user mode pointer *ptr* to the local variable *datum*/*local*, the size of data to be transferred is determined by the size of the datatype pointer by the pointer *ptr*. These function are fast and do less checks so you need to check if the pointer is user pointer by *access_ok* function.<br>
Note that these function accept data types of the size 2/4/8, otherwise it will generate a compile time error “conversion to non-scalar type requested.”.

## Capabilities and Restricted Operations
There predefined capabilities or permission in the kernel for the drivers to restrict some operations for some user mode processes. To check if the calling process has a certain permission we use the following function:
```
int capable(int capability);
```
To use it:
```
if (! capable (CAP_SYS_ADMIN))
	return -EPERM;
```
And here is some of the permissions defined in *<linux/sched.h>* :
#### CAP_DAC_OVERRIDE
The ability to override access restrictions (data access control, or DAC) on files
and directories.
#### CAP_NET_ADMIN
The ability to perform network administration tasks, including those that affect
network interfaces.
#### CAP_SYS_MODULE
The ability to load or remove kernel modules.
CAP_SYS_RAWIO
#### The ability to perform “raw” I/O operations. Examples include accessing device
ports or communicating directly with USB devices.
#### CAP_SYS_ADMIN
A catch-all capability that provides access to many system administration operations.
#### CAP_SYS_TTY_CONFIG
The ability to perform tty configuration tasks.
