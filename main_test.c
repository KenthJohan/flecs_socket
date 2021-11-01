#include <stdio.h>
#include "flecs_socket.h"

void test_server()
{
	ecs_fd_t s = ecs_fd_bind("tcp://localhost:9000");
	ecs_fd_listen(s);
	printf("Server fd: %i\n", s);

	ecs_sockaddr_t addr;
	ecs_fd_t c;
	c = ecs_fd_accept(s, &addr);
	printf("Client fd: %i\n", c);
}



int main(int argc, char *argv[])
{
	ecs_fd_init();
	test_server();
	return 0;
}
