#pragma once
#include <flecs.h>
#include <uv.h>

typedef struct
{
	int a;
	int b;
	int c;
} TestComponent;


extern ECS_COMPONENT_DECLARE(uv_loop_t);
extern ECS_COMPONENT_DECLARE(uv_tcp_t);
extern ECS_COMPONENT_DECLARE(uv_stream_t);
typedef struct sockaddr_in sockaddr_in;
extern ECS_COMPONENT_DECLARE(sockaddr_in);
extern ECS_COMPONENT_DECLARE(TestComponent);

//extern ECS_DECLARE(MyTag);

void flecs_uv_init(ecs_world_t *world);
