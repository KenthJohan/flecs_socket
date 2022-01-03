#include "eg_cklog.h"
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>



void eg_cklog_create(struct eg_cklog * context, unsigned queue_capacity, unsigned msg_capacity)
{
	ck_ring_init(&context->ring_free, queue_capacity);
	ck_ring_init(&context->ring_used, queue_capacity);
	context->buffer_free = malloc(sizeof(ck_ring_buffer_t) * queue_capacity);
	context->buffer_used = malloc(sizeof(ck_ring_buffer_t) * queue_capacity);
	context->msg_mem = malloc(sizeof(char) * queue_capacity * msg_capacity);
	context->msg_size = msg_capacity;
	for(intptr_t i = 0; i < queue_capacity; ++i)
	{
		bool rv = ck_ring_enqueue_mpsc(&context->ring_free, context->buffer_free, (void*)i);
		//printf("Dummy rv:%i\n", rv);
	}
}



bool eg_cklog_enqueue(struct eg_cklog * context, char const * msg)
{
	intptr_t i;
	bool rv;
	size_t n = context->msg_size;
	rv = ck_ring_dequeue_mpmc(&(context->ring_free), context->buffer_free, (void*)&i);
	if (CK_CC_UNLIKELY(rv == false)){return false;}
	strncpy(context->msg_mem + i * n, msg, n);
enqueue:
	rv = ck_ring_enqueue_mpmc(&(context->ring_used), context->buffer_used, (void*)i);
	if (CK_CC_UNLIKELY(rv == false))
	{
		// Unlikely
		ck_pr_stall();
		goto enqueue;
	}
}


void eg_cklog_consume(struct eg_cklog * context)
{
	ck_backoff_t backoff;
	intptr_t i;
	bool rv;
	backoff = CK_BACKOFF_INITIALIZER;
dequeue:
	rv = ck_ring_dequeue_mpmc(&(context->ring_used), context->buffer_used, (void*)&i);
	if (rv == false)
	{
		//printf("backoff: %i\n", backoff);
		ck_pr_stall();
		usleep(backoff);
		ck_backoff_eb(&backoff);
		goto dequeue;
	}
	char * msg = context->msg_mem + i * context->msg_size;
	//printf("%i ", ck_ring_size(&(context->ring_free)));
	//printf("%i ", ck_ring_size(&(context->ring_used)));
	printf("%s\n", msg);
	backoff = CK_BACKOFF_INITIALIZER;
enqueue:
	rv = ck_ring_enqueue_mpmc(&(context->ring_free), context->buffer_free, (void*)i);
	if (CK_CC_UNLIKELY(rv == false))
	{
		// This is impossible ?
		ck_pr_stall();
		usleep(backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
}




void log_msg1(int32_t level, const char *file, int32_t line, const char *msg)
{

}




/*


void * consumer(void *msg)
{
	while (1)
	{
		void *result;
		// Multiple producers single consumer
		while ()
		{
			bool a = ck_ring_dequeue_mpsc(&ring, buffer, &result);
			printf("%s", msg);
			bool b = ck_ring_enqueue_mpsc(&ring, buffer, &result);

		}
		usleep(10000);
	}
}



void logck_init()
{
	ck_ring_init(&ring, 4);
	msgv = calloc(1, 1024*1024);
	pthread_t thr;
	pthread_create(&thr, NULL, consumer, "hello123456789");
}

*/








