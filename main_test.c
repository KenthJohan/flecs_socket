#include <stdio.h>
#include "flecs_socket.h"

void test_server()
{
	ecs_fd_t s = ecs_fd_listen(NULL, 9000);
	printf("server %i\n", s);

	ecs_sockaddr_t addr;
	ecs_fd_t c;
	c = ecs_fd_accept(s, &addr);
	printf("client %i\n", c);
	c = ecs_fd_accept(s, &addr);
	printf("client %i\n", c);
	c = ecs_fd_accept(s, &addr);
	printf("client %i\n", c);
}



int main(int argc, char *argv[])
{
	ecs_fd_init();
	test_server();
	return 0;
}
