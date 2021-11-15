#pragma once
#include <flecs.h>
#include <uv.h>

typedef struct
{
	int a;
	int b;
	int c;
} TestComponent;


typedef struct
{
	uv_loop_t * loop;
} UvLoop;

typedef struct
{
	uv_tcp_t * stream;
} UvTcp;

typedef struct
{
	uv_stream_t * stream;
} UvStream;








extern ECS_COMPONENT_DECLARE(UvLoop);
extern ECS_COMPONENT_DECLARE(UvTcp);
extern ECS_COMPONENT_DECLARE(UvStream);
extern ECS_COMPONENT_DECLARE(uv_buf_t);
typedef struct sockaddr_in sockaddr_in;
extern ECS_COMPONENT_DECLARE(sockaddr_in);
extern ECS_COMPONENT_DECLARE(TestComponent);

//extern ECS_DECLARE(MyTag);

void flecs_uv_init(ecs_world_t *world);
