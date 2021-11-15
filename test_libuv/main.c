// https://discord.com/channels/633826290415435777/633826290415435781/905405276071555092
// https://sandermertens.github.io/query_explorer/?remote=true
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



int main(int argc, char * argv[])
{
	main_init();
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});
	ecs_set(world, EcsWorld, EcsRest, {0});

	ecs_log_set_level(1);


	flecs_uv_init(world);

	ecs_entity_t loop;
	{
		loop = ecs_new_entity(world, "loop");
		ecs_add(world, loop, UvLoop);
	}


	{
		sockaddr_in addr;
		uv_ip4_addr("0.0.0.0", 7000, &addr);
		ecs_entity_t e = ecs_new_entity(world, "My TCP Server");
		ecs_add(world, e, UvTcp);
		ecs_add_pair(world, e, EcsChildOf, loop);
		ecs_set_ptr(world, e, sockaddr_in, &addr);
	}




	//int i = 0;
	//printf("main loop %i.\n", i++);
	while(1)
	{
		ecs_progress(world, 0.0f);
		ecs_os_sleep(0, 1000*1000*100);
		//uv_run(loop, UV_RUN_NOWAIT);
		//printf("main loop %i.\n", i++);
	}


	return 0;
}
