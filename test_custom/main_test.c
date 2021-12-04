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


void test_server()
{
	//ecs_fd_t fd_file = ecs_fd_path("/c/hello/hello.txt", O_CREAT);
	//ecs_fd_t fd_file = ecs_fd_path("/C/Users/Johan/Documents/flecs_socket/hello.txt", O_CREAT);
	ecs_fd_t fd_tcpsock = ecs_fd_path("tcp://localhost:9000", 0);
	ecs_fd_t fd_udpsock = ecs_fd_path("udp://127.0.0.1:9000", 0);
	ecs_socket_connect(fd_udpsock, "udp://127.0.0.1:9001"); //read and write in udp://localhost:9001
	ecs_socket_listen(fd_tcpsock);
	printf("fd_tcpsock: %i\n", fd_tcpsock);
	printf("fd_udpsock: %i\n", fd_tcpsock);

	while(1)
	{
		ecs_sockaddr_t addr;
		char addr_text[128];
		ecs_fd_t fd_tcpsock_client = ecs_socket_accept(fd_tcpsock, &addr);
		ecs_sockaddr_to_string(&addr, addr_text, 128);
		printf("\n=============\nAccepted: %i %s\n", fd_tcpsock_client, addr_text);

		char buf[100];
		ecs_fd_read(fd_tcpsock_client, buf, 100);
		printf("fd_tcpsock_client: %s\n", buf);
		//ecs_fd_write(fd_file, buf, 100);
		ecs_fd_read(fd_udpsock, buf, 100);
		printf("fd_udpsock: %s\n", buf);
		//ecs_fd_write(fd_file, buf, 100);
	}
}



int main(int argc, char *argv[])
{
	ecs_fd_init();
	main_init();
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});
	ecs_set(world, EcsWorld, EcsRest, {0});

	ecs_log_set_level(0);


	test_server();
	return 0;
}
