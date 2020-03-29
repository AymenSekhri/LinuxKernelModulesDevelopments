# Timers And Delays
## jiffies
### what is jiffies ?
jiffies is a counter that incremented every timer interrupt, which will happen depending on kernel configuration and the architecture (1000 ticks per second for x86 processors). number of ticks can be increased to get more precise asynchronous events but it will does more CPU overhead, when this value is changed in kernel source, the whole kernel and all the modules must be recompiled.<br>
The constant _**HZ**_ is used to represent a second in terms of jiffies:
```
#include <linux/jiffies.h>
unsigned long j, stamp_1, stamp_half, stamp_n;
j = jiffies; /* read the current value */
stamp_1 = j + HZ; /* 1 second in the future */
stamp_half = j + HZ/2; /* half a second */
stamp_n = j + n * HZ / 1000; /* n milliseconds */
```
Some operation on jiffies:
```
#include <linux/jiffies.h>
int time_after(unsigned long a, unsigned long b);
int time_before(unsigned long a, unsigned long b);
int time_after_eq(unsigned long a, unsigned long b);
int time_before_eq(unsigned long a, unsigned long b);
```
To convert jiffies to user mode structures:
```
#include <linux/time.h>
unsigned long timespec_to_jiffies(struct timespec *value);
void jiffies_to_timespec(unsigned long jiffies, struct timespec *value);
unsigned long timeval_to_jiffies(struct timeval *value);
void jiffies_to_timeval(unsigned long jiffies, struct timeval *value);
```

### jiffies vs jiffies_64
_jiffies_64_ and jiffies are both the same in 64bit architectures, but in 32bit architecture _jiffies_64_ is 64bit value but jiffies is the lower half of the 64bit version, and it wraps every ~50 days.<br>
To get _jiffies_64_ 
```
#include <linux/jiffies.h>
#include <linux/types.h>
u64 get_jiffies_64(void);
```
## Processor-Specific Registers
Some architectures have a a register that incremented each clock cycle which are more precise than jiffies but also dependent on the CPU's frequency.<br>
In 86x processors there is 64bit register named rdtsc which is used as a counter and can be accessed by:
```
#include <<asm/msr.h> //Machine Specific Registers
rdtsc(low32,high32);
rdtscl(low32);
rdtscll(var64);
```
To access the counter register independent from the architecture we may use:
```
#include <linux/timex.h>
cycles_t get_cycles(void);
```
This function returns 0 always if the target architecture doesn't have such registers.
## Current Date/Time
It is not recommended to deal with real world time in the kernel, it should be left for the user space. but there are some functions for it anyway.
```
#include <linux/time.h>
unsigned long mktime (unsigned int year, unsigned int mon,
unsigned int day, unsigned int hour,
unsigned int min, unsigned int sec);
```
This function converts wallclock time into a jiffies value.
```
#include <linux/time.h>
void do_gettimeofday(struct timeval *tv);
```
fill timeval structure.

## Delays
### Busy-Waiting
Most naive way, just simple loop that checks time.
```
while (time_before(jiffies, j1))
cpu_relax( );
```
Cons:
* it hangs the whole system when the CPU is uniprocessor and not preemptive or interrupts are disabled.
* When a heavy process is running, gets precise delays for non-preemptive systems but not for preemptive systems.

### Yielding the Processor
Instead of preserve the CPU doing nothing we tell it to reschedule other process.
```
while (time_before(jiffies, j1))
    schedule( );
```
Cons:
* Not precise delays when the system is busy, since there is no guarantees that the system will schedule our process before the delay expires. 
* The idle task will never be scheduled since our process will still be in the scheduler queue anyway.

