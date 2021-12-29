#pragma once
#include "flecs.h"


typedef struct
{
	char const * value;
} EgMsg;

typedef struct
{
	char const * value;
} EgPath;

typedef struct
{
	ecs_i32_t value;
} EgLine;

typedef struct
{
	ecs_i32_t value;
} EgLevel;


extern ECS_COMPONENT_DECLARE(EgMsg);
extern ECS_COMPONENT_DECLARE(EgLine);
extern ECS_COMPONENT_DECLARE(EgPath);
extern ECS_COMPONENT_DECLARE(EgLevel);
extern ECS_DECLARE(EgInfo);
extern ECS_DECLARE(EgWarning);
extern ECS_DECLARE(EgError);
extern ECS_DECLARE(EgFatal);

void FlecsComponentsEgLogImport(ecs_world_t *world);
