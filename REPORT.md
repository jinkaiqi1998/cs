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

    Three steps are needed to initialize a `semaphore`. Allocate space with the size of `semaphore`. Create a `queue` for the blocked threads. Assign the semaphore counter with the input `count`. 

### sem_destroy ###

    Before deallocating the space, a error handler is needed to check the status of `semaphore`. First, make sure `semaphore` has been correctly initialized. A structure cannot be destroyed if it is not created. Second, make sure there are no more blocked threads in `queue`. Then `free(semaphore)`.

### sem_down ###

    Check the initialization of `semaphore`. Return if it is not correctly initialized. The following actions should be bounded in critical section. If there is no more semaphore available, put the thread into `queue` and block the thread.

### sem_up ###

    Check the initialization of `semaphore`. Return if it is not correctly initialized. The following actions should be bounded in critical section. If there are blocked threads in `queue`, release one.

### sem_getvalue ###

    Check the initialization of `semaphore`. Return if it is not correctly initialized. Change value of `sval` according to the status of `semaphore`.


## phase 2 ##




    The structure `linked_node` is a singly linked list. And since
`queue` wraps this structure with two pointer `head` and `tail`,
the `enqueue` and `dequeue` operations could be done very
efficiently, i.e., in O(1). Though, deleting a specific element
and iterating the whole list still need linear time.

- Each thread has its TCB (*thread control block*), which is designed
as the following.

    ```c
    enum uthread_state {
        UT_READY,
        UT_BLOCKED,
        UT_DEAD
    };

    struct uthread_tcb {
        uthread_t tid;
        uthread_ctx_t *ctx;
        enum uthread_state st;
        uthread_t join_parent;
        int retval;
        void *top_of_stack;
    };
    ```

    The structure `uthread_tcb` contains six fields.
    * `tid` stores the thread identifier.
    * `ctx` stores the `uthread_ctx_t` structure which is
a simple wrapper of `ucontext_t` defined in `context.h`. In this structure
it stores the context of a running thread including the pointer to
the stack, register values, etc.
    * `join_parent` stores the identifier of the parent
thread (who calls `join`).
    * `retval` stores the return value of this thread which could be
specified by `uthread_exit`.
    * `top_of_stack` stores the pointer to the stack.
    * And last, in this design, there are 3 threading states:
`UT_READY`, `UT_BLOCKED`, `UT_DEAD`. When a thread
is ready, it means that it could be the next running thread. When
a thread is blocked, in this project, the only possible case is that
this thread calls `join` and waits some other thread. So at this time
it can't be the target of switching. When a thread is dead, it means that
the thread is no longer alive. However, it still consumes the resource.
Only if some other thread explicitly calls `join`, the resource could be
freed by that parent thread.

- The join mechanism works as following.
    * Thread `A` calls `uthread_join(B)`.
    * If thread `B` is still active (`UT_READY` or `UT_BLOCKED`). Then
first set `join_parent` of `B` be `A` to tell `B` that `A` is waiting now.
Then set `UT_BLOCKED` to `A` and immediately yield.
    * Thread `B` exits with some `retval`. `B` writes this `retval`
to its own tcb. And then `B` set `A`'s state to `UT_READY`. `B` could
know `A` by its `join_parent` in tcb.
    * Finally, `A` could join. And the resource consumed by `B` should be
freed at this time.

- The advanced preemptive mechanism is implemented via `sigaction` and
`setitimer` syscalls.
    * `setitimer` could set an repeated alarm with fixed interval via the
signal. `SIGVTALRM` is chose for this mechanism. And the frequency is 100 Hz.
    * `sigaction` could set the interrupt handler for different signals.
Here for our purpose we need to write the handler for `SIGVTALRM`. The
handler simply calls `uthread_yield`.
    * For implementing the preemption, we also need to protect some operations
which manipulates the global variables. To be short, all operations related
to `thread_queue` should be atomic.
    * For this protective purpose, we need to temporarily disable the
preemption by set back the handler to `SIG_IGN`.

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