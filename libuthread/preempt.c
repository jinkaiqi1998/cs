#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
static struct sigaction act;

#define inter 10000

void timer_handler (int sig_num)
{
    uthread_yield();
}

void preempt_disable(void)
{
	sigdelset(&act.sa_mask, SIGVTALRM);
}

void preempt_enable(void)
{
	sigdelset(&act.sa_mask, SIGVTALRM);
}

void preempt_start(void)
{
    struct itimerval timer;
    act.sa_handler = timer_handler;
    sigemptyset(&act.sa_mask);

    /* detect if sigaction has created succussful */
    if( sigaction(SIGVTALRM, &act, NULL) == -1 ) {
        perror("signal create fial\n");
        exit(1);
    }
    /* Initialize timer set up alarm */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = inter;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = inter;

    /* detect if alarm has created succussful */
    if( setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1 ) {
        perror("create alarm fail\n");
        exit(1);
    }
}


