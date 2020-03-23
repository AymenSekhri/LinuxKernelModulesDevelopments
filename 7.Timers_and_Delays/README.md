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