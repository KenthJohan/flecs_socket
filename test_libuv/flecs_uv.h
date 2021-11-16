#pragma once
#include <flecs.h>
#include <uv.h>

typedef struct
{
	int a;
	int b;
	int c;
} TestComponent;


struct uv_tcp_ecs
{
	uv_tcp_t uvtcp;
	ecs_world_t * world;
	ecs_entity_t entity;
};


struct uv_loop_ecs
{
	uv_loop_t uvloop;
	ecs_world_t * world;
	ecs_entity_t entity;
};


typedef struct
{
	struct uv_loop_ecs * loop;
} UvLoop;

typedef struct
{
	struct uv_tcp_ecs * stream;
} UvTcp;

typedef struct
{
	uv_stream_t * stream;
} UvStream;








extern ECS_COMPONENT_DECLARE(UvLoop);
extern ECS_COMPONENT_DECLARE(UvTcp);
extern ECS_COMPONENT_DECLARE(UvStream);
extern ECS_COMPONENT_DECLARE(uv_buf_t);
extern ECS_COMPONENT_DECLARE(TestComponent);

//extern ECS_DECLARE(MyTag);

void flecs_uv_init(ecs_world_t *world);
