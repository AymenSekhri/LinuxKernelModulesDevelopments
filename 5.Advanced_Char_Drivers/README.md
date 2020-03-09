# Character Drivers
## *ioctl* System Call
This system call is used to configure the device, it definition in user mode is: <br>
```int ioctl(int fd, unsigned long cmd, ...);```<br>
Although it seems it has variable number of arguments but it doesn't, The number is hardcoded in the system call depending on the command type specified in second argument.

```
int (*ioctl) (struct inode *inode, struct file *filp,
unsigned int cmd, unsigned long arg); 
```