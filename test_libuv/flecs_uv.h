#pragma once
#include <flecs.h>
#include <uv.h>
#include "circularbuf.h"


struct uv_tcp_ecs
{
	uv_tcp_t uvtcp;
	ecs_world_t * world;
	ecs_entity_t entity;
	struct circular_buf buf;
};

struct uv_udp_ecs
{
	uv_udp_t uvtcp;
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
	struct uv_udp_ecs * stream;
} UvUdp;








extern ECS_COMPONENT_DECLARE(UvLoop);
extern ECS_COMPONENT_DECLARE(UvTcp);
extern ECS_COMPONENT_DECLARE(UvUdp);
extern ECS_COMPONENT_DECLARE(UvStream);

//extern ECS_DECLARE(MyTag);

void flecs_uv_init(ecs_world_t *world);
