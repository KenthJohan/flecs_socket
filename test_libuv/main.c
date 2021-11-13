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
	ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_log_set_level(1);
	flecs_uv_init(world);

	ecs_entity_t loop;
	{
		loop = ecs_new_entity(world, "loop");
		ecs_add(world, loop, uv_loop_t);
	}


	{
		sockaddr_in addr;
		uv_ip4_addr("0.0.0.0", 7000, &addr);
		ecs_entity_t e = ecs_new_entity(world, "tcp_server");
		ecs_add(world, e, uv_tcp_t);
		ecs_add_pair(world, e, EcsChildOf, loop);
		ecs_set_ptr(world, e, sockaddr_in, &addr);
	}



	int i = 0;
	printf("main loop %i.\n", i++);
	while(1)
	{
		ecs_progress(world, 0.0f);
		ecs_os_sleep(1, 0);
		//uv_run(loop, UV_RUN_NOWAIT);
		printf("main loop %i.\n", i++);
	}


	return 0;
}
