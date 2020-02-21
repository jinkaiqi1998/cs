# Project 3 Report #
- Implemented semaphores locks based on project 2. 

- Set a private storage for each thread.

- Added critical sections and synchronization between threads.


## Phase 1 ##

- Implement a structure `semaphore`. The structure contains three objects: `queue` lists all the blocked threads, `blocked` stores the number of elements in the queue and `count` records the number of semaphores that are available.

    ```c
    struct semaphore {
	    int blocked;
	    size_t count;
	    queue_t queue;
    };
    ```
### sem_create ###

Three steps are needed to initialize a `semaphore`. Allocate space 
with the size of `semaphore`. Create a `queue` for the blocked 
threads. Assign the semaphore counter with the input `count`. 

### sem_destroy ###

 Before deallocating the space, a error handler is needed to check 
the status of `semaphore`. First, make sure `semaphore` has been 
correctly initialized. A structure cannot be destroyed if it is not 
created. Second, make sure there are no more blocked threads in 
`queue`. Then `free(semaphore)`.

### sem_down ###

Check the initialization of `semaphore`. Return if it is 
not correctly initialized. The following actions should be bounded 
in critical section. If there is no more semaphore available, put 
the thread into `queue` and block the thread until releasing.

### sem_up ###

Check the initialization of `semaphore`. Return if it is not 
correctly initialized. The following actions should be bounded 
in critical section. If there are blocked threads in `queue`, release one.

### sem_getvalue ###

Check the initialization of `semaphore`. Return if it is not 
correctly initialized. Change value of `sval` according to the status of `semaphore`.


## phase 2 ##

- To design a private storage for each thread, two structures are created 
as following. The main structure `Tps` has two objects. `tid` records the 
ID of the thread and `mp` represents a structure of memory page. To make 
`Tps` easier to understand, a scecond structure `MemPage` is introduced. 
In the structure, `count` functions as a memory counter while `addr` 
stores the mapping address of `mp`. Also a static queue `memMsp` is 
introduced to list all the threads.

    ```c
    static queue_t memMap;
    ```
    ```c
    typedef struct MemPage 
    {
	    int count;
	    void* addr;
    } MemPage;
    ```
    ```c
    typedef struct Tps 
    {
	    pthread_t tid;
	    MemPage* mp;
    } Tps;
    ```
### check_sig ###

It is a subfunction used to simplify `segv_handler()`. The function will 
find whether the input `sig` has already existed in the input `Tps` by 
cheking its `addr`. Return -1 if found, and 0 otherwise.

### check_tid ###

It is a subfunction used to simplify the `queue_iterate()`. It will find 
whether the input `tid` exists in `Tps`. Return -1 if found, and 0 otherwise.

### mempage_set ###

It is a subfunction used to simplify the `tps_create()`. After a `Tps` 
is created, the function will allocate space for memory page, initialize 
`count` as 1 and `addr` with `mmap()`. Return -1 if the function fails 
to set up the memory page.

### segv_handler ###

It is a subfunction used to simplify the `tps_init()`. It iterates through 
`memMap` and print error through `stderr` if TPS protection fails.

### tps_init ###

The function could only be called once to set up the environemt. 
`memMap` should be set up first and then set up the signal alarms.

### tps_create ###

The function allocates space for `Tps` first and then call `mempage_set` 
to set up the memory page. After setting up `Tps`, list it into `memMap`. 
The whole function should be bounded by critical section expcept the error handler part.

### tps_destroy ###

Before destroying `Tps`, `addr` and `queue` should be emptied and deleted first. 
Once `Tps` is emptied, free the memory page and free the TPS at last.

### tps_read ###
The whole function should be bounded into critical section except the 
error handler. Due to memory protection, we need to give reading access 
to the program thorugh `mprotect()` with `PROT_READ` flag. Then, use 
`memcpy()` to read the `addr` into `buffer`.

### tps_write ###
The whole function should be bounded into critical section except the 
error handler. Due to memory protection, we need to give writing access 
to the program thorugh `mprotect()` with `PROT_WRITE` flag. Then, use 
`memcpy()` to write the `addr` from `buffer`. Moreover, a `Copy_on_write` 
function is needed to initialize a memory page before writing the address.
Within the `Copy_on_write`, a new memory page `mem` is allocated and 
initialized as the same standard of mempage_set. Then copy `addr` of 
the to-be-written TPS to `addr` of `mem`. Do the memory protection after reading.

### tps_clone ###
Four steps are taken to implement the fucntion. As usual, the function 
is bounded by critical section except the error handler part. First, 
find the target tps `sample` with `queue_iterate()`. Second, initialzie 
a temp TPS `temp`. Allocate space for it. Then copy the memory page 
from `sample` to `temp`. At last, put `temp` into `memMap`.

## Error Handler ##

Almost every function in Phase 2 needs to consider error handler. Most 
importantly, a TPS needs to be initialized beforing using related API. 
Thus, in `tps_create()`, `tps_destroy()`, `tps_read()`, `tps_write()` 
and `tp_clone()`, a initialization check is needed. The functions 
return -1 if the checker fails. Also, other initialization, i.e. memory page 
initialization, also needs checkers. Except init checks, `queue_iterate()`, 
`queue_enqueue()` and `queue_dequeue()` also needs error handlers since they 
have potential risks to return -1. Lastly, `tps_read()` and `tps_write()` 
needs to check whether `buffer` fits the size of memory page. If `offset + length` 
is larger than `MAXSIZE`, which is defined as 4096 at the beginning, 
reading or writing will not fit inside the memory page, resulting in return at the value of -1. 

## Reference ##

1. https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-for-Handler
2. https://www.gnu.org/software/libc/manual/html_mono/libc.html#Semaphores
3. https://www.gnu.org/software/libc/manual/html_mono/libc.html#Signal-Handling
4. https://linux.die.net/man/2/munmap
5. http://man7.org/linux/man-pages/man2/mprotect.2.html
6. http://man7.org/linux/man-pages/man2/mmap.2.html
7. http://man7.org/linux/man-pages/man7/pkeys.7.html