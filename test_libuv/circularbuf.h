#pragma once
#include <flecs.h>
#include <uv.h>
#include <stdint.h>
#include <assert.h>


#define CIRCULAR_BUF_SIZE 65536
struct circular_buf
{
	uint8_t * buffer;
	uint16_t head;
	uint16_t tail;
};


static void circular_buf_init(struct circular_buf * b)
{
	b->buffer = malloc(CIRCULAR_BUF_SIZE);
	b->head = 0;
	b->tail = 0;
}


static void * circular_buf_head(struct circular_buf * b)
{
	return b->buffer + b->head;
}


static uint16_t circular_buf_size(struct circular_buf * b)
{
	return b->head - b->tail;
}

static uint32_t circular_buf_freespace(struct circular_buf * b)
{
	uint16_t size = circular_buf_size(b);
	uint32_t freespace = CIRCULAR_BUF_SIZE - size;
	if(b->buffer == NULL)
	{
		assert(b->buffer);
	}
	return freespace;
}


static void test_circular_buf()
{
	struct circular_buf buf;
	circular_buf_init(&buf);
	assert(circular_buf_size(&buf) == 0);
	assert(circular_buf_freespace(&buf) == CIRCULAR_BUF_SIZE);
	buf.head += 100;
	assert(circular_buf_size(&buf) == (100));
	assert(circular_buf_freespace(&buf) == (CIRCULAR_BUF_SIZE-100));
	buf.head = CIRCULAR_BUF_SIZE-111;
	buf.tail = CIRCULAR_BUF_SIZE-111;
	buf.head += 1339;
	assert(circular_buf_size(&buf) == (1339));
	assert(circular_buf_freespace(&buf) == (CIRCULAR_BUF_SIZE-1339));
	free(buf.buffer);
}



