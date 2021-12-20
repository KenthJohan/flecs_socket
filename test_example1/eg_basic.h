#pragma once
#include "flecs.h"

enum eg_action
{
EG_ACTION_NOP,
EG_ACTION_START,
EG_ACTION_RUN,
EG_ACTION_EXIT,
EG_ACTION_STOP,
EG_ACTION_PAUSE,
};

enum eg_state
{
EG_STATE_INITIAL,
EG_STATE_STOPPED,
EG_STATE_RUNNING,
EG_STATE_PAUSED,
EG_STATE_EXITING,
EG_STATE_PENDING,
};


static char const * eg_state_str[] =
{
"EG_STATE_INITIAL",
"EG_STATE_STOPPED",
"EG_STATE_RUNNING",
"EG_STATE_PAUSED",
"EG_STATE_EXITING",
"EG_STATE_PENDING",
};

static char const * eg_action_str[] =
{
"EG_ACTION_NOP",
"EG_ACTION_START",
"EG_ACTION_RUN",
"EG_ACTION_EXIT",
"EG_ACTION_STOP",
"EG_ACTION_PAUSE",
};





struct eg_we
{
	ecs_world_t * world;
	ecs_entity_t entity;
	ecs_u32_t action;
	ecs_u32_t state;
};


typedef struct
{
	void * memory;
	ecs_u32_t size;
} EgBuffer;





typedef struct
{
	ecs_u32_t state;
	char const * text;
} EgState;

typedef struct
{
	ecs_u32_t action;
	char const * text;
} EgAction;


extern ECS_COMPONENT_DECLARE(EgBuffer);
extern ECS_COMPONENT_DECLARE(EgState);
extern ECS_COMPONENT_DECLARE(EgAction);


void FlecsComponentsBasicImport(ecs_world_t *world);
