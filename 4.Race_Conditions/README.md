# Race Conditions
Race condition happens when a process/thread tries to check a data while another process/thread is writing to it.
### Semaphore
Semaphore is an integer variable with functions; up and down. the down function checks if the semaphore's value is greater than zero, so it decreases it and continues the execution, otherwise it sends the process to sleep, the up function increases the value of the semaphore, and wakes up the sleeping processes/threads if there are any (semaphore value less than 0).<br>
* if the semaphore's value is zero or negative; the number indicates who many processes/threads are sleeping and waiting for the execution, and that no other processes/threads will execute if they enter the critical region.
* if the semaphore's value is positive; the number indicates who many processes/threads can enter the critical region directly without being told to sleep.
### Mutex
Mutex is a semaphore but with a maximum value of 1, which means that only one process/thread can enter the critical region.


