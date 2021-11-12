#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <uv.h>
#include <flecs.h>

#include "flecs_uv.h"


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

ecs_world_t *world;





int main(int argc, char * argv[])
{
	main_init();
	world = ecs_init_w_args(argc, argv);
	ecs_log_set_level(1);
	flecs_uv_init(world);
	comps_init(world);

	/*
	ecs_entity_t ws_prefab = ecs_new_prefab(world, "WebSocket Prefab");
	ecs_set(world, ws_prefab, EgWS, {50});
	ecs_set(world, ws_prefab, EgReqHTTP, {50});
	//ecs_set_override(world, ws_prefab, EgWebsockMeta, {55});

	ecs_entity_t e = ecs_set_name(world, 0, "MyEntity");
	ecs_add(world, e, EgSocket);
	ecs_add(world, e, EgTCP);
	ecs_set(world, e, EgPort, {8080});
	ecs_set(world, e, EgMaxconn, {8});
	ecs_set(world, e, EgAcceptThread, {ws_prefab});
	*/

	ecs_entity_t e = ecs_new_entity(world, "loop");
	ecs_add(world, e, uv_loop_t);

	ecs_entity_t e1 = ecs_new_entity(world, "tcp_server");
	ecs_add(world, e1, uv_tcp_t);
	ecs_add_pair(world, e1, EcsChildOf, e);



	int i = 0;
	while(1)
	{
		ecs_progress(world, 0.0f);
		ecs_os_sleep(1, 0);
		//uv_run(loop, UV_RUN_NOWAIT);
		printf("main loop %i\n", i++);
	}






	return 0;
}
