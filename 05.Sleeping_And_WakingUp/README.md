# Sleeping And Waking Up
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
