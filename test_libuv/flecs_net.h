#pragma once

#include <flecs.h>
#include <uv.h>


typedef char * IpAddr;
typedef int Port;
typedef struct sockaddr_storage sockaddr_storage;

extern ECS_COMPONENT_DECLARE(sockaddr_storage);
extern ECS_COMPONENT_DECLARE(IpAddr);
extern ECS_COMPONENT_DECLARE(Port);


void FlecsNetImport(ecs_world_t *world);
