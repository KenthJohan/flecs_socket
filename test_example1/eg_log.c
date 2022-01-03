#include "eg_log.h"

#include <ck_ring.h>
#include <ck_backoff.h>




static char const * get_color(int level)
{
	switch (level)
	{
	case -1: return "";
	case -2: return ECS_YELLOW;
	case -3: return ECS_RED;
	case -4: return ECS_RED;
	default: return ECS_MAGENTA;
	}
}
static char const * get_text(int level)
{
	switch (level)
	{
	case -1: return "";
	case -2: return "warning";
	case -3: return "error";
	case -4: return "fatal";
	default: return "info";
	}
}

enum
{
	Q_FREE,
	Q_STDIN,
	Q_STDERR,
	Q_COUNT
};
#define QN 64
#define CAP 1024
ck_ring_t ring[Q_COUNT];
ck_ring_buffer_t * buffer[Q_COUNT];
char * g_msg;







static void eg_log_msg1(int32_t level, const char *file, int32_t line, const char *msg)
{
	char const * color = get_color(level);
	char const * text = get_text(level);

	bool rv;
	intptr_t index;
	printf("ringsize1: %i\n", ck_ring_size(ring+Q_FREE));
	do
	{
		rv = ck_ring_dequeue_mpmc(ring+Q_FREE, buffer[Q_FREE], (void*)&index);
	}
	while(rv == false);
	printf("ringsize2: %i\n", ck_ring_size(ring+Q_FREE));
	char * buf = g_msg + CAP * index;
	char indent[32] = {'\0'};
	if (level >= 0)
	{
		if (ecs_os_api.log_indent_)
		{
			int i;
			for (i = 0; i < ecs_os_api.log_indent_; i ++)
			{
				indent[i * 2] = '|';
				indent[i * 2 + 1] = ' ';
			}
			indent[i * 2] = '\0';
		}
	}

	int q;
	if (level < 0)
	{
		snprintf(buf, 1024, "%s%s"ECS_NORMAL": %s%s:%d: %s", color, text, indent, file, line, msg);
		q = Q_STDERR;
	}
	else
	{
		snprintf(buf, 1024, "%s%s"ECS_NORMAL":%s %s", color, text, indent, msg);
		q = Q_STDIN;
	}

	rv = ck_ring_enqueue_mpmc(ring+q, buffer[q], (void*)index);
	printf("rv: %i %i\n", rv, index);
}


static void * thread1(void *arg)
{
	bool rv1;
	bool rv2;
	while(1)
	{
		ecs_os_sleep(1,0);
		intptr_t index1;
		intptr_t index2;
		rv1 = ck_ring_dequeue_mpmc(ring+Q_STDIN, buffer[Q_STDIN], (void*)&index1);
		rv2 = ck_ring_dequeue_mpmc(ring+Q_STDERR, buffer[Q_STDERR], (void*)&index2);

		if(rv1)
		{
			char * buf = g_msg + CAP * index1;
			printf("thread1: %s\n", buf);
		}
		if(rv2)
		{
			char * buf = g_msg + CAP * index2;
			printf("thread1: %s\n", buf);
		}

		if (rv1)
		{
			do
			{
				rv1 = ck_ring_enqueue_mpmc(ring+Q_FREE, buffer[Q_FREE], (void*)index1);
			}
			while(rv1 == false);
		}

		if (rv2)
		{
			do
			{
				rv2 = ck_ring_enqueue_mpmc(ring+Q_FREE, buffer[Q_FREE], (void*)index2);
			}
			while(rv2 == false);
		}


	}
}

void FlecsComponentsEgLogImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgLog);
	ecs_set_name_prefix(world, "Eg");



	ck_ring_init(ring + Q_FREE, QN);
	ck_ring_init(ring + Q_STDIN, QN);
	ck_ring_init(ring + Q_STDERR, QN);
	buffer[Q_FREE] = ecs_os_calloc(sizeof(ck_ring_buffer_t) * QN);
	buffer[Q_STDIN] = ecs_os_calloc(sizeof(ck_ring_buffer_t) * QN);
	buffer[Q_STDERR] = ecs_os_calloc(sizeof(ck_ring_buffer_t) * QN);
	g_msg = ecs_os_calloc(sizeof(char) * CAP * QN);

	for(intptr_t i = 0; i < QN; ++i)
	{
		ck_ring_enqueue_mpmc(ring + Q_FREE, buffer[Q_FREE], (void*)i);
	}


	ecs_os_api.log_ = eg_log_msg1;
	ecs_os_thread_t t = ecs_os_thread_new(thread1, NULL);


}



