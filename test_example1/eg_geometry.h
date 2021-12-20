#pragma once
#include "flecs.h"

typedef struct
{
	ecs_f32_t width;
	ecs_f32_t height;
} EgRectangle;



extern ECS_COMPONENT_DECLARE(EgRectangle);


void FlecsComponentsGeometryImport(ecs_world_t *world);
