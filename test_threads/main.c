#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "eg_cklog.h"

struct eg_cklog g_logctx;


void *printme(void *msg)
{
	eg_cklog_consume(&g_logctx);
	eg_cklog_consume(&g_logctx);
	eg_cklog_consume(&g_logctx);
	eg_cklog_consume(&g_logctx);
}

int main(int argc, char * argv[])
{
	eg_cklog_create(&g_logctx, 8, 1024);
	eg_cklog_enqueue(&g_logctx, "Hello folks!");
	eg_cklog_enqueue(&g_logctx, "Vote for martians!");
	eg_cklog_enqueue(&g_logctx, "Blue Banana!");

	pthread_t thr;
	pthread_create(&thr, NULL, printme, "hello");
	pthread_join(thr, NULL);

	ck_pr_stall();

	/*
	int i = 0;
	while(1)
	{
		usleep(10000);
		printf("i: %d\n", i++);
	}
	pthread_join(thr, NULL);
	*/
}
