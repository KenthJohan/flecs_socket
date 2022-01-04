#include "eg_log.h"
#include "eg_mempool.h"

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
	CHANNEL_STDOUT,
	CHANNEL_STDERR,
	CHANNEL_COUNT
};
#define EG_LOG_RING_CAPACITY 64
static ck_ring_t g_ring[CHANNEL_COUNT];
static ck_ring_buffer_t * g_rbuffer[CHANNEL_COUNT];
static struct eg_mempool pool;



static void enqueue_blocking(enum channel q, char* msg)
{
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	bool rv;
enqueue:
	rv = ck_ring_enqueue_mpmc(g_ring+q, g_rbuffer[q], (void*)msg);
	if (CK_CC_UNLIKELY(rv == false))
	{
		ck_pr_stall();
		ecs_os_sleep(0, backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
}


static char * dequeue_blocking(enum channel q)
{
	char * msg;
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	bool rv;
enqueue:
	rv = ck_ring_dequeue_mpmc(g_ring+q, g_rbuffer[q], (void*)&msg);
	if(rv == false)
	{
		ck_pr_stall();
		ecs_os_sleep(0, backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
	return msg;
}






static void eg_log_msg1(int32_t level, const char *file, int32_t line, const char *msg)
{
	char const * color = get_color(level);
	char const * text = get_text(level);
	int len = strlen(msg);
	char * buf = eg_mempool_get(&pool, len);
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
		sprintf(buf, "%s%s"ECS_NORMAL": %s%s:%d: %s", color, text, indent, file, line, msg);
		q = CHANNEL_STDERR;
	}
	else
	{
		sprintf(buf, "%s%s"ECS_NORMAL":%s %s", color, text, indent, msg);
		q = CHANNEL_STDOUT;
	}
	enqueue_blocking(q, buf);
}








static bool consume_print(enum channel q)
{
	char * buf;
	bool rv;
	rv = ck_ring_dequeue_mpmc(g_ring+q, g_rbuffer[q], (void*)&buf);
	if (rv == true)
	{
		printf("Q%i: %s\n", q, buf);
		eg_mempool_reclaim_ex(&pool, (uint8_t*)buf);
	}
	return rv;
}

int stall = 0;
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
			stall++;
			ck_pr_stall();
			ecs_os_sleep(0, backoff);
			ck_backoff_eb(&backoff);
		}
	}
}

static void * thread2(void *arg)
{
	int i = 0;
	while(1)
	{
		ecs_os_sleep(1,0);
		ecs_trace("Testing %i %i", i++, stall);
	}
}


void FlecsComponentsEgLogImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgLog);
	ecs_set_name_prefix(world, "Eg");
	for (int i = 0; i < CHANNEL_COUNT; ++i)
	{
		ck_ring_init(g_ring + i, EG_LOG_RING_CAPACITY);
		g_rbuffer[i] = ecs_os_calloc(sizeof(ck_ring_buffer_t) * EG_LOG_RING_CAPACITY);
	}
	eg_mempool_init(&pool);

	ecs_os_api.log_ = eg_log_msg1;
	ecs_os_thread_t t1 = ecs_os_thread_new(thread1, NULL);
	ecs_os_thread_t t2 = ecs_os_thread_new(thread2, NULL);


	//printf("%i %i\n", 17, LOG2(17));
	//printf("%i %i\n", 1024, LOG2(4096));

	//test_eg_mempool();
}



