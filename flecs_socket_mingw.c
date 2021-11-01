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


void ecs_fd_printerror(char * filename, int line)
{
	wchar_t buf[256];
	DWORD e = GetLastError();
	DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	FormatMessageW(dwFlags, NULL, e, dwLanguageId, buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
	printf("%s:%i: GetLastError: %S (%li)\n", filename, line, buf, e);
}

#define ECS_FD_PRINTERROR() ecs_fd_printerror(__FILE__, __LINE__)

void ecs_fd_init()
{
	/* If on Windows, test if winsock needs to be initialized */
	SOCKET testsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ((SOCKET_ERROR == (int)testsocket) && (WSANOTINITIALISED == WSAGetLastError()))
	{
		WSADATA data = { 0 };
		int rv = WSAStartup(MAKEWORD(2, 2), &data);
		if (rv){ECS_FD_PRINTERROR();goto error;}
	}
	else
	{
		int rv;
		rv = closesocket(testsocket);
		if (rv){ECS_FD_PRINTERROR();goto error;}
		rv = shutdown(testsocket, SD_BOTH);
		if (rv){ECS_FD_PRINTERROR();goto error;}
		rv = close(testsocket);
		if (rv){ECS_FD_PRINTERROR();goto error;}
	}
	return;
error:
	exit(1);
}


ecs_fd_t ecs_fd_open(const char *pathname, int flags)
{
	ecs_fd_t file;
	file.fd = open(pathname, flags);
	return file;
}


void ecs_fd_url_split(const char *url, char * proto, char * addr, unsigned * port)
{
	sscanf(url, "%[^://]://%[^:]:%u", proto, addr, port);
}


ecs_fd_t ecs_fd_bind(const char *url)
{
	char proto[10] = {0};
	char addr[INET6_ADDRSTRLEN] = {0};
	unsigned port = 0;
	ecs_fd_url_split(url, proto, addr, &port);
	//printf("proto: %s, addr: %s, port: %d\n", proto, addr, port);
	ecs_fd_t server;
	struct sockaddr_in sain =
	{
		.sin_family = AF_INET,
		.sin_port = htons(port),
	};
	if (strcmp(proto, "tcp") == 0)
	{
		int rv;
		server.fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server.fd < 0){ECS_FD_PRINTERROR();goto error;}
		rv = bind(server.fd, (struct sockaddr*)&sain, sizeof(struct sockaddr_in));
		if (rv){ECS_FD_PRINTERROR();goto error;}
	}
	else if (strcmp(proto, "udp") == 0)
	{
		int rv;
		server.fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (server.fd < 0){ECS_FD_PRINTERROR();goto error;}
		rv = bind(server.fd, (struct sockaddr*)&sain, sizeof(struct sockaddr_in));
		if (rv){ECS_FD_PRINTERROR();goto error;}
	}
	else
	{
		printf("Protocol not found\n");
		exit(1);
	}
	return server;
error:
	exit(1);
}



void ecs_fd_connect(ecs_fd_t client, const char *url)
{
	char proto[10] = {0};
	char addr[INET6_ADDRSTRLEN] = {0};
	unsigned port = 0;
	ecs_fd_url_split(url, proto, addr, &port);
	struct sockaddr_in sain =
	{
		.sin_family = AF_INET,
		.sin_port = htons(port),
	};
	sain.sin_addr.s_addr = inet_addr(addr);
	int rv = connect(client.fd, (struct sockaddr *)&sain, sizeof(struct sockaddr_in));
	if (rv)
	{
		ECS_FD_PRINTERROR();
		exit(1);
	}
}

void ecs_fd_listen(ecs_fd_t server)
{
	int rv = listen(server.fd, SOMAXCONN);
	if (rv)
	{
		ECS_FD_PRINTERROR();
		exit(1);
	}
}





ecs_fd_t ecs_fd_accept(ecs_fd_t s, ecs_sockaddr_t *addr)
{
	ecs_fd_t client;
	int len = sizeof(ecs_sockaddr_t);
	client.fd = accept(s.fd, (struct sockaddr*)addr, &len);
	if (client.fd < 0){goto error;}
	return client;
error:
	ECS_FD_PRINTERROR();
	exit(1);
}



void ecs_sockaddr_tostring(ecs_sockaddr_t *addr, ecs_sockaddr_str_t *str)
{
	struct sockaddr * sa = (struct sockaddr *)addr;
	switch(sa->sa_family)
	{
	case AF_INET:
		inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), str->text, sizeof(ecs_sockaddr_str_t));
		break;
	case AF_INET6:
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), str->text, sizeof(ecs_sockaddr_str_t));
		break;
	default:
		//strncpy(s, "Unknown AF", maxlen);
		return;
	}
}


/*
char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
	switch(sa->sa_family) {
	case AF_INET:
		inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
		s, maxlen);
		break;

	case AF_INET6:
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
		s, maxlen);
		break;

	default:
		strncpy(s, "Unknown AF", maxlen);
		return NULL;
	}

	return s;
}
*/








