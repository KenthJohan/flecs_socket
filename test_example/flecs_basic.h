#pragma once
#include "flecs.h"

typedef struct
{
	void * memory;
	ecs_u32_t size;
} EgBuffer;


typedef struct
{
	ecs_os_thread_t thread;
	void * arg;
	int status;
} EgThread;

extern ECS_COMPONENT_DECLARE(EgBuffer);
extern ECS_COMPONENT_DECLARE(EgThread);


void FlecsComponentsBasicImport(ecs_world_t *world);
