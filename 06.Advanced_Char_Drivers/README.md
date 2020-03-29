# Advanced Character Drivers
## *ioctl* System Call
This system call is used to configure the device, it definition in user mode is: <br>
```int ioctl(int fd, unsigned long cmd, ...);```<br>
Although it seems it has variable number of arguments but it doesn't, The number is hardcoded in the system call depending on the command type specified in second argument.<br>
Here is the function definition is the kernel:

```
int (*ioctl) (struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg); 
```
The _cmd_ argument represent the command which _ioctl_ will do, In order to not send a right command to a wrong device, this number "should" (doesn't have to) be unique for all commands in the system and it is structured like the following, although it could be ignored completly since the kernel doesn't check it<br>
* **Type**: Should be unique for each device and the same for all commands of a one device, consult _ioctl-number.txt_ to get a number that has not been used by other system drivers. _type_'s size is __IOC_TYPEBITS_ (8 bits in 5.04 Kernel x86)
* **number**: The ordinal (sequential) number used to differentiate the commands, its size is __IOC_NRBITS_ (8 bits in 5.04 Kernel x86).
* **direction**: The direction of data transfer; so it's either __IOC_NONE_, __IOC_READ_ or __IOC_WRITE_, you can combine the last two by OR operation.
* **size**: The size of the cmd arguments, which is in __IOC_SIZEBITS_ macro.
<br><br>
There are few macros to setup the cmd command:
* _**_IO(type,nr)**_: to setup cmd command without arguments.
* _**_IOR(type,nr,datatype)**_: to setup cmd command to read.
* _**_IOW(type,nr,datatype)**_: to setup cmd command to write.
* _**_IOWR(type,nr,datatype)**_: to setup cmd command to write and.
And there is some macros to decode it: _**_IOC_DIR(nr)**_, _**_IOC_TYPE(nr)**_, _**_IOC_NR(nr)**_, and _**_IOC_SIZE(nr)**_.<br><br>
The return value, can be either _**-EINVAL**_ (Invalid argument) or _**-ENOTTY**_ (Inappropriate ioctl for a Device), the later is what POSIX standard supports.

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
It returns _**true**_ when the given permission is set.<br>
To use it:
```
if (!capable (CAP_SYS_ADMIN))
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
#### CAP_SYS_RAWIO
The ability to perform “raw” I/O operations. Examples include accessing device
ports or communicating directly with USB devices.
#### CAP_SYS_ADMIN
A catch-all capability that provides access to many system administration operations.
#### CAP_SYS_TTY_CONFIG
The ability to perform tty configuration tasks.

## Sending Command Using write Instead of ioctl
Instead of using ioctl it is possible to use write to interpret the commands for the driver. This method is actually used by the console driver using escape characters to move curser, new line, tab ...


## _poll_ System Call
```
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```
_**poll**_ request is used to fetch if some read/write is available for multiple file descriptors which is returned in a bitmask. The driver function of poll is the following:
```
unsigned int (*poll) (struct file *filp, poll_table *wait);
```
The driver should call _**poll_wait**_ and then sets up the bitmask and return it.
```
void poll_wait (struct file *, wait_queue_head_t *, poll_table *);
```
Then the kernel will check the returned value, if it is zero, the kernel will sets the process to sleep until someone wakes it up using the wait_queue given, then it will check again and return to user mode if the bitmask is not zero.<br>
If the driver doesn't call poll_wait the bitmask wont be checked by the kernel and will returned to the user without blocking even if it is zero.<br>
### bitmask flags
#### POLLIN
This bit must be set if the device can be read without blocking.
#### POLLRDNORM
This bit must be set if “normal” data is available for reading. A readable device
returns (POLLIN | POLLRDNORM).
#### POLLRDBAND
This bit indicates that out-of-band data is available for reading from the device.
It is currently used only in one place in the Linux kernel (the DECnet code) and
is not generally applicable to device drivers.
#### POLLPRI
High-priority data (out-of-band) can be read without blocking. This bit causes
select to report that an exception condition occurred on the file, because select
reports out-of-band data as an exception condition.
#### POLLHUP
When a process reading this device sees end-of-file, the driver must set POLLHUP
(hang-up). A process calling select is told that the device is readable, as dictated
by the select functionality.
#### POLLERR
An error condition has occurred on the device. When poll is invoked, the device
is reported as both readable and writable, since both read and write return an
error code without blocking.
#### POLLOUT
This bit is set in the return value if the device can be written to without blocking.
#### POLLWRNORM
This bit has the same meaning as POLLOUT, and sometimes it actually is the same
number. A writable device returns (POLLOUT | POLLWRNORM).
#### POLLWRBAND
Like POLLRDBAND, this bit means that data with nonzero priority can be written to
the device. Only the datagram implementation of poll uses this bit, since a datagram
can transmit out-of-band data.

### poll Example
```
static unsigned int scull_p_poll(struct file *filp, poll_table *wait)
{
	struct scull_pipe *dev = filp->private_data;
	unsigned int mask = 0;
	/*
	* The buffer is circular; it is considered full
	* if "wp" is right behind "rp" and empty if the
	* two are equal.
	*/
	down(&dev->sem);
	poll_wait(filp, &dev->inq, wait);
	poll_wait(filp, &dev->outq, wait);
	if (dev->rp != dev->wp)
		mask |= POLLIN | POLLRDNORM; /* readable */
	if (spacefree(dev))
		mask |= POLLOUT | POLLWRNORM; /* writable */
	up(&dev->sem);
	return mask;
}
```
## fsync System Call
_write_ system call should never block, if the device is ready to be written to it applications use poll to check it, if the application wants to insure that the written data was successfully reached the device, it should use _fsync_.

## Asynchronous Notification
To setup asynchronous notification we need to set it up in user application and driver.<br>
### User Application:
* Use _signal_ system call to tell the kernel about our handler's address for _SIGIO_ signals.
* Send _F_SETOWN_ command using the _fcntl_ to save the process ID in file descriptor in ilp->f_owner.
* Set _FASYNC_ flag in the device using _fcntl_ with _F_SETFL_ command.
#### Code
```
signal(SIGIO, &input_handler); /* dummy sample; sigaction( ) is better */
fcntl(STDIN_FILENO, F_SETOWN, getpid( ));
oflags = fcntl(STDIN_FILENO, F_GETFL);
fcntl(STDIN_FILENO, F_SETFL, oflags | FASYNC);
```
### Driver:
* When _F_SETFL_ is executed to turn on _FASYNC_, the driver’s _fasync_ method is called, Which is used to setup the asynchronous notification.
* When Data is received the driver should notify the process using _SIGIO_ signal.
We use the two functions:
```
int fasync_helper(int fd, struct file *filp, int mode, struct fasync_struct **fa);
void kill_fasync(struct fasync_struct **fa, int sig, int band);
```
in fasync entry:
```
static int scull_p_fasync(int fd, struct file *filp, int mode)
{
	struct scull_pipe *dev = filp->private_data;
	return fasync_helper(fd, filp, mode, &dev->async_queue);
}
```
when the data is available (in write) the drive should call:
```
if (dev->async_queue)
	kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
```


## Control The Device Access to One User


### Code in _open_ Entry
```
spin_lock(&scull_u_lock);
if (scull_u_count &&
	(scull_u_owner != current->uid) && /* allow user */
	(scull_u_owner != current->euid) && /* allow whoever did su */
	!capable(CAP_DAC_OVERRIDE)) { /* still allow root */
		spin_unlock(&scull_u_lock);
		return -EBUSY; /* -EPERM would confuse the user */
}
if (scull_u_count = = 0)
	scull_u_owner = current->uid; /* grab it */
scull_u_count++;
spin_unlock(&scull_u_lock);
```
### Code in _release_ Entry
```
static int scull_u_release(struct inode *inode, struct file *filp)
{
	spin_lock(&scull_u_lock);
	scull_u_count--; /* nothing else */
	spin_unlock(&scull_u_lock);
	return 0;
}
```

### Code for Blocking Instead of _EBUSY_
```
spin_lock(&scull_w_lock);
while (! scull_w_available( )) {
	spin_unlock(&scull_w_lock);
	if (filp->f_flags & O_NONBLOCK)
		return -EAGAIN;
	if (wait_event_interruptible (scull_w_wait, scull_w_available()))
		return -ERESTARTSYS; /* tell the fs layer to handle it */
	spin_lock(&scull_w_lock);
}
if (scull_w_count = = 0)
	scull_w_owner = current->uid; /* grab it */
scull_w_count++;
spin_unlock(&scull_w_lock);
```