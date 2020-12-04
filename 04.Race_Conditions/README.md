# Race Conditions
A race condition happens when a process/thread tries to check a data while another process/thread is writing to it.
### Critical Region
Critical Region is where the race condition happens.
### Semaphore
Semaphore is an integer variable with functions; up and down. the down function checks if the semaphore's value is greater than zero, so it decreases it and continues the execution, otherwise it sends the process to sleep, the up function increases the value of the semaphore, and wakes up the sleeping processes/threads if there are any (semaphore value less than 0).<br>
* if the semaphore's value is zero or negative; the number indicates who many processes/threads are sleeping and waiting for the execution, and that no other processes/threads will execute if they enter the critical region.
* if the semaphore's value is positive; the number indicates who many processes/threads can enter the critical region directly without being told to sleep.
### Semaphore/Mutex Holding.
Semaphore/Mutex holding means that the process/thread can enter the critical region directly without being told to sleep.
### Mutex
Mutex is a semaphore but with a maximum value of 1, which means that only one process/thread can enter the critical region.

## Create and Use a Semaphore/Mutex
``` void sema_init(struct semaphore *sem, int val);```<br>
Initiates the semaphore.<br>
``` void init_MUTEX(struct semaphore *sem); ```<br>
Initiates the mutex.<br>
```void init_MUTEX_LOCKED(struct semaphore *sem); ```<br>
Initiates a locked mutex (its value is 0).<br>
``` void down(struct semaphore *sem); ```<br>
uninterruptible down function. This should not be used because it may make a process harder to kill.<br>
``` int down_interruptible(struct semaphore *sem); ```<br>
interruptable down function. which stops waiting if the KILL_SIG (or other signals) is sent from user mode, it returns non-zero integer at that case.<br>
``` int down_trylock(struct semaphore *sem); ```<br>
down function but does not sleep if it can't hold the semaphore/mutex.<br>
``` void up(struct semaphore *sem); ```<br>
up function (releases the semaphore/mutex).<br>

## Reader/Writer Semaphore
``` void init_rwsem(struct rw_semaphore *sem); ```<br>
``` void down_read(struct rw_semaphore *sem); ```<br>
``` int down_read_trylock(struct rw_semaphore *sem); ```<br>
``` void up_read(struct rw_semaphore *sem); ```<br>
``` void down_write(struct rw_semaphore *sem); ```<br>
``` int down_write_trylock(struct rw_semaphore *sem); ```<br>
``` void up_write(struct rw_semaphore *sem); ```<br>
``` void downgrade_write(struct rw_semaphore *sem); ```<br>
Reader/Writer Semaphore is the same as the normal semaphore except it allows either one writer to hold the semaphore or unlimited readers. This is rarely used, but it might be useful.

## Completion
in case where a thread class another thread and waits unit completes a task to continue like the following example:<br>
``` 
struct semaphore sem;
init_MUTEX_LOCKED(&sem);
start_external_task(&sem);
down(&sem); 
```
The semaphore in this case will have a race condition due the internal implementation of the semaphore in Linux which happens when the the waiting thread is woke up  and going to deallocate the semaphore object, before the other thread finishes up function. Plus, the semaphores are not optimized for this case, and we better use completion for it.<br>

``` init_completion(&my_completion); ```<br>
Initiates the completion.<br>
``` void wait_for_completion(struct completion *c); ```<br>
Wait until the "completion semaphore" is released.<br>
``` void complete(struct completion *c); ```<br>
Wake up one waiting thread.<br>
``` void complete_all(struct completion *c); ```<br>
Wake up all waiting threads.<br>

