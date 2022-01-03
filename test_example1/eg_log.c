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


enum channel
{
	CHANNEL_FREE,
	CHANNEL_STDOUT,
	CHANNEL_STDERR,
	CHANNEL_COUNT
};
#define QUEUE_CAPACITY 64
#define LOGMSG_CAPACITY 1024
static ck_ring_t g_ring[CHANNEL_COUNT];
static ck_ring_buffer_t * g_rbuffer[CHANNEL_COUNT];
static char * g_msg;


static void enqueue_blocking(enum channel q, intptr_t index)
{
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	bool rv;
enqueue:
	rv = ck_ring_enqueue_mpmc(g_ring+q, g_rbuffer[q], (void*)index);
	if (CK_CC_UNLIKELY(q == CHANNEL_FREE && rv == false))
	{
		fprintf(stderr, "Failure to enqueue the free channel should never happen.");
	}
	if (CK_CC_UNLIKELY(rv == false))
	{
		ck_pr_stall();
		ecs_os_sleep(0, backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
}


static intptr_t dequeue_blocking(enum channel q)
{
	intptr_t index;
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	bool rv;
enqueue:
	rv = ck_ring_dequeue_mpmc(g_ring+q, g_rbuffer[q], (void*)&index);
	if(rv == false)
	{
		ck_pr_stall();
		ecs_os_sleep(0, backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
	return index;
}






static void eg_log_msg1(int32_t level, const char *file, int32_t line, const char *msg)
{
	char const * color = get_color(level);
	char const * text = get_text(level);
	char * buf;
	intptr_t index = INTPTR_MAX;

	{
		index = dequeue_blocking(CHANNEL_FREE);
		buf = g_msg + LOGMSG_CAPACITY * index;
	}

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

	enum channel q;
	if (level < 0)
	{
		snprintf(buf, LOGMSG_CAPACITY, "%s%s"ECS_NORMAL": %s%s:%d: %s", color, text, indent, file, line, msg);
		q = CHANNEL_STDERR;
	}
	else
	{
		snprintf(buf, LOGMSG_CAPACITY, "%s%s"ECS_NORMAL":%s %s", color, text, indent, msg);
		q = CHANNEL_STDOUT;
	}

	enqueue_blocking(q, index);
}








static bool consume_print(enum channel q)
{
	intptr_t index;
	bool rv;
	rv = ck_ring_dequeue_mpmc(g_ring+q, g_rbuffer[q], (void*)&index);
	if (rv == true)
	{
		char * buf = g_msg + LOGMSG_CAPACITY * index;
		printf("Q%i: %s\n", q, buf);
		enqueue_blocking(CHANNEL_FREE, index);
	}
	return rv;
}


static void * thread1(void *arg)
{
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	while(1)
	{
		//ecs_os_sleep(1,0);
		bool rv = consume_print(CHANNEL_STDERR) || consume_print(CHANNEL_STDOUT);
		if (rv == true)
		{
			backoff = CK_BACKOFF_INITIALIZER;
		}
		else
		{
			ck_pr_stall();
			ecs_os_sleep(0, backoff);
			ck_backoff_eb(&backoff);
		}
	}
}

void FlecsComponentsEgLogImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgLog);
	ecs_set_name_prefix(world, "Eg");
	for (int i = 0; i < CHANNEL_COUNT; ++i)
	{
		ck_ring_init(g_ring + i, QUEUE_CAPACITY);
		g_rbuffer[i] = ecs_os_calloc(sizeof(ck_ring_buffer_t) * QUEUE_CAPACITY);
	}
	g_msg = ecs_os_calloc(sizeof(char) * LOGMSG_CAPACITY * QUEUE_CAPACITY);

	for(intptr_t i = 0; i < QUEUE_CAPACITY; ++i)
	{
		int q = CHANNEL_FREE;
		ck_ring_enqueue_mpmc(g_ring + q, g_rbuffer[q], (void*)i);
	}
	ecs_os_api.log_ = eg_log_msg1;
	ecs_os_thread_t t = ecs_os_thread_new(thread1, NULL);
}



