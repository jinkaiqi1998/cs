#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <tps.h>
#include <sem.h>

/* 
 * tps_read(), tps_write() test
 */

static char msg[TPS_SIZE] = "Hello World!\n";
pthread_t t1;


void *thread1(void* arg)
{
	char* buffer = malloc(TPS_SIZE);
	tps_create();

	tps_write(0, TPS_SIZE, msg);
	memset(buffer, 0, TPS_SIZE);
	tps_read(0, TPS_SIZE, buffer);
	assert(!memcmp(msg, buffer, TPS_SIZE));
	printf("thread1: read OK!\n");

	tps_destroy();
	free(buffer);
	return NULL;
}


int main()
{
	tps_init(1);
	pthread_create(&t1, NULL, thread1, NULL);
	pthread_join(t1, NULL);
	
	return 0;
}
