#include "flecs_socket.h"

#if defined WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>

void test()
{
	SOCKET s;
	char buf[100];
	send(s, buf, 100, MSG_OOB);
}


void socket_connect()
{
	//ecs_fd_t s;
	//s.socket = socket();
}



void ecs_fd_init()
{
	/* If on Windows, test if winsock needs to be initialized */
	SOCKET testsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == testsocket && WSANOTINITIALISED == WSAGetLastError())
	{
		WSADATA data = { 0 };
		int result = WSAStartup(MAKEWORD(2, 2), &data);
		if (result)
		{
			printf ("GetLastError(): %d", GetLastError());
		}
	}
	else
	{
		closesocket(testsocket);
		shutdown(testsocket, SD_BOTH);
		close(testsocket);
	}
}



ecs_fd_t ecs_fd_open(const char *pathname, int flags)
{
	ecs_fd_t file;
	file.fd = open(pathname, flags);
	return file;
}


ecs_fd_t ecs_fd_listen(const char *ipaddr, uint16_t port)
{
	ecs_fd_t server;
	struct sockaddr_in addr =
	{
		.sin_family = AF_INET,
		.sin_port = htons(port),
	};
	if (ipaddr)
	{
		inet_pton(AF_INET, ipaddr, &(addr.sin_addr));
	}
	else
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	server.fd = s;
	u_long iMode = 0;
	int iResult;
	iResult = ioctlsocket(server.fd, FIONBIO, &iMode);
	printf("ioctlsocket iResult: %i\n", iResult);
	bind(server.fd, (struct sockaddr*)&addr, sizeof(addr));
	listen(server.fd, SOMAXCONN);
}


ecs_fd_t ecs_fd_generic(char * path)
{

}



void ecs_fd_printerror()
{
	wchar_t buf[256];
	DWORD a = GetLastError();
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	NULL, a, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
	printf("GetLastError: %s (%d)\n", buf, a);
}


ecs_fd_t ecs_fd_accept(ecs_fd_t s, ecs_sockaddr_t *addr)
{
	ecs_fd_t client;
	int len = sizeof(ecs_sockaddr_t);
	client.fd = accept(s.fd, (struct sockaddr*)addr, &len);
	if (client.fd < 0)
	{
		ecs_fd_printerror();
	}
	printf("accept len: %i\n", len);
	return client;
}












