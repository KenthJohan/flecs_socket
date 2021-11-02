#include <stdio.h>
#include <fcntl.h>
#include "flecs_socket.h"

void test_server()
{
	//ecs_fd_t fd_file = ecs_fd_path("/c/hello/hello.txt", O_CREAT);
	ecs_fd_t fd_file = ecs_fd_path("/C/Users/Johan/Documents/flecs_socket/hello.txt", O_CREAT);
	ecs_fd_t fd_tcpsock = ecs_fd_path("tcp://localhost:9000", 0);
	ecs_fd_t fd_udpsock = ecs_fd_path("udp://localhost:9000", 0);
	ecs_socket_connect(fd_udpsock, "udp://localhost:9001"); //read and write in udp://localhost:9001
	ecs_socket_listen(fd_tcpsock);
	printf("Server fd: %i\n", fd_tcpsock);

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
	test_server();
	return 0;
}
