#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

/*
 * Define the structure of semaphore
 * @blocked: blocked count
 * @count: semaphore count
 * @queue: blocked queue
 */
struct semaphore {
	int blocked;
	size_t count;
	queue_t queue;
};


/*
 * Allocate space for semaphore
 * Initialize the queue for blocked thread
 * Set the smaphore count
 */
sem_t sem_create(size_t count)
{
	sem_t sem = (sem_t)malloc(sizeof(struct semaphore));
	sem->queue = queue_create();
	sem->count = count;
	return sem;
}


/*
 * Destroy the semaphore only when it is empty
 * Otherwise return -1
 */
int sem_destroy(sem_t sem)
{
    if (sem == NULL || queue_length(sem->queue) != 0) return -1;
    free(sem);
    return 0;
}


/* Take a semaphore
 * Block the thread when there is no more semaphore
 */
int sem_down(sem_t sem)
{	
	/* Check initialization*/
	if (sem == NULL) return -1;
	enter_critical_section();
	while (sem->count == 0) {
		queue_enqueue(sem->queue, (void*)pthread_self());
		sem->blocked++;
		thread_block();
	}
	sem->count--;
	exit_critical_section();
	return 0;
}


/*
 * Release a semaphore
 */
int sem_up(sem_t sem)
{
	/* Check initialization*/
	if (sem == NULL) return -1;
	enter_critical_section();
	if (sem->count == 0 && queue_length(sem->queue) != 0) {
		pthread_t unblock;
		queue_dequeue(sem->queue, (void**)&unblock);
        thread_unblock(unblock);
	}
	sem->count++;
	exit_critical_section();
	return 0;
}


/* 
 * Set positive number for available semaphore
 * Set negative number for blocked thread
 */
int sem_getvalue(sem_t sem, int *sval)
{
	/* Check initialization*/
	if (sem == NULL) return -1;
    if (sem->count > 0) *sval = (int)sem->count;
    else *sval = sem->blocked * -1;
    return 0;
}

