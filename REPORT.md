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

-To design a private storage for each thread, two structures are created 
as following. The main structure `Tps` has two objects. `tid` records the 
ID of the thread and `mp` represents a structure of memory page. To make 
`Tps` easier to understand, a scecond structure `MemPage` is introduced. 
In the structure, `count` functions as a memory counter while `addr` 
stores the mapping address of `mp`.


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
    

## Some More Details

- For implementing the round-robin strategy, when a thread yield its execution,
the scheduler first remove this thread from the `thread_queue` (it should be
at the fore of the queue), and then push this thread to the back of queue.
Finally, the scheduler looks up an available thread (`UT_READY`) from front
to end in the queue. This strategy simulates a circular queue efficiently.

- At the first time calling `uthread_create`, we also need to create the main
thread for scheduling later. This function is implemented in
`int uthread_init()` in `uthread.c`.

- To avoid redundant code, several helper functions for iterating the queue
is implemented as following.

    ```c
    int queue_round_robin(void *data, void *arg);

    int find_in_queue(void *data, void *arg);

    int debug_queue(void *data, void *arg);
    ```
    `queue_round_robin` finds the first available thread as described above.
`find_in_queue` finds the thread which equals to `data`. `debug_queue`
prints all elements in the queue for debug purpose.

## Reference

1. [signal handler](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-for-Handler)
2. [set timer](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Setting-an-Alarm)