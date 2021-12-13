#pragma once
#include "flecs.h"


typedef struct
{
	int a;
} EgNetProtocol1;


extern ECS_COMPONENT_DECLARE(EgNetProtocol1);

void FlecsComponentsNetImport(ecs_world_t *world);
