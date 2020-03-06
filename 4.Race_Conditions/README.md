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
in case where a thread class another thread and waits unit completes a task to continue like the following example:
``` 
struct semaphore sem;
init_MUTEX_LOCKED(&sem);
start_external_task(&sem);
down(&sem); 
```
The semaphore in this case will have a race condition due the internal implementation of the semaphore in Linux which happens when the the waiting thread is woke up  and going to deallocate the semaphore object, before the other thread finishes up function. Plus, the semaphores are not optimized for this case, and we better use completion for it.

``` init_completion(&my_completion); ```<br>
Initiates the completion.<br>
``` void wait_for_completion(struct completion *c); ```<br>
Wait until the "completion semaphore" is released.<br>
``` void complete(struct completion *c); ```<br>
Wake up one waiting thread.<br>
``` void complete_all(struct completion *c); ```<br>
Wake up all waiting threads.<br>





