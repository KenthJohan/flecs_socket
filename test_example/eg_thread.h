#pragma once
#include "flecs.h"


typedef struct
{
	ecs_u32_t state;
	ecs_os_thread_t thread;
	ecs_world_t * world;
	ecs_entity_t entity;
	ecs_os_thread_callback_t callback;
} eg_callback_arg_t;

typedef struct
{
	ecs_u32_t state;
	eg_callback_arg_t * arg;
} EgThread;


extern ECS_COMPONENT_DECLARE(EgThread);


void eg_thread_start(ecs_world_t * world, ecs_entity_t e, ecs_os_thread_callback_t callback);



void FlecsComponentsThreadImport(ecs_world_t *world);