## Spinlocks
### Preemptive Kernel
Preemptive Kernel is a kernel where the scheduler is active even for drivers, most modern operating systems use Preemptive kernels<br>
### Non-Preemptive Kernel
Non-Preemptive Kernel is a kernel where the scheduler does not switch to other process until the process exits its kernel mode or voluntarily gives up the CPU<br>
### Spinlocks
Unlike the semaphore it does not make the calling thread sleep, it keeps checking for the release of lock in loop (busy waiting). Another difference is that Spinlock is just a binary value (locked or unlocked).<br>
When the waiting for semaphore is a small period, semaphore may be not the best for performance. and spinlock is more suitable <br>
Spinlocks are intended to be used in Multi-Processor or Preemptive systems. because if Uni-Processor and Non-Preemptive then the CPU will spin forever if the spinlock is locked. same case for interrupt handlers (even for MultiProcessor systems or preemptive kernel).<br>
``` void spin_lock_init(spinlock_t *lock); ```<br>
Initiates the lock.<br>
``` void spin_lock(spinlock_t *lock); ```<br>
``` void spin_unlock(spinlock_t *lock); ```<br>
lock and unlock normal spinlock.<br>
``` void spin_lock_irqsave(spinlock_t *lock, unsigned long flags); ```<br>
``` void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags); ```<br>
lock and unlock a spinlock that disables the interrupts and return the interrupt flags to the user.<br>
``` void spin_lock_irq(spinlock_t *lock); ```<br>
``` void spin_unlock_irq(spinlock_t *lock); ```<br>
lock and unlock a spinlock that disables the interrupts.<br>
``` void spin_lock_bh(spinlock_t *lock) ```<br>
``` void spin_unlock_bh(spinlock_t *lock); ```<br>
lock and unlock a spinlock that disables software interrupts and enables hardware interrupts.<br>
``` int spin_trylock(spinlock_t *lock); ```<br>
``` int spin_trylock_bh(spinlock_t *lock); ```<br>
Checking the spinlock without spinning.<br>
### Reader/Writer Spinlock
Same concept of reader/writer semaphore but for spinlock; one writer and unlimited readers.
```
void rwlock_init(rwlock_t *my_rwlock); 
void read_lock(rwlock_t *lock);
void read_lock_irqsave(rwlock_t *lock, unsigned long flags);
void read_lock_irq(rwlock_t *lock);
void read_lock_bh(rwlock_t *lock);
void read_unlock(rwlock_t *lock);
void read_unlock_irqrestore(rwlock_t *lock, unsigned long flags);
void read_unlock_irq(rwlock_t *lock);
void read_unlock_bh(rwlock_t *lock);
void write_lock(rwlock_t *lock);
void write_lock_irqsave(rwlock_t *lock, unsigned long flags);
void write_lock_irq(rwlock_t *lock);
void write_lock_bh(rwlock_t *lock);
int write_trylock(rwlock_t *lock);
void write_unlock(rwlock_t *lock);
void write_unlock_irqrestore(rwlock_t *lock, unsigned long flags);
void write_unlock_irq(rwlock_t *lock);
void write_unlock_bh(rwlock_t *lock);
```

### Spinlock Rules
* When a spinlock is held, the kernel disables preemption (thread will not be rescheduled).
* In code that holds the spinlock, no function that may cause sleep should be present (Atomic code), because if it is, then thread will not be rescheduled.
* If there is a code that has a spinlock and there is an interrupt handler that needs to hold a spinlock, you must use spinlock functions that disables the interrupts.

## Semaphore/Mutex/Spinlock Rules
* If a function that requires a lock calls another function that requires the same lock, the thread will keep spinning forever, You should never do this.
* If a code needs to hold two locks then all codes must acquire the locks in the same order, otherwise one thread may hold lock1 and seeks for lock2, while another thread holds a lock2 and seeks for lock1 and they both be hanging for ever.
* In device drivers you should use lock for the whole driver and optimize to smaller chunks of code only if you have reason to, use the tool *lock-meter* to measure the time spent on each lock.

## Locking Alternatives
### Lock-Free Algorithms
In the case of reader/writer if there is only one reader and one writer then it can be represented using a circular buffer with two indexes one for next data to read and one for the next data to write, if the write/read operation happen before updating the index then this algorithm does not require a lock.

