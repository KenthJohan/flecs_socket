#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <flecs.h>


#include "eg_basic.h"
#include "eg_socket.h"
#include "eg_geometry.h"
#include "eg_log.h"
#include "eg_thread.h"




//https://www.flecs.dev/explorer/?remote=true

void main_init()
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode (hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode (hOut, dwMode);
#endif
}


// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	main_init();
	ecs_log_set_level(0);
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});


	ECS_IMPORT(world, FlecsComponentsEgLog);
	//ECS_IMPORT(world, FlecsComponentsBasic);
	//ECS_IMPORT(world, FlecsComponentsEgThread);
	//ECS_IMPORT(world, FlecsComponentsBasic);
	//ECS_IMPORT(world, FlecsComponentsSocket);
	//ECS_IMPORT(world, FlecsComponentsGeometry);
	//ECS_IMPORT(world, FlecsComponentsNet);

	/*

	{
		//ecs_id_t i1 = ecs_id(EgURL);
		//ecs_id_t i2 = ecs_id(EgURL);
		//ecs_id_t i3 = ecs_id(EgBuffer);
		//ecs_id_t i4 = ecs_id(EgThread);
		//ecs_entity_t e1 = ecs_new(world, EgURL);
		//ecs_entity_t e2 = ecs_new(world, EgURL);
		//ecs_entity_t e4 = ecs_new(world, EgBuffer);
		//ecs_set(world, e2, EgURL, {"tcp://localhost:3000"});
	}

	*/

	/*
	{
		ecs_entity_t e = ecs_new(world, 0);
		ecs_set(world, e, EgURL, {"udp://localhost:3000"});
		ecs_add(world, e, EgNetProtocol1);
		ecs_set(world, e, EgState, {EG_STATE_INITIAL, NULL});
		ecs_set(world, e, EgAction, {EG_ACTION_NOP, NULL});
		//EgThread * tr = ecs_get_mut(world, e, EgThread, NULL);
		//ecs_trace("TR: %p", tr);
	}
	*/

	/*
	return ecs_app_run(world, &(ecs_app_desc_t) {
	.target_fps = 60, .enable_rest = true
	});
	*/

	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_trace("Testing %p", world);
	ecs_set(world, EcsWorld, EcsRest, {0});
	while (1)
	{
		ecs_progress(world, 0);
	}

	return 0;
}
