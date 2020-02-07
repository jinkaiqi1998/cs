# Project 2 REPORT #
- Zeyu Bai(914237257)
- Kaiqi Jin(914037402)
---
```shell
sshell$ date
2020/2/5/ Tue 13:11:18 PDT
sshell$
```
## Phase 1 ##
- Queue Implement
    
    - At the beginning, we use `element()` function to struct element represents every single element in the queue. It cant store the pointer of current element as well as the pointer of the next element.
    
    - We use `queue()` to store the first and last elements, and the size of queue.
    
    - We use `queue_create()` to initialize the queue and we use `queue_destory()` to free the elements first. 
    
    - We use `queue_enqueue()` to add elements into a queue. In this function, we consider two situation. The queue is empty and the queue already had some members.
    
    - We use `queue_dequeue()` to delete the first element. If the queue is empty already, we will return -1.
    
    - We use `queue_delete()` to delete the data called in other functions. If the data is not found in the queue, we will return value -1. 
    
    - We use `queue_iterate()` to iterate queue when there are members in the queue. When the exection is found, return 0.


## Phase 2 and 3: Uthread ##

- UThread Implement
    - At the beginning, we define `UTHREAD_MAX` and `UTHREAD_STACK_SIZE`. And create 4 static thread_state `ready`, `running`, `blocked`, and `zombie`.
   
     - And then in our `initialization()` function, we check if the queue is empty or not. If empty, we use `queue_create()` to create a thread to main and assign initialize values to them. If it is failed to initialize, we return -1.  At the end we enqueue into the `Ready State` and return the number of queue.

    - We use the function `Uthread_yield()` to dequeue the first `ready state` and switch it with the prvious `running state`. 
    
    - We use the function `uthread_self()` to return the TID of the running thread. 
    
    - We use the function `uthread_exit()` to find the current running state and put it into zombie state which will stop it. And then find the ready state and put it into running state. And then we dertermine if  threads in `zombie-state` has a parent, if so, put it in `ready_state`.
    
    - We use the function `uthread_join()` here to check and put queue into `block_state` and swap it with the thread it join with. It will wait until the child thread finish jobs. After they finish and be put into zombie state to exit, then the parents will collect the values.


# Phase 4: Preemptive #

- The beginning, we define `inter=10000`. And then we write a code below to set up a static struct to receive signal from timer alarm named `act` which is of type `SIGVTALRM`:
```c
static struct sigaction act; 
```

- We use `timer_handler()` to set up a signal handler to receive signals if there are signals transfered.

- We learned the ways to use preemptive from really useful resource `24.7.4 Blocking to Test for Delivery of a Signal`: 
    - We use `preemptive_disable()` to delete the signal flag SIGVTALRM in blockmask:
    
    - We use `preemptive_enable()` to receive signal flag SIGVTALRM into static variable blockmask. 

- In `preempt_start()` function:
    - we set a signal alarm and initialize sigaction. And then define a timer for alarm setting. 

# Test Implements #

- We use the file `queue_tester.c`that professor provided us with. And try to use unit test and add more unit to test our queue.

- We used professor's files `uthread_hello` and `uthread_yield` to test `uthread()`.

- To test the preemptive, we add a file `Preemptive_tester.c`. We tried to implement two threads that can swtich to each other automatically. By this way, we can check if the preempt can yield the current uthread and swtich to another. 


# Resource & Reference:


 - Learning a lot about preemptive from the sources provided by professor [source]:
    - (https://www.gnu.org/software/libc/manual/html_mono/libc.html#Signal-Actions)
    - (https://www.gnu.org/software/libc/manual/html_mono/libc.html#Setting-an-Alarm)
    - (https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-Signals)
 








    
    