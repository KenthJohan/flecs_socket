#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ck_ring.h>
#include <ck_backoff.h>



#define EG_MEMPOOL_GRIDS_COUNT 16
#define EG_MEMPOOL_QUEUE_CAPACITY 1024

struct eg_mempool
{
	ck_ring_t ring[EG_MEMPOOL_GRIDS_COUNT];
	ck_ring_buffer_t * buf[EG_MEMPOOL_GRIDS_COUNT];
};


void eg_mempool_init(struct eg_mempool * pool);
void * eg_mempool_get(struct eg_mempool * pool, unsigned size);
void eg_mempool_reclaim(struct eg_mempool * pool, uint8_t * mem, unsigned size);
void eg_mempool_reclaim_ex(struct eg_mempool * pool, uint8_t * mem);

void test_eg_mempool();




