#pragma once

#include <flecs.h>
#include <stdio.h>


typedef struct {
	int sock;
} EgSocketTCP;

typedef struct {
	int port;
} EgSocketPort;

typedef struct {
	int backlog;
} EgSocketMaxconn;

typedef struct {
	ecs_world_t *world;
	ecs_entity_t entity;
	int sock;
} EgSocketAcceptThread;

typedef struct {
	int meta;
} EgWebsockMeta;




typedef struct {
	int meta;
} EgAccept;


extern ECS_COMPONENT_DECLARE(EgSocketTCP);
extern ECS_COMPONENT_DECLARE(EgSocketPort);
extern ECS_COMPONENT_DECLARE(EgSocketMaxconn);
extern ECS_COMPONENT_DECLARE(EgSocketAcceptThread);
extern ECS_COMPONENT_DECLARE(EgWebsockMeta);



void eg_ws_init();
void ws_flecs_init(ecs_world_t *world);