### Atomic Variables
If you need a lock for a simple variable then using semaphore/spinlock is overkill and not good for performance, thus making this variable atomic is a better solution.<br>
Atomic code means that code runs as whole with absolutely no interruption, not from scheduler nor from the interrupts nor from CPU pipelining, some CPUs uses a dedicated instructions for such thing.<br>
atomic_t DOES NOT accept full integer value but only 24bit.<br>
```
void atomic_set(atomic_t *v, int i);
int atomic_read(atomic_t *v);
```
Write and read to/from the variable.<br>

```
void atomic_add(int i, atomic_t *v);
void atomic_sub(int i, atomic_t *v);
void atomic_inc(atomic_t *v);
void atomic_dec(atomic_t *v);
```
As the names tell !!<br>
```
int atomic_inc_and_test(atomic_t *v);
int atomic_dec_and_test(atomic_t *v);
int atomic_sub_and_test(int i, atomic_t *v);
```
Do the operation and test if result it zero.<br>
```
int atomic_add_negative(int i, atomic_t *v);
```
Do the operation and test if result it negative.<br>
```
int atomic_add_return(int i, atomic_t *v);
int atomic_sub_return(int i, atomic_t *v);
int atomic_inc_return(atomic_t *v);
int atomic_dec_return(atomic_t *v);
```
Do the operation and return the result.<br>


#### Note
for example: <br>
```
atomic_sub(amount, &first_atomic);
atomic_add(amount, &second_atomic);
```
If it's not safe to run some code between the two lines then you should consider doing locking rather than atomic variables because each line is atomic but not both.

### Atomic Bit Operations
same as the atomic variable but with bit operations.<br>
```
void set_bit(nr, void *addr);
void clear_bit(nr, void *addr);
void change_bit(nr, void *addr);
test_bit(nr, void *addr);
int test_and_set_bit(nr, void *addr);
int test_and_clear_bit(nr, void *addr);
int test_and_change_bit(nr, void *addr);
```
<br>

### Seqlocks
Seqlocks work in situations where the resource to be protected is small, simple, and frequently accessed, and where write access is rare but must be fast. Essentially, hey work by allowing readers free access to the resource but requiring those readers to checkfor collisions with writers and, when such a collision happens, retry their access.<br>

```
seqlock_t lock2;
seqlock_init(&lock2);
//The Reader
unsigned int seq;
do {
seq = read_seqbegin(&the_lock);
/* Read Your Stuff Here */
} while read_seqretry(&the_lock, seq);
// The Writer
void write_seqlock(seqlock_t *lock);
/* Write Your Stuff Here */
void write_sequnlock(seqlock_t *lock);
```

```
unsigned int read_seqbegin_irqsave(seqlock_t *lock,
unsigned long flags);
int read_seqretry_irqrestore(seqlock_t *lock, unsigned int seq,
unsigned long flags);
```
These function are used when the lock is accessed in the interrupt handlers too (they disable interrupts same as spinlocks).<br>
```
void write_seqlock_irqsave(seqlock_t *lock, unsigned long flags);
void write_seqlock_irq(seqlock_t *lock);
void write_seqlock_bh(seqlock_t *lock);
void write_sequnlock_irqrestore(seqlock_t *lock, unsigned long flags);
void write_sequnlock_irq(seqlock_t *lock);
void write_sequnlock_bh(seqlock_t *lock);
```

## Summery
### Semaphore
* Used in uniprocessor or when the waiting is long.
* CAN NOT used in interrupt handlers.
* You should (not necessary) drop the semaphore before going to sleep voluntarily.
### Spinlocks
* More efficient in the multiprocessor or when the waiting is short.
* Can be used in interrupt handlers but only in multiprocessor.
* You should (not necessary) drop the spinlock before going to sleep voluntarily.
* More efficient when the lock is hold for very short time and doesn't sleep inside (if it sleeps then it will take long time)

## NOTE
Consider the code:
```
if (down_interruptible(&dev->sem))
    return -ERESTARTSYS;
if (dev->rp = = dev->wp) { 
    /* do something */
}
up(&dev->sem); /* release the lock */
```
It may seem useless to use semaphore or any lock for a simple check like `dev->rp = = dev->wp` which is may executed atomically, but actually it's not atomic code, that line requires getting data from stack for the two values then compare them which is at least two instructions and the preemption may happen between them.
