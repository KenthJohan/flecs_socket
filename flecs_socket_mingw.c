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
#include <ctype.h>
#include <errno.h>


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


ecs_fd_t ecs_fd_open(const char *filepath, int flags)
{
	char windowspath[500];
	if(filepath[0] == '/' && isalpha(filepath[1]) && filepath[2] == '/')
	{
		windowspath[0] = filepath[1];
		windowspath[1] = ':';
		strcpy(windowspath+2, filepath+2);
	}
	ecs_fd_t file;
	file.fd = open(windowspath, flags);
	if(file.fd < 0)
	{
		ECS_FD_PRINTERROR();
		exit(1);
	}
	return file;
}


void ecs_url_split(const char *url, char * proto, char * addr, unsigned * port)
{
	sscanf(url, "%[^://]://%[^:]:%u", proto, addr, port);
}


ecs_fd_t ecs_socket_bind(const char *url, int flags)
{
	char proto[10] = {0};
	char addr[INET6_ADDRSTRLEN] = {0};
	unsigned port = 0;
	ecs_url_split(url, proto, addr, &port);
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


ecs_fd_t ecs_fd_path(const char *path, int flags)
{
	if (strstr(path, "://"))
	{
		ecs_socket_bind(path, flags);
	}
	else
	{
		return ecs_fd_open(path, flags);
	}
}


void ecs_socket_connect(ecs_fd_t client, const char *url)
{
	char proto[10] = {0};
	char addr[INET6_ADDRSTRLEN] = {0};
	unsigned port = 0;
	ecs_url_split(url, proto, addr, &port);
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

void ecs_socket_listen(ecs_fd_t server)
{
	int rv = listen(server.fd, SOMAXCONN);
	if (rv)
	{
		ECS_FD_PRINTERROR();
		exit(1);
	}
}





ecs_fd_t ecs_socket_accept(ecs_fd_t s, ecs_sockaddr_t *addr)
{
	ecs_fd_t client;
	int len = sizeof(ecs_sockaddr_t);
	client.fd = accept(s.fd, (struct sockaddr*)addr, &len);
	if (client.fd < 0)
	{
		ECS_FD_PRINTERROR();
		exit(1);
	}
	return client;
}



void ecs_sockaddr_to_string(ecs_sockaddr_t *addr, char *str, int length)
{
	struct sockaddr * sa = (struct sockaddr *)addr;
	switch(sa->sa_family)
	{
	case AF_INET:
		inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), str, length);
		break;
	case AF_INET6:
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), str, length);
		break;
	default:
		//strncpy(s, "Unknown AF", maxlen);
		return;
	}
}





int ecs_fd_read(ecs_fd_t fd, char *data, int size)
{
	int rv = recv(fd.fd, data, size, 0);
	printf("read(%i):%i\n", fd, rv);
	//int rv = read(fd.fd, data, size);
	if (rv < 0)
	{
		ECS_FD_PRINTERROR();
		perror("read");
		//ECS_FD_PRINTERROR();
		//exit(1);
	}
	return rv;
}


int ecs_fd_write(ecs_fd_t fd, char *data, int size)
{
	int rv = send(fd.fd, data, size, 0);
	printf("write(%i):%i\n", fd, rv);
	//int rv = write(fd.fd, data, size);
	if (rv < 0)
	{
		perror("write");
		//ECS_FD_PRINTERROR();
		//exit(1);
	}
	return rv;
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








