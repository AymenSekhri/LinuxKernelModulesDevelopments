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
#### CAP_SYS_RAWIO
The ability to perform “raw” I/O operations. Examples include accessing device
ports or communicating directly with USB devices.
#### CAP_SYS_ADMIN
A catch-all capability that provides access to many system administration operations.
#### CAP_SYS_TTY_CONFIG
The ability to perform tty configuration tasks.

## Sending Command Using write Instead of ioctl
Instead of using ioctl it is possible to use write to interpret the commands for the driver. This method is actually used by the console driver using escape characters to move curser, new line, tab ...

## Non-Blocking I/O
This is the unusual case and it's rarely used but should be implemented and executed when the flag _**O_NONBLOCK**_ is present in _**filp->f_flags**_. Only _read_,_write_ and _open_ may implement this behavior. If the device is available for read or write the driver should return _**-EAGAIN**_.

## Blocking I/O
If we want some I/O to block when the the requested data is not available we block the process and wake it up when some condition is met.<br> if a process blocks, some interrupt handlers will wake up the process every periodicity and let it check if the condition is met, if it's not, it will go to sleep again.<br>
There is a structure for the driver that holds the information about the asleep process which is _wait_queue_head_t_
```
wait_queue_head_t my_queue;
init_waitqueue_head(&my_queue);
```
To make a process sleep:
```
wait_event(queue, condition)
wait_event_interruptible(queue, condition)
wait_event_timeout(queue, condition, timeout)
wait_event_interruptible_timeout(queue, condition, timeout)
```
_wait_event_ is wait without an interruption from the user, the other one can be interrupted.
The last two is the same as the first two but with a timeout, the functions return 0 (success) if the timeout is passed.<br>
To wake up asleep processes we use:
```
void wake_up(wait_queue_head_t *queue);
void wake_up_interruptible(wait_queue_head_t *queue);
```
_wake_up_ is for _wait_event_ and _wake_up_interruptible_ is for _wait_event_interruptible_
<br>
Example:
```
static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;
ssize_t sleepy_read (struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) going to sleep\n",
	current->pid, current->comm);
	wait_event_interruptible(wq, flag != 0);
	flag = 0;
	printk(KERN_DEBUG "awoken %i (%s)\n", current->pid, current->comm);
	return 0; /* EOF */
}
ssize_t sleepy_write (struct file *filp, const char __user *buf, size_t count,loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) awakening the readers...\n",
	current->pid, current->comm);
	flag = 1;
	wake_up_interruptible(&wq);
	return count; /* succeed, to avoid retrial */
}
```
If a process tries to read when no process wrote before it will block, until another process does write something. There is a race condition in this code which is when a two processes are asleep and another process writes and wakes them both up, both will check the flag and both may pass that condition and executed. in this example it may not be important but in other situations waiting the event and setting the flag should have been an atomic operation (we may use spinlocks instead but we may not use semaphores because Linux might disabled  the interrupts at the condition check)//TODO: check if this is true.<br>
Note that is a process may want block on write if the buffer is full for example, you have use different wait queue variable. 

### Blocking I/O Example
```
static ssize_t scull_p_read (struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	struct scull_pipe *dev = filp->private_data;
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	while (dev->rp = = dev->wp) { /* nothing to read */
		up(&dev->sem); /* release the lock */
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		PDEBUG("\"%s\" reading: going to sleep\n", current->comm);
		if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		/* otherwise loop, but first reacquire the lock */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	/* ok, data is there, return something */
	if (dev->wp > dev->rp)
		count = min(count, (size_t)(dev->wp - dev->rp));
	else /* the write pointer has wrapped, return data up to dev->end */
		count = min(count, (size_t)(dev->end - dev->rp));
	if (copy_to_user(buf, dev->rp, count)) {
		up (&dev->sem);
		return -EFAULT;
	}
	dev->rp += count;
	if (dev->rp = = dev->end)
		dev->rp = dev->buffer; /* wrapped */
	up (&dev->sem);
	/* finally, awake any writers and return */
	wake_up_interruptible(&dev->outq);
	PDEBUG("\"%s\" did read %li bytes\n",current->comm, (long)count);
	return count;
}
```
When *wait_event_interruptible* is called and the process is waken up, we should start the race for the resource immediately by trying to hold the semaphore using *down_interruptible*.

### Internal Implementation of Sleep
```
if (!condition){
	set_current_state(TASK_INTERRUPTIBLE); //current->state = TASK_INTERRUPTIBLE;
	if (!condition)
		schedule(); //Give up the processor and reschedule.
	set_current_state(TASK_RUNNING); //current->state = TASK_RUNNING
}
```
A more error prone way is by using a higher level functions for sleeping and cleaning up.
```
wait_queue_t my_wait;
init_wait(&my_wait);
```
Initiates the sleep operation.
```
void prepare_to_wait(wait_queue_head_t *queue, wait_queue_t *wait, int state);
```
Change the state of the process, after this you should check you condition then call _schedule()_ .
```
void finish_wait(wait_queue_head_t *queue, wait_queue_t *wait);
```
Clean up after sleeping.

### Example of write with Low Level Sleep
The code is mostly like the read but in the sleep part we have :
```
static int scull_getwritespace(struct scull_pipe *dev, struct file *filp)
{
	while (spacefree(dev) == 0) { /* full */
		DEFINE_WAIT(wait);
		up(&dev->sem);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		PDEBUG("\"%s\" writing: going to sleep\n",current->comm);
		prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
		if (spacefree(dev) == 0)
			schedule( );
		finish_wait(&dev->outq, &wait);
		if (signal_pending(current))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	return 0;
}
```
The purpose of the second check `if (spacefree(dev) = = 0)` is that the buffer may has been freed between the moment we dropped the semaphore and the that check, if this check is not present of the buffer is freed after it, it is okay, since the process which will wake the writer up will reset the process state to _TASK_RUNNING_ and the _schedule_ will return but it will happen after we voluntarily gave up the processor. So without this check we may volunteer without a reason (actually there is no altruism in processes lol or in anything in that matter)

### Exclusive waking up
Sometimes we want to wake up one process at a time if a two conditions are met : you expect significant contention for a resource, and waking a single process is sufficient to completely consume the resource. We can do this by using the function:
```
void prepare_to_wait_exclusive(wait_queue_head_t *queue, wait_queue_t *wait, int state);
```
it sets the process to wait and woken up exclusively. But, all processes who used the normal sleep function  will be woken up all together and only ones who called _prepare_to_wait_exclusive_ will run exclusively.<br>
We can't use `wait_event` and its variants in this way.

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
