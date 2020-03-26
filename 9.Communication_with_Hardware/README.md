# Communication with Hardware
## using barrier to Stop Compiler Optimization and Processor Pipelining
When accessing memory mapped IO the processor doesn't know if you are accessing an IO and tries to optimize it by reordering the operations or executing them concurrently or even cashing them and preventing them to access the device at all, same as thing happen for the compiler optimization. The Linux kernel provides some functions to stop that. You can see in `/proc/ioports` who got which port.
```
#include <linux/kernel.h>
void barrier(void);
```
This function is used to stop compiler optimization across the barrier.

```
#include <asm/system.h>
void rmb(void);
void read_barrier_depends(void);
void wmb(void);
void mb(void);
```
These functions are used to stop the processor from pipelining and ordering of write operation or read or both.

```
void smp_rmb(void);
void smp_read_barrier_depends(void);
void smp_wmb(void);
void smp_mb(void);
```
Same as above but for SMP kernel.
<br><br>
Example:
```
writel(dev->registers.addr, io_destination_address);
writel(dev->registers.size, io_size);
writel(dev->registers.operation, DEV_READ);
wmb();
writel(dev->registers.control, DEV_GO);
```

## Using IO Ports
### Requesting The Access
Before using a IO port you should request the access to it and the release when you are done.
```
#include <linux/ioport.h>
struct resource *request_region(unsigned long first, unsigned long n, const char *name);
void release_region(unsigned long start, unsigned long n);
int check_region(unsigned long first, unsigned long n);// used to check but it can cause race condition so you shouldn't.
```
### Manipulating The Ports
```
#include <asm/io.h>
// 8bit
unsigned inb(unsigned port);
void outb(unsigned char byte, unsigned port);
// 16bit
unsigned inw(unsigned port);
void outw(unsigned short word, unsigned port);
// 32bit
unsigned inl(unsigned port);
void outl(unsigned longword, unsigned port);
// There is no device that uses 64bit ports (maybe in 2005)
```

### String Operation
Some architectures support IO read/write operations using a sequence of bytes/words/longs in a single instructions. The following macros use that, and for architectures that don't support string operation the macros extends to simple loops for portability.When the port and the host system have different byte ordering rules, the results can be surprising.

```
// Sequence of 8bit
void insb(unsigned port, void *addr, unsigned long count);
void outsb(unsigned port, void *addr, unsigned long count);
// Sequence of 16bit
void insw(unsigned port, void *addr, unsigned long count);
void outsw(unsigned port, void *addr, unsigned long count);
// Sequence of 32bit
void insl(unsigned port, void *addr, unsigned long count);
void outsl(unsigned port, void *addr, unsigned long count);
```