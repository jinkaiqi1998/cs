#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

#define UTHREAD_STACK_SIZE 32768

enum thread_state {
	Ready,
	Running,
	Blocked,
	Zombie
};

struct uthread {
	uthread_t tid;
	uthread_t parent_tid;
	uthread_ctx_t* text;
	void* stack;
	enum thread_state state;
	int retval;
};

static queue_t ready;
static queue_t running;
static queue_t blocked;
static queue_t zombie;
static uthread_t running_tid;
int tmarker = 0; //tmarker marks every thread with unique TID.

void initialization(uthread_func_t func)
{	
	struct uthread* head = (struct uthread*)malloc(sizeof(struct uthread));
	ready = queue_create();
	running = queue_create();
	blocked = queue_create();
	zombie = queue_create();
	
	head->tid = 0;
	head->parent_tid = -1;
	head->text = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));
	head->stack = uthread_ctx_alloc_stack();
	head->state = Ready;
	head->retval = 0;


	head->text->uc_stack.ss_sp = head->stack;
	head->text->uc_stack.ss_size = UTHREAD_STACK_SIZE;
	
	queue_enqueue(running, head);
	running_tid = 0;
	preempt_start();	
}

int thread_search(void* data, void* tid)
{	
	struct uthread* temp = (struct uthread*)data;
	if ( temp->tid == *(uthread_t*) tid) return 1;
	return 0;
}

/* 
 * Yield function has 3 steps:
 * First, enqueue the running thread into ready
 * Second, enqueue the first element in ready to running
 * At last, switch the context.
 */

void uthread_yield(void)
{
	struct uthread* to_ready = NULL;
	struct uthread* to_running = NULL;

	if(queue_dequeue(running, (void**) &to_ready) == 0 && 
			queue_dequeue(ready, (void**) &to_running) == 0) {
		
		preempt_disable();
		queue_enqueue(running,to_running);
		running_tid = to_running->tid;
        queue_enqueue(ready,to_ready);
		to_ready->state = Ready;
		to_running->state = Running;
		uthread_ctx_switch(to_running->text, to_ready->text);
		preempt_enable();
	}
}


/*
 * Return the TID of the running thread (or the first element of running)
 */
uthread_t uthread_self(void)
{
  	return running_tid;
}


/*
 * Create a new thread and then initiate its execution context
 * If it failed the initialization return -1
 */

int uthread_create(uthread_func_t func, void *arg)
{
	struct uthread* new_thread = (struct uthread*)malloc(sizeof(struct uthread));
	if (tmarker == 0) initialization(func);
	new_thread->tid = ++tmarker;
	new_thread->parent_tid = -1;
	new_thread->retval = 0;
	new_thread->text = (void*)malloc(sizeof(uthread_ctx_t));
	new_thread->stack = uthread_ctx_alloc_stack();
	int curstate = uthread_ctx_init(new_thread->text, new_thread->stack, func, arg);
	if (curstate == -1) return -1;
	else {
		new_thread->state = Ready;
		queue_enqueue(ready, new_thread);
		return new_thread->tid;
	}
}


/*
 * If there is thread in runnning, exit it.
 * If there is thread in ready, put it in Running
 * If the thread ended has a parent, put it in Ready
 */
void uthread_exit(int retval)
{
	struct uthread* to_running = NULL;
	struct uthread* to_zombie = NULL;
	struct uthread* to_ready = NULL;

	if(queue_dequeue(running, (void**) &to_zombie) == 0) {
		preempt_disable();
    	if(to_zombie->parent_tid > -1) {
			queue_iterate(blocked, thread_search, (void*) &to_zombie->parent_tid, (void**) &to_ready);
			if (to_ready != NULL) {
				to_ready->state = Ready;
        		queue_delete(blocked, to_ready);
        		queue_enqueue(ready, to_ready);
			}
		}
		to_zombie->retval = retval;
		to_zombie->state = Zombie;
		queue_enqueue(zombie, to_zombie);
		
		if(queue_dequeue(ready, (void**) &to_running) == 0) {
			to_running->state = Running;
			queue_enqueue(running,to_running);
			running_tid = to_running->tid;
			uthread_ctx_switch(to_zombie->text, to_running->text);
		}
		preempt_enable();
	}
}



int uthread_join(uthread_t tid, int *retval)
{
	struct uthread* child = NULL;
    struct uthread* parent = NULL;
	struct uthread* to_running = NULL;

	if (tid == 0 || tid == running_tid) return -1;
	queue_iterate(zombie, thread_search, (void*)&tid, (void**)&child);

	if (child != NULL) {
		*retval = child->retval;
		queue_delete(zombie, child);
		return 0;
	}

	queue_iterate(ready, thread_search, (void*)&tid, (void**)&child);
	queue_iterate(running, thread_search, (void*)&tid, (void**)&child);
	if (child == NULL || child->parent_tid > -1) return -1;

	child->parent_tid = running_tid;
	if ((queue_dequeue(running, (void**) &parent) != 0) || 
			(queue_dequeue(ready, (void**) &to_running) != 0)) {
		return -1;
	}
	preempt_disable();
	parent->state = Blocked;
	to_running->state = Running;
	queue_enqueue(blocked, parent);
    queue_enqueue(running, to_running);
	running_tid = to_running->tid;
	uthread_ctx_switch(parent->text, to_running->text);
	*retval = child->retval;
	preempt_enable();

	return 0;
}

