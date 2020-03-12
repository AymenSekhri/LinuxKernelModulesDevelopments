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
