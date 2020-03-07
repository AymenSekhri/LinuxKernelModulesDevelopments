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
* When a spinlock is hold, the kernel disable preemption (thread will be rescheduled).
* In code that holds the spinlock, no function that may cause sleep should be present (Atomic code), because if it is, then thread will be rescheduled.
* If there is a code that has a spinlock and there is an interrupt handler that needs to hold a spinlock, you must use spinlock functions that disables the interrupts.










