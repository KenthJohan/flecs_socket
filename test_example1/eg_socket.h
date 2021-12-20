#pragma once
#include "flecs.h"

#define UDP_SAFE_SIZE 508

typedef struct
{
	ecs_u64_t fd;
} EgSocketUDP;

typedef struct
{
	ecs_u64_t fd;
} EgSocketTCP;

typedef struct
{
	ecs_string_t path;
} EgURL;




typedef struct
{
	ecs_os_thread_t thread;
	ecs_sparse_t *connections; /* sparse<http_connection_t> */
} EgReceiver;



extern ECS_COMPONENT_DECLARE(EgSocketUDP);
extern ECS_COMPONENT_DECLARE(EgSocketTCP);
extern ECS_COMPONENT_DECLARE(EgURL);


#define WIN32_PRINT_ERROR(fmt) win32_print_error(__FILE__, __LINE__, fmt)
void win32_print_error(char * filename, int line, char const * fmt);

void FlecsComponentsSocketImport(ecs_world_t *world);
