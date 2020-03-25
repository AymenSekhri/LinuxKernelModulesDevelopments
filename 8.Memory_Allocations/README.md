# Memory Allocations
## kmalloc Flags
#### GFP_ATOMIC
Used to allocate memory from interrupt handlers and other code outside of a
process context. Never sleeps.
#### GFP_KERNEL
Normal allocation of kernel memory. May sleep.
#### GFP_USER
Used to allocate memory for user-space pages; it may sleep.
#### GFP_HIGHUSER
Like GFP_USER, but allocates from high memory, if any. High memory is
described in the next subsection.
#### GFP_NOIO
#### GFP_NOFS
These flags function like GFP_KERNEL, but they add restrictions on what the kernel can do to satisfy the request. A GFP_NOFS allocation is not allowed to perform any filesystem calls, while GFP_NOIO disallows the initiation of any I/O at all. They are used primarily in the filesystem and virtual memory code where an allocation
may be allowed to sleep, but recursive filesystem calls would be a bad idea.<br><br>

The allocation flags listed above can be augmented by an ORing in any of the following flags, which change how the allocation is carried out:
#### __GFP_DMA
This flag requests allocation to happen in the DMA-capable memory zone. The exact meaning is platform-dependent and is explained in the following section.
#### __GFP_HIGHMEM
This flag indicates that the allocated memory may be located in high memory.
#### __GFP_COLD
Normally, the memory allocator tries to return “cache warm” pages—pages that
are likely to be found in the processor cache. Instead, this flag requests a “cold”
page, which has not been used in some time. It is useful for allocating pages for
DMA reads, where presence in the processor cache is not useful. See the section
“Direct Memory Access” in Chapter 1 for a full discussion of how to allocate
DMA buffers.
#### __GFP_NOWARN
This rarely used flag prevents the kernel from issuing warnings (with printk) when an allocation cannot be satisfied.
#### __GFP_HIGH
This flag marks a high-priority request, which is allowed to consume even the last pages of memory set aside by the kernel for emergencies.
#### __GFP_REPEAT
#### __GFP_NOFAIL
#### __GFP_NORETRY
These flags modify how the allocator behaves when it has difficulty satisfying an allocation. __GFP_REPEAT means “try a little harder” by repeating the attempt—but the allocation can still fail. The __GFP_NOFAIL flag tells the allocator never to fail; it works as hard as needed to satisfy the request. Use of __GFP_NOFAIL is very strongly discouraged; there will probably never be a valid reason to use it in a device driver. Finally, __GFP_NORETRY tells the allocator to give up immediately if the requested memory is not available.

### High Memory vs DMA Memory vs Normal Memory
DMA memory is lower address memory for some architectures and used for direct IO memory access, High memory is used to access some larger memory regions which is harder to deal with.
if  `__GFP_DMA` is selected than memory is searched in normal and DMA memory, if `__GFP_HIGHMEM` is selected then the memory is searched in all three regions.
## Lookaside Caches
The book didn't explain it intensively and it said that the most drivers wont be needing it anyway.<br>
To get more about it look about Lookaside Caches chapter in the operating system books in the other repository.
```
//They should be called in given order.
kmem_cache_t *kmem_cache_create(const char *name, size_t size, size_t offset, unsigned long flags,
                                 void (*constructor)(void *, kmem_cache_t *,unsigned long flags),
                                 void (*destructor)(void *, kmem_cache_t *,unsigned long flags));
// The name will not be copied to kernel so you should not de-allocate it.
void *kmem_cache_alloc(kmem_cache_t *cache, int flags);
void kmem_cache_free(kmem_cache_t *cache, const void *obj);
int kmem_cache_destroy(kmem_cache_t *cache);
```
## get_free_page and its friends
`get_free_page` as opposed to `kmalloc` allocated pages that are physically contagious and aligned, kmalloc may stack multiple small allocations into one page. `kmalloc` uses `get_free_page` for some flags.
```
get_zeroed_page(unsigned int flags);
__get_free_page(unsigned int flags);
__get_free_pages(unsigned int flags, unsigned int order);
void free_page(unsigned long addr);
void free_pages(unsigned long addr, unsigned long order);
```
## vmalloc and its friends
The memory allocated using `vmalloc` is virtually contiguous but not physically contiguous. Addresses allocated by vmalloc can’t be used outside of the microprocessor, because they make sense only on top of the processor’s MMU. When a driver needs a real
physical address (such as a DMA address, used by peripheral hardware to drive the system’s bus), you can’t easily use vmalloc. The right time to call vmalloc is when you are allocating memory for a large sequential buffer that exists only in software.
It’s important to note that vmalloc has more overhead than __get_free_pages, because it must both retrieve the memory and build the page tables. Therefore, it doesn’t make sense to call vmalloc to allocate just one page.Both ioremap and vmalloc are page oriented (they work by modifying the page tables); consequently, the relocated or allocated size is rounded up to the nearest page boundary.
```
#include <linux/vmalloc.h>
void *vmalloc(unsigned long size);
void vfree(void * addr);
void *ioremap(unsigned long offset, unsigned long size);// get the address and doesn't allocate it.
void iounmap(void * addr);
```
## Per-CPU Variable
Per-CPU Variable are variables defined for each processor and accessing them doesn't require any locking since they are different copies but they require to the preemption to be disabled which is done internally by the following functions.
```
DEFINE_PER_CPU(type, name);
DEFINE_PER_CPU(int[3], my_percpu_array);
get_cpu_var(value);
put_cpu_var(value);
per_cpu(variable, int cpu_id);// access other copy of Per-CPU variable
void *alloc_percpu(type);
void *__alloc_percpu(size_t size, size_t align);
per_cpu_ptr(void *per_cpu_var, int cpu_id);
free_percpu(address);
```