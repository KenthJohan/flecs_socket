#pragma once
#include "flecs.h"






typedef struct
{
	ecs_u32_t state;
	ecs_u32_t action;
	ecs_entity_t * e;
	ecs_entity_t * c;
	ecs_size_t * s;
	struct eg_we * we;
} EgNetProtocol1;


static void EgNetProtocol1_malloc(EgNetProtocol1 * a)
{
	a->e = ecs_os_calloc(sizeof(ecs_entity_t)*256);
	a->c = ecs_os_calloc(sizeof(ecs_entity_t)*256);
	a->s = ecs_os_calloc(sizeof(ecs_entity_t)*256);
}

extern ECS_COMPONENT_DECLARE(EgNetProtocol1);

void FlecsComponentsNetImport(ecs_world_t *world);
