#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>
#include <sem.h>


/*
 * tps_clone test
 */

static char msg[TPS_SIZE] = "Hello World!\n";
pthread_t t1, t2;

void *thread2(void* arg)
{
    char *buffer = malloc(TPS_SIZE);
    tps_clone(t1);
    tps_read(0, TPS_SIZE, buffer);
    assert(!memcmp(msg, buffer, TPS_SIZE));
    printf("thread2: clone OK!\n");
    tps_destroy();
    free(buffer);
    return NULL;
}

void *thread1(void* arg)
{
 	tps_create();
	tps_write(0, TPS_SIZE, msg);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_join(t2, NULL);

  	tps_destroy();
  	return NULL;
}


int main()
{
  	tps_init(1);
  	pthread_create(&t1, NULL, thread1, NULL);
  	pthread_join(t1, NULL);
	return 0;
}
