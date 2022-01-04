#include "eg_mempool.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ck_ring.h>
#include <ck_backoff.h>



//https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
#define LOG2(x) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((x)) - 1))
#define POW2(x) (1 << (x))
#define POW_MIN 5
#define POW2_NEAREST(x) 1 << (32 - __builtin_clz((x) - 1));


int eg_mempool_size_to_grid(unsigned size)
{
	int r = LOG2(size);
	r = r - POW_MIN;
	r = (r < 0) ? 0 : r;
	return r;
}

void * eg_mempool_malloc(unsigned size)
{
	unsigned s1 = POW2_NEAREST(size+1);
	uint8_t * v = malloc(s1);
	int grid = eg_mempool_size_to_grid(size);
	v[0] = grid;
	printf("malloc %i %i %i\n", size, s1, grid);
	return v + 1;
}



void eg_mempool_init(struct eg_mempool * mem)
{
	for (int i = 0; i < EG_MEMPOOL_GRIDS_COUNT; ++i)
	{
		ck_ring_init(mem->ring + i, EG_MEMPOOL_GRIDS_COUNT);
		mem->buf[i] = malloc(sizeof(ck_ring_buffer_t) * EG_MEMPOOL_QUEUE_CAPACITY);
		size_t size = POW2(i+POW_MIN);
		printf("grid=%i, size=%i\n", i, (int)size);
		//mem->memory[i] = malloc(size);
	}
}


void * eg_mempool_get(struct eg_mempool * q, unsigned size)
{
	void * v;
	int grid = eg_mempool_size_to_grid(size);
	bool rv = ck_ring_dequeue_mpmc(q->ring + grid, q->buf[grid], (void*)&v);
	if (rv == false)
	{
		v = eg_mempool_malloc(size);
	}
	else
	{
		printf("dequeue size=%i, grid=%i\n", size, grid);
	}
	return v;
}

void eg_mempool_reclaim_grid(struct eg_mempool * q, void * mem, unsigned grid)
{
	printf("recycle grid=%i\n", grid);
	bool rv = ck_ring_enqueue_mpmc(q->ring + grid, q->buf[grid], (void*)mem);
	if (rv == false)
	{
		free(mem);
	}
}


void eg_mempool_reclaim(struct eg_mempool * pool, uint8_t * mem, unsigned size)
{
	int grid = eg_mempool_size_to_grid(size);
	mem[0] = grid;
	eg_mempool_reclaim_grid(pool, mem+1, grid);
}

void eg_mempool_reclaim_ex(struct eg_mempool * pool, uint8_t * mem)
{
	int grid = mem[-1];
	eg_mempool_reclaim_grid(pool, mem, grid);
}


void test_eg_mempool()
{
	struct eg_mempool q;
	eg_mempool_init(&q);
	{
		void * m1 = eg_mempool_get(&q, 100);
		void * m2 = eg_mempool_get(&q, 1253);
		void * m3 = eg_mempool_get(&q, 333);
		void * m4 = eg_mempool_get(&q, 444);
		void * m5 = eg_mempool_get(&q, 666);
		eg_mempool_reclaim(&q, m1, 100);
		eg_mempool_reclaim(&q, m2, 1253);
		eg_mempool_reclaim(&q, m3, 333);
		eg_mempool_reclaim(&q, m4, 444);
		eg_mempool_reclaim(&q, m5, 666);
	}

	{
		void * m1 = eg_mempool_get(&q, 100);
		void * m2 = eg_mempool_get(&q, 1253);
		void * m3 = eg_mempool_get(&q, 333);
		void * m4 = eg_mempool_get(&q, 444);
		void * m5 = eg_mempool_get(&q, 666);
	}
}

