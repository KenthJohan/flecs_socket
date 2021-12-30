#pragma once
#include <stdint.h>
#include <ck_ring.h>
#include <ck_backoff.h>

struct eg_cklog
{
	ck_ring_t ring_free;
	ck_ring_t ring_used;
	ck_ring_buffer_t * buffer_free;
	ck_ring_buffer_t * buffer_used;
	unsigned msg_size;
	char * msg_mem;
};

void eg_cklog_create(struct eg_cklog * context, unsigned queue_size, unsigned msg_size);

bool eg_cklog_enqueue(struct eg_cklog * context, char const * msg);
void eg_cklog_consume(struct eg_cklog * context);
