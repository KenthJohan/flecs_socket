#pragma once
#include "flecs.h"


typedef struct
{
	ecs_u64_t fd;
} EgUdpSocket;

typedef struct
{
	ecs_string_t path;
} EgURL;

extern ECS_COMPONENT_DECLARE(EgUdpSocket);
extern ECS_COMPONENT_DECLARE(EgURL);


void FlecsComponentsSocketImport(ecs_world_t *world);
