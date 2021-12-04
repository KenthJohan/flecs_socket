#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <flecs.h>

#include "flecs_socket.h"

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



int main(int argc, char *argv[])
{
	main_init();
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});
	//ecs_set(world, EcsWorld, EcsRest, {0});

	ecs_log_set_level(0);


	ECS_IMPORT(world, FlecsComponentsSocket);

	ecs_entity_t e1 = ecs_new(world, EgURL);
	ecs_entity_t e2 = ecs_new(world, EgUdpSocket);
	ecs_set(world, e1, EgURL, {"udp://localhost:3000"});

	return ecs_app_run(world, &(ecs_app_desc_t) {
	.target_fps = 60, .enable_rest = true
	});
	return 0;
}
