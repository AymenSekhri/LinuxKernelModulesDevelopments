# Communication with Hardware
## using barrier to Stop Compiler Optimization and Processor Pipelining
When accessing memory mapped I/O the processor doesn't know if you are accessing an I/O and tries to optimize it by reordering the operations or executing them concurrently or even cashing them and preventing them to access the device at all, same as thing happen for the compiler optimization. The Linux kernel provides some functions to stop that. You can see in `/proc/ioports` who got which port.
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

## Using I/O Ports
### Requesting The Access
Before using a I/O port you should request the access to it and the release when you are done.
```
#include <linux/ioport.h>
struct resource *request_region(unsigned long first, unsigned long n, const char *name);
void release_region(unsigned long start, unsigned long n);
int check_region(unsigned long first, unsigned long n);// used to check but it can cause race condition so you shouldn't.
```
### Manipulating The Ports
The following functions are used to access the I/O independently from whether the platform is actually a port mapped I/O or memory mapped. 
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
Some architectures support I/O read/write operations using a sequence of bytes/words/longs in a single instructions. The following macros use that, and for architectures that don't support string operation the macros extends to simple loops for portability.When the port and the host system have different byte ordering rules, the results can be surprising.

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

## Using I/O Memory
### Requesting The Access
Before using a I/O port you should request the access to it and the release when you are done. All I/O memory
allocations are listed in `/proc/iomem`.
```
#include <linux/ioport.h>
struct resource *request_mem_region(unsigned long start, unsigned long len, char *name);
void release_mem_region(unsigned long start, unsigned long len);
int check_mem_region(unsigned long start, unsigned long len); // used to check but it can cause race condition so you shouldn't.
```
Before accessing the I/O memory we need to insure that the physical addresses of the IO is currently in page tables and mapped to a virtual address.
```
#include <asm/io.h>
void *ioremap(unsigned long phys_addr, unsigned long size);
void *ioremap_nocache(unsigned long phys_addr, unsigned long size);
void iounmap(void * addr);
```
### Accessing The I/O Memory

```
#include <asm/io.h>
unsigned int ioread8(void *addr);
unsigned int ioread16(void *addr);
unsigned int ioread32(void *addr);

void iowrite8(u8 value, void *addr);
void iowrite16(u16 value, void *addr);
void iowrite32(u32 value, void *addr);

//String Operations
void ioread8_rep(void *addr, void *buf, unsigned long count);
void ioread16_rep(void *addr, void *buf, unsigned long count);
void ioread32_rep(void *addr, void *buf, unsigned long count);

void iowrite8_rep(void *addr, const void *buf, unsigned long count);
void iowrite16_rep(void *addr, const void *buf, unsigned long count);
void iowrite32_rep(void *addr, const void *buf, unsigned long count);
```
Note that the count is as respecting to the data being read/written for example iowrite32_rep writes count * 4 bytes.

```
void memset_io(void *addr, u8 value, unsigned int count);
void memcpy_fromio(void *dest, void *source, unsigned int count);
void memcpy_toio(void *dest, void *source, unsigned int count);
```
### Ports as I/O Memory
This function remaps count I/O ports and makes them appear to be I/O memory.
```
void *ioport_map(unsigned long port, unsigned int count);
void ioport_unmap(void *addr);
```
Note that you must be allocated the ports before using them.