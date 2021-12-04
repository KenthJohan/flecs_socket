#pragma once
#include "flecs.h"


#undef ECS_META_IMPL
#ifndef FLECS_COMPONENTS_SOCKET_IMPL
#define ECS_META_IMPL EXTERN // Ensure meta symbols are only defined once
#endif



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
