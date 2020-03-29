# Interrupts Handling
## Registering an interrupt Handler
Interrupt handler registeration should be in _open_ system call not at module loading because the you might reserve the interrupt like and not use it for anything (since an interrupt line once it is registered it won't be shared only if both drivers agree), the interrupt handler should be unregistered at _close_ system call. The correct place to call request_irq is when the device is first opened, before the hardware is instructed to generate interrupts. The place to call free_irq is the last  time the device is closed, after the hardware is told not to interrupt the processor any more. Interrupts are reported in `/proc/interrupts` and some statistics are reported in `/proc/stat`.
```
int request_irq(unsigned int irq, 
                irqreturn_t (*handler)(int, void *, struct pt_regs *),
                unsigned long flags ,const char *dev_name,
                void *dev_id);
void free_irq(unsigned int irq, void *dev_id);
// Only for i386 and x86_64, note that it is not atomic with request_irq.
int can_request_irq(unsigned int irq, unsigned long flags);
```
The bits that can be set in flags are as follows:
* **SA_INTERRUPT**<br>
When set, this indicates a “fast” interrupt handler. Fast handlers are executed with interrupts disabled on the current processor (the topic is covered in the section
“Fast and Slow Handlers”).
* **SA_SHIRQ**<br>
This bit signals that the interrupt can be shared between devices. The concept of sharing is outlined in the section “Interrupt Sharing.”
* **SA_SAMPLE_RANDOM**<br>
This bit indicates that the generated interrupts can contribute to the entropy pool used by _/dev/random_ and _/dev/urandom_. 

## Interrupt Probing
You can initiate the interrupt handler by the default interrupt numbers given by the hardware, but this might be changed so we need a way to get the interrupt used by the device.
### Using kernel functions

```
unsigned long probe_irq_on(void);
int probe_irq_off(unsigned long);
```
Example:
```
int count = 0;
do {
    unsigned long mask;
    mask = probe_irq_on( );
    outb_p(0x10,short_base+2); /* enable reporting */
    outb_p(0x00,short_base); /* clear the bit */
    outb_p(0xFF,short_base); /* set the bit: interrupt! */
    outb_p(0x00,short_base+2); /* disable reporting */
    udelay(5); /* give it some time */
    short_irq = probe_irq_off(mask);
    if (short_irq = = 0) { /* none of them? */
        printk(KERN_INFO "short: no irq reported by probe\n");
        short_irq = -1;
    }
    /*
    * if more than one line has been activated, the result is
    * negative. We should service the interrupt (no need for lpt port)
    * and loop over again. Loop at most five times, then give up
    */
} while (short_irq < 0 && count++ < 5);
if (short_irq < 0)
    printk("short: probe failed %i times, giving up\n", count);
```

### Bare Metal Way

```
int trials[ ] = {3, 5, 7, 9, 0};
int tried[ ] = {0, 0, 0, 0, 0};
int i, count = 0;
/*
* install the probing handler for all possible lines. Remember
* the result (0 for success, or -EBUSY) in order to only free
* what has been acquired
*/
for (i = 0; trials[i]; i++)
tried[i] = request_irq(trials[i], short_probing,
SA_INTERRUPT, "short probe", NULL);
do {
    short_irq = 0; /* none got, yet */
    outb_p(0x10,short_base+2); /* enable */
    outb_p(0x00,short_base);
    outb_p(0xFF,short_base); /* toggle the bit */
    outb_p(0x00,short_base+2); /* disable */
    udelay(5); /* give it some time */
    /* the value has been set by the handler */
    if (short_irq = = 0) { /* none of them? */
        printk(KERN_INFO "short: no irq reported by probe\n");
    }
    /*
    * If more than one line has been activated, the result is
    * negative. We should service the interrupt (but the lpt port
    * doesn't need it) and loop over again. Do it at most 5 times
    */
} while (short_irq <=0 && count++ < 5);
/* end of loop, uninstall the handler */
for (i = 0; trials[i]; i++)
    if (tried[i] = = 0)
        free_irq(trials[i], NULL);
if (short_irq < 0)
    printk("short: probe failed %i times, giving up\n", count);
```
IRQ Handler:
```
irqreturn_t short_probing(int irq, void *dev_id, struct pt_regs *regs)
{
    if (short_irq = = 0) short_irq = irq; /* found */
    if (short_irq != irq) short_irq = -irq; /* ambiguous */
    return IRQ_HANDLED;
}
```
## Interrupt Handlers
There are some restrictions for interrupts:
* No sleep functions like semaphores or schedule.
* No access yo user mode.
* No malloc without GFP_ATOMIC.
* Should be small and fast (if you have slow code than use tasklet or workqueue).

## Disabling/Enabling Interrupts
Calling any of these functions may update the maskfor the specified irq in the programmable interrupt controller (PIC), thus disabling or enabling the specified IRQ across all processors.
```
//Disable/Enable a Single IRQ
void disable_irq(int irq);
void disable_irq_nosync(int irq);
void enable_irq(int irq);
```
To disable/enable all the interrupts for a single CPU.
```
void local_irq_save(unsigned long flags);
void local_irq_disable(void);
void local_irq_restore(unsigned long flags);
void local_irq_enable(void);
```
## Top and Bottom Halves

Since you cant spend too much time in ISR (to prevent stack overflow of kernel due the context stacking), ISR is divided into two parts the top half is the normal ISR which later register the bottom part to be executed later on using either tasklet or workqueue.

### Tasklet 
Tasklets are also guaranteed to run on the same CPU as the function that first schedules them. Therefore, an interrupt handler can be secure that a tasklet does not begin executing before the handler has completed. However, another interrupt can certainly
be delivered while the tasklet is running, so locking between the tasklet and the interrupt handler may still be required. <br>
Note that tasklets are not cumulative; if you call them two times only one will be executed. Thus, if multiple same interrupts occurred and executed same tasklet, the driver must handle this by a counter to the received events.
### Workqueues
Like before you can sleep but you cant copy to user space memory.

## Shared Interrupt Lines
You can use shared interrupt lines by `SA_SHIRQ` flag in `request_irq` and passing a unique name to it. And in the ISR you should return IRQ_NONE. You shouldn't disable interrupt line too, because it is shared and not only you who would use it. When an interrupt occurs, all the ISRs will be called. `/proc/interrupts` shows shared interrupts in the same line.

## Interrupt Based I/O

### Write
1. See if there is available space in the buffer, if not sleep until there is.
2. Schedule a timer for timeout.
3. Schedule a write queue.
4. The scheduled write queue will check if all there data in the buffer to be written to the device, if there is write it.
5. The scheduled write queue will check if there is a space in the buffer, if there is wake up the sleeping processes (which they slept in step 1) and delete the timer.
6. When an interrupt occurs indicating that the write operation has been completed, schedule write queue again and go to step 3.
7. When the timer timeout is called it should check if the device is busy (using input ports) if it's then schedule another timer, if not then we have missed an interrupt and we should call the interrupt handler manually.