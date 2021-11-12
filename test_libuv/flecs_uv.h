#pragma once
#include <flecs.h>
#include <uv.h>
#include "comps.h"



extern ECS_COMPONENT_DECLARE(uv_loop_t);
extern ECS_COMPONENT_DECLARE(uv_tcp_t);
extern ECS_COMPONENT_DECLARE(uv_stream_t);

//extern ECS_DECLARE(MyTag);

void flecs_uv_init(ecs_world_t *world);
