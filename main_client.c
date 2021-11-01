#include <stdio.h>
#include "flecs_socket.h"

void test_client()
{
	ecs_fd_t client = ecs_fd_bind("tcp://127.0.0.1:9001");
	ecs_fd_connect(client, "tcp://127.0.0.1:9000");
	printf("Client fd: %i\n", client);
}



int main(int argc, char *argv[])
{
	ecs_fd_init();
	test_client();
	return 0;
}
