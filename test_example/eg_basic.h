#pragma once
#include "flecs.h"

typedef struct
{
	void * memory;
	ecs_u32_t size;
} EgBuffer;


extern ECS_COMPONENT_DECLARE(EgBuffer);


void FlecsComponentsBasicImport(ecs_world_t *world);
