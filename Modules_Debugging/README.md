# Modules Debugging

## Debugging using printk
The classic way of debugging is obviously is printk function.

### View Last Debug Messages
```dmesg | tail -10 | grep "MyLinuxModule"```
Replacing "MyLinuxModule" with a string you should attach to the printk at the beginning function each time to identify the module.

### View Debug Messages in Realtime
*printk* messages can be viewed in realtime with the command ```sudo cat /dev/kmsg```

### Special Macros to Used
Use the macro *__FILE__* to get string of the current file.</br>
Use the macro *__LINE__* to get string of the current line of code.

### Notes
* You should not abuse debugging messages, because it may slow down the system because of disk operations of it.
* You should always disable debugging messages in production modules, use ``` #ifdef DEBUG_FLAG ``` for this.

## Debugging using Querying

### */proc/* Filesystem

*/proc/* is a file system used to query information about the kernel, such that:
* */proc/modules* to get currently running modules.
* */proc/devices* to get currently installed devices.

### Create an Entry in */proc/* Filesystem

```int (*read_proc)(char *page, char **start, off_t offset, int count,int *eof, void *data); ```
``` struct proc_dir_entry *create_proc_read_entry(const char *name,mode_t mode, struct proc_dir_entry *base,
read_proc_t *read_proc, void *data); ``` </br>
``` remove_proc_entry(const char *name, struct proc_dir_entry *parent) ```