### Events Timouts
is the best method which uses the normal `wait_event_interruptible_timeout` function.
```
#include <linux/wait.h>
long wait_event_timeout(wait_queue_head_t q, condition, long timeout);
long wait_event_interruptible_timeout(wait_queue_head_t q, condition, long timeout);
```
Example:
```
wait_queue_head_t wait;
init_waitqueue_head (&wait);
wait_event_interruptible_timeout(wait, 0, delay);
```
since the prcoess will be be out of scheduler queue by wait_event_interruptible_timeout function, then the caller process will go to sleep and the scheduler may run idle task if possible. And the same result whether the system is preemptive or not.<br>
More error-prone way is using `schedule_timeout` since `wait_event_interruptible_timeout` relies on `schedule_timeout`
internally
```
#include <linux/sched.h>
signed long schedule_timeout(signed long timeout);
```
Example:
```
set_current_state(TASK_INTERRUPTIBLE);
schedule_timeout (delay);
```
### Short Delays
These are architecture specific functions and uses busy-waiting.
```
#include <linux/delay.h>
void ndelay(unsigned long nsecs);
void udelay(unsigned long usecs);
void mdelay(unsigned long msecs);
```
if you want tolerate more delays than expected you can use:
```
void msleep(unsigned int millisecs);
unsigned long msleep_interruptible(unsigned int millisecs);
void ssleep(unsigned int seconds)
```
## Kernel Timers
Timer are used to execute a function after sometime asynchronously without blocking the current thread, When executed the caller user mode process may not be in memory nor be running at all, so you can't access user memory nor `current` global variable. The timer function is software interrupt handler so it should be atomic and you cant use any function that sleeps nor semaphores...
To when you are in interrupt handler use `in_interrupt()` which returns nonzero if the processor is currently
running in interrupt context, either hardware interrupt or software interrupt.<br>
To know if you can schedule or sleep use `in_atomic()` which similar to `in_interrupt()`, it can be useful to know if you are in interrupt or you are in a code protected using a spinlock.<br>
It’s also worth knowing that in an SMP system, the timer function is executed by thesame CPU that registered it, to achieve better cache locality whenever possible.Therefore, a timer that reregisters itself always runs on the same CPU.<br>

```
#include <linux/timer.h>
struct timer_list {
    /* ... */
    unsigned long expires;
    void (*function)(unsigned long);
    unsigned long data;
};
void init_timer(struct timer_list *timer);
struct timer_list TIMER_INITIALIZER(_function, _expires, _data);
void add_timer(struct timer_list * timer);
int del_timer(struct timer_list * timer);
```
some other APIs
```
int mod_timer(struct timer_list *timer, unsigned long expires);
int del_timer_sync(struct timer_list *timer);
int timer_pending(const struct timer_list * timer);
```
## Tasklets

Tasklets are almost same timers, however, you can’t ask to execute the function at a specific time. By scheduling a tasklet, you simply ask for it to be executed a later time chosen by the kernel. This behavior is especially useful with interrupt handlers, where the hardware interrupt must be managed as quickly as possible, but most of the data management can be safely delayed to a later time.
```
#include <linux/interrupt.h>
struct tasklet_struct {
    /* ... */
    void (*func)(unsigned long);
    unsigned long data;
};
void tasklet_init(struct tasklet_struct *t,
void (*func)(unsigned long), unsigned long data);
DECLARE_TASKLET(name, func, data);
DECLARE_TASKLET_DISABLED(name, func, data);
```
A tasklets can be concurrent with other tasklets but is strictly serialized with respect to itself—the same tasklet never runs simultaneously on more than one processor. Also, as already noted, a tasklet always runs on the same CPU that schedules it.
som other APIs:
```
void tasklet_disable(struct tasklet_struct *t);
void tasklet_disable_nosync(struct tasklet_struct *t);
void tasklet_enable(struct tasklet_struct *t);
void tasklet_schedule(struct tasklet_struct *t);
void tasklet_hi_schedule(struct tasklet_struct *t);
void tasklet_kill(struct tasklet_struct *t);

```
### Note
Tasklets are atomic which means that you can't sleep there because they don't belong to any process so they can't be rescheduled, though, interrupt are not disabled in the tasklet context.
## Workqueues
Workqueues are same as tasklets except they are not atomic so they can sleep because they are executed in a special kernel process, but still it can't access user mode memory.
APIs to create a work queue
```
struct workqueue_struct *create_workqueue(const char *name);
struct workqueue_struct *create_singlethread_workqueue(const char *name);
```
If you use create_workqueue, you get a workqueue that has a dedicated thread for each processor on the system.<br>
APIs to intilize the work
```
DECLARE_WORK(name, void (*function)(void *), void *data); //Static
INIT_WORK(struct work_struct *work, void (*function)(void *), void *data);
PREPARE_WORK(struct work_struct *work, void (*function)(void *), void *data);
```
There are two functions for submitting work to a workqueue:
```
int queue_work(struct workqueue_struct *queue, struct work_struct *work);
int queue_delayed_work(struct workqueue_struct *queue, struct work_struct *work, unsigned long delay);
int cancel_delayed_work(struct work_struct *work);
void flush_workqueue(struct workqueue_struct *queue);
```
You can use a shared workqueue provided by the kernel:
```
INIT_WORK(&jiq_work, jiq_print_wq, &jiq_data);
int schedule_work(struct work_struct *work);
```