# Modules Debugging

## Debugging using printk
The classic way of debugging is obviously is printk function.

### View Last Debug Messages
```dmesg | tail -10 | grep "MyLinuxModule"```
Replacing "MyLinuxModule" with a string you should attach to the printk at the beginning function each time to identify the module.

### View Debug Messages in Realtime
*printk* messages can be viewed in realtime with the command ```sudo cat /dev/kmsg```

### Special Macros to Used
Use the macro *_ _FILE_ _* to get string of the current file.</br>
Use the macro *_ _LINE_ _* to get string of the current line of code.

### Notes
* You should not abuse debugging messages, because it may slow down the system because of disk operations of it.
* You should always disable debugging messages in production modules, use ``` #ifdef DEBUG_FLAG ``` for this.

## Debugging using Querying

### */proc/* Filesystem

*/proc/* is a file system used to query information about the kernel; the files in there are virtual and any attempt to read them will be directed to driver function to fill up the buffer. Some of these entries:
* */proc/modules* to get currently running modules.
* */proc/devices* to get currently installed devices.
There is no disk operation in this filesystem (since it's virtual) so it will be faster than printk.

### Create an Entry in */proc/* Filesystem

```int (*read_proc)(char *page, char **start, off_t offset, int count,int *eof, void *data); ```
``` struct proc_dir_entry *create_proc_read_entry(const char *name,mode_t mode, struct proc_dir_entry *base,read_proc_t *read_proc, void *data); ```
``` remove_proc_entry(const char *name, struct proc_dir_entry *parent) ```
* you should check if the name exists before create or remove an entry in the /proc/ because the kernel trusts the driver on that, idk why lol.

### file_seq interface
This seems like creating an entry in */proc/* but with different functions and supposed to work good with large files (see KDD3 book for more information). But i'm gonna stick with the printk since i wont be printing much anyway.