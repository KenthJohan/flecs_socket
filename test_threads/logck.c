#include <ck_ring.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

ck_ring_t ring;
ck_ring_buffer_t buffer[1024];
char * msgv;

struct msgbuf
{
	char * msg;
	int size;
};

void msgbuf_malloc(struct msgbuf * m)
{

}




void log_msg1(int32_t level, const char *file, int32_t line, const char *msg)
{
}


int i = 0;
void log_msg(const char *msg)
{
	void *entry = msg;
	if (ck_ring_enqueue_spsc(&ring, buffer, &entry) == false)
	{
		printf("ck_ring_enqueue_spsc is full: %i", i);
		return;
	}
	i ++;
}




void * consumer(void *msg)
{
	while (1)
	{
		void *result;
		while (ck_ring_dequeue_spsc(&ring, buffer, &result))
		{

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










