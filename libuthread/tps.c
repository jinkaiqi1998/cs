#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "queue.h"
#include "thread.h"
#include "tps.h"
#define MAXSIZE 4096

static queue_t memMap;

/* 
 * Define a memory page
 * @count: memory page counter
 * @addr: map address
 */
typedef struct MemPage 
{
	int count;
	void* addr;
} MemPage;


/* Define the sturcture of TPS
 * @mp: memory page
 * @tid: thread ID
 */
typedef struct Tps 
{
	pthread_t tid;
	MemPage* mp;
} Tps;


/* 
 * Find a specific signal by searching the map address
 * Return -1 if found
 * Return 0 if not
 */
int check_sig(void* tps_data, void* sig)
{
	Tps* first = (Tps*)tps_data;
    if(first->mp->addr == sig) return -1;
    return 0;
}


/* 
 * Find a specific tid
 * Return -1 if found
 * Return 0 if not
 */
int check_tid(void* tps_data, void* tid)
{
    pthread_t temp = (*(pthread_t*)tid);
    Tps* first = (Tps*)tps_data;
    if ( first->tid == temp ) return 1;
    return 0;
}


/* 
 * A subfunction of tps_create()
 * Set up the memory page when creating a tps
 * Initialize count with 1
 * Initialize addr with mmap()
 */
int mempage_set(Tps* tps)
{
	if (tps->mp != NULL) free(tps->mp);
	tps->mp = malloc(sizeof(MemPage));
	tps->mp->count = 1;
	tps->mp->addr = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (tps->mp->addr == MAP_FAILED) return -1;
	return 0;
}


/* 
 * The frame of segv_handler fuction is provided by the Professor
 */
void segv_handler(int sig, siginfo_t *si, void *context) {    

    void *p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));
	Tps* temp = NULL;

    queue_iterate(memMap, check_sig, (void*)p_fault, (void**)&temp);
    if (temp != NULL) 
		fprintf(stderr, "TPS protection error!\n");

    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    raise(sig);
}


/*
 * Initialize the environment
 * Need and only can be called once.
 * Install a page handler if segv is not 0
 */
int tps_init(int segv)
{
    /* Initialize the memory map first */
	memMap = queue_create();
    if (segv) {
        struct sigaction sa;

        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = segv_handler;
        sigaction(SIGBUS, &sa, NULL);
        sigaction(SIGSEGV, &sa, NULL);
    }
	return 0;
}


/* 
 * Create a TPS
 * Set up the memory page at the same time
 */
int tps_create(void)
{
	enter_critical_section();
	Tps* temp = NULL;
	pthread_t current = pthread_self();
	
    /* 
     * Error Handler
     * TPS in memory map needs to be unique
     * Return -1 if a same tid exists
     */
    if (queue_iterate(memMap, check_tid, (void*)&current, (void**)&temp) != 0) {
		exit_critical_section();
        return -1;
	}

	Tps* new_tps = malloc(sizeof(Tps));
	new_tps->tid = pthread_self();
    
    /* 
     * Error Handler
     * Return -1 if fails to set up a memory page
     */
	if (mempage_set(new_tps) == -1 || new_tps->mp == NULL || new_tps->mp->count != 1) {
        exit_critical_section();
        return -1;
    }
	int state = queue_enqueue(memMap, (void*)new_tps);
	exit_critical_section();
	return state;
	
}


/*
 * Destroy a tps
 */
int tps_destroy(void)
{
    enter_critical_section();
	Tps* temp = NULL;
	pthread_t current = pthread_self();
   
    /* 
     * Error Handler
     * Return -1 if fails to find the TPS to be deleted
     */
	if(queue_iterate(memMap, check_tid, (void*)&current, (void**)&temp) != 1) {
		exit_critical_section();
        return -1;
	}

    /* 
     * Error Handler
     * Return -1 if fails to delete the mapping or the tps
     */
	if(munmap((void*)temp->mp->addr, MAXSIZE) == -1 || queue_delete(memMap, (void*)temp) == -1) {
		exit_critical_section();
        return -1;
	}
    free(temp->mp);
	free(temp);
    exit_critical_section();
    return 0;
}


/* 
 * Read from a tps
 * @offset: the start posisition of reading on mapping address
 * @length: the length of reading on mapping address
 * @buffer: data reciever
 */
int tps_read(size_t offset, size_t length, void *buffer)
{
	enter_critical_section();
	Tps* temp = NULL;
	pthread_t current = pthread_self();

    /* 
     * Error Handler
     * Return -1 if fails to iterate
     */
	if (queue_iterate(memMap, check_tid, (void*)&current, (void**)&temp) == -1) {
		exit_critical_section();
        return -1;
	}

    /* 
     * Error Handler
     * A memory page has a size of 4096
     * Return -1 when the finish position is outside the page
     */
	if((length + offset > MAXSIZE) || (buffer == NULL)) {
		exit_critical_section();
        return -1;
	}

    /* memory protection, reading, memory protection */
	mprotect(temp->mp->addr, TPS_SIZE, PROT_READ);
    memcpy((void*)buffer, (void*)(temp->mp->addr + offset), length);
    mprotect(temp->mp->addr, TPS_SIZE, PROT_NONE);
    exit_critical_section();
    return 0;

}


/* 
 * Write to a memory page
 * @offset: the start posisition of writing on mapping address
 * @length: the length of writing on mapping address
 * @buffer: data to be written
 */
int tps_write(size_t offset, size_t length, void *buffer)
{
	enter_critical_section();
	Tps* temp = NULL;
	pthread_t current = pthread_self();
	
    /* Error Handler */
	if (queue_iterate(memMap, check_tid, (void*)&current, (void**)&temp) == -1) {
		exit_critical_section();
    	return -1;
	}
    
    /* Error Handler */
	if((length + offset > MAXSIZE) || (buffer == NULL)) {
		exit_critical_section();
        return -1;
	}

    /* 
     * Copy on Write
     * Copy a memory page and initialize it
     */
	if (temp->mp->count > 1){
        MemPage* mem = malloc(sizeof(MemPage));
        mem->count = 1;
        mem->addr = mmap(NULL, TPS_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

        mprotect(temp->mp->addr, TPS_SIZE, PROT_READ);
        memcpy((void*)mem->addr, (void*)temp->mp->addr, TPS_SIZE);
        mprotect(temp->mp->addr, TPS_SIZE, PROT_NONE);
        mprotect(mem->addr, TPS_SIZE, PROT_NONE);
        temp->mp = mem;
    }
    
    /* memory protection, writing, memory protection */
	mprotect(temp->mp->addr, TPS_SIZE, PROT_WRITE);
    memcpy((void*)(temp->mp->addr + offset), (void*)buffer, length);
    mprotect(temp->mp->addr, TPS_SIZE, PROT_NONE);
    exit_critical_section();
    return 0;
}

/* 
 * Clone the tps
 */
int tps_clone(pthread_t tid)
{
	enter_critical_section();
	Tps* sample = NULL;
	Tps* temp = NULL;

    /* 
     * Error Handler 
     * Return -1 if fails to find the sample object
     */
    if (queue_iterate(memMap, check_tid, (void*)&tid, (void**)&sample) == -1){
        exit_critical_section();
        return -1;
    }


    temp = malloc(sizeof(Tps));
    temp->tid = pthread_self();
	sample->mp->count ++;
	temp->mp = sample->mp;

    int state = queue_enqueue(memMap, (void*)temp);
    exit_critical_section();
    return state;
	
}