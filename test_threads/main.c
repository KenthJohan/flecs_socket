#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "logck.h"

void *printme(void *msg)
{
	int i = 0;
	while (1)
	{
		usleep(1000);
		char buf[100] = {'\0'};
		snprintf(buf, 100, "%s: %i\n", (char*)msg, i++);
		log_msg(buf);
	}
}

int main(int argc, char * argv[])
{
	logck_init();
	pthread_t thr;
	pthread_create(&thr, NULL, printme, "hello");
	pthread_create(&thr, NULL, printme, "goodbye");
	int i = 0;
	while(1)
	{
		usleep(10000);
		printf("i: %d\n", i++);
	}
	pthread_join(thr, NULL);
}
