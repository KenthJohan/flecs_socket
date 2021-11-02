#include <stdio.h>
#include <unistd.h>
#include "flecs_socket.h"

void test_client()
{
	ecs_fd_t fd_tcp = ecs_fd_path("tcp://127.0.0.1:9001", 0);
	ecs_fd_t fd_udp = ecs_fd_path("udp://127.0.0.1:9001", 0);
	printf("fd_tcp: %i\n", fd_tcp);
	printf("fd_udp: %i\n", fd_udp);
	ecs_socket_connect(fd_tcp, "tcp://127.0.0.1:9000");
	ecs_socket_connect(fd_udp, "tcp://127.0.0.1:9000");

	while(1)
	{
		int r = 0;
		r = ecs_fd_write(fd_udp, "Hello\n", sizeof("Hello\n"));
		r = ecs_fd_write(fd_tcp, "Hello\n", sizeof("Hello\n"));
		usleep(1000*1000);
	}
}



int main(int argc, char *argv[])
{
	ecs_fd_init();
	test_client();
	return 0;
}
