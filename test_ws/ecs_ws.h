#pragma once

#include <flecs.h>
#include <stdio.h>


typedef struct {
	int sock;
} EgSocket;

typedef struct {
	int unused;
} EgTCP;

typedef struct {
	int unused;
} EgUDP;

typedef struct {
	int unused;
} EgWS;

typedef struct {
	int port;
} EgPort;

typedef struct {
	ecs_strbuf_t content;
} EgReqHTTP;

typedef struct {
	int backlog;
} EgMaxconn;

typedef struct {
	ecs_entity_t prefab;
} EgAcceptThread;




extern ECS_COMPONENT_DECLARE(EgSocket);
extern ECS_COMPONENT_DECLARE(EgTCP);
extern ECS_COMPONENT_DECLARE(EgUDP);
extern ECS_COMPONENT_DECLARE(EgPort);
extern ECS_COMPONENT_DECLARE(EgMaxconn);
extern ECS_COMPONENT_DECLARE(EgAcceptThread);
extern ECS_COMPONENT_DECLARE(EgWS);
extern ECS_COMPONENT_DECLARE(EgReqHTTP);



void eg_ws_init();
void ws_flecs_init(ecs_world_t *world);
