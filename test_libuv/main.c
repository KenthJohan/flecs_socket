// https://discord.com/channels/633826290415435777/633826290415435781/905405276071555092
// https://sandermertens.github.io/query_explorer/?remote=true
// https://www.flecs.dev/explorer/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <uv.h>
#include <flecs.h>

#include "flecs_uv.h"
#include "flecs_net.h"


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



int main(int argc, char * argv[])
{
	main_init();
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});
	ecs_set(world, EcsWorld, EcsRest, {0});

	ecs_log_set_level(0);

	FlecsNetImport(world);
	flecs_uv_init(world);

	ecs_entity_t loop;
	{
		loop = ecs_new_entity(world, "loop");
		ecs_add(world, loop, UvLoop);
	}


	{
		ecs_entity_t e = ecs_new_entity(world, "My TCP Server");
		ecs_set(world, e, IpAddr, {"0.0.0.0"});
		ecs_set(world, e, Port, {7000});
		ecs_add(world, e, sockaddr_storage);
		ecs_add(world, e, UvTcp);
		ecs_add_pair(world, e, EcsChildOf, loop);
	}

	{
		ecs_entity_t e = ecs_new_entity(world, "My UDP Server");
		ecs_set(world, e, IpAddr, {"0.0.0.0"});
		ecs_set(world, e, Port, {7000});
		ecs_add(world, e, sockaddr_storage);
		ecs_add(world, e, UvUdp);
		ecs_add_pair(world, e, EcsChildOf, loop);
	}

	/*
	{
		ecs_entity_t e = ecs_new_entity(world, "My TCP Server2");
		ecs_add(world, e, UvTcp);
	}
	*/


	//int i = 0;
	//printf("main loop %i.\n", i++);
	while(1)
	{
		ecs_progress(world, 0.0f);
		ecs_os_sleep(0, 1000*1000*100);//100ms sleep
		//uv_run(loop, UV_RUN_NOWAIT);
		//printf("main loop %i.\n", i++);
	}


	return 0;
}
