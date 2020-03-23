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

### jiffies vs ## jiffies_64
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