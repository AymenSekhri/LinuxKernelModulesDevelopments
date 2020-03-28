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
