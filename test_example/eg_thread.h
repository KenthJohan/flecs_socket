#pragma once
#include "flecs.h"

enum eg_thread_state_t
{
	eg_thread_status_undefined, //Controlled by outside
	eg_thread_status_empty,
	eg_thread_status_standby, //Controlled by thread or outside
	eg_thread_status_starting, //Controlled by outside
	eg_thread_status_running, //Controlled by thread
	eg_thread_status_stopping //Controlled by outside
};

typedef struct
{
	ecs_u32_t state;
	ecs_os_thread_t thread;
	ecs_world_t * world;
	ecs_entity_t entity;
	ecs_os_thread_callback_t callback;
} eg_callback_arg_t;

typedef struct
{
	ecs_string_t debug_text;
	eg_callback_arg_t * arg;
} EgThread;


extern ECS_COMPONENT_DECLARE(EgThread);


void eg_thread_set_callback(EgThread * t, ecs_os_thread_callback_t callback);
void eg_thread_action(EgThread * t, uint32_t state);

void FlecsComponentsThreadImport(ecs_world_t *world);
