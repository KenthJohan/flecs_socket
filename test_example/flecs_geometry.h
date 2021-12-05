#pragma once
#include "flecs.h"

typedef struct
{
	ecs_f32_t width;
	ecs_f32_t height;
} EgRectangle;


typedef struct
{
	void * memory;
	ecs_u32_t size;
} EgBuffer;

extern ECS_COMPONENT_DECLARE(EgRectangle);
extern ECS_COMPONENT_DECLARE(EgBuffer);


void FlecsComponentsGeometryImport(ecs_world_t *world);
