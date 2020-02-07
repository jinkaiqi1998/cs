#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "preempt.h"
#include "uthread.h"
#define inter 10000

static struct sigaction act;
static sigset_t blockmask;

void handler(int signum) 
{
     uthread_yield();

}

void preempt_disable(void)
{
  	act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    act.sa_mask = blockmask;
}

void preempt_enable(void)
{
    sigaddset (&blockmask, SIGINT);
  	sigaddset (&blockmask, SIGQUIT);
  	act.sa_handler = handler;
    act.sa_flags = 0;
  	act.sa_mask = blockmask;
}

void preempt_start(void)
{
    struct itimerval timer;
	sigemptyset (&blockmask);
	sigaction(SIGVTALRM, &act, NULL);
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = inter;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = inter;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}


