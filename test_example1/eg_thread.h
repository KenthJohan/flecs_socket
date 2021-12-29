#pragma once
#include "flecs.h"



typedef struct
{
	ecs_world_t * world;
	ecs_entity_t entity;
	ecs_os_thread_t thread;
} EgThread;


extern ECS_COMPONENT_DECLARE(EgThread);


void FlecsComponentsEgThreadImport(ecs_world_t *world);
