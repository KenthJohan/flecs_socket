#define FLECS_COMPONENTS_SOCKET_IMPL
#include "flecs_socket.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>


ECS_COMPONENT_DECLARE(EgUdpSocket);
ECS_COMPONENT_DECLARE(EgURL);




ECS_CTOR(EgUdpSocket, ptr, {
ecs_trace("EgUdpSocket::ECS_CTOR");
ptr->fd = INVALID_SOCKET;
});

ECS_DTOR(EgUdpSocket, ptr, {
ecs_trace("EgUdpSocket::ECS_DTOR");
if (ptr->fd != INVALID_SOCKET)
{
closesocket(ptr->fd);
}
});



ECS_CTOR(EgURL, ptr, {
ecs_trace("EgAddress::ECS_CTOR");
ptr->path = NULL;
});

ECS_DTOR(EgURL, ptr, {
ecs_trace("EgAddress::ECS_DTOR");
if(ptr->path)
{
ecs_os_free(ptr->path);
}
});

ECS_MOVE(EgURL, dst, src, {
ecs_trace("EgAddress::ECS_MOVE");
dst->path = src->path;
src->path = NULL;
})

ECS_COPY(EgURL, dst, src, {
ecs_trace("EgAddress::ECS_COPY");
ecs_os_strset(&dst->path, src->path);
});


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
void win32_sock_init()
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



void url_split(const char *url, char * proto, char * addr, unsigned * port)
{
	sscanf(url, "%[^://]://%[^:]:%u", proto, addr, port);
}


void EgURLTrigger(ecs_iter_t *it)
{
	EgURL *a = ecs_term(it, EgURL, 1);
	for (int i = 0; i < it->count; i ++)
	{
		char proto[10] = {0};
		char addr[INET6_ADDRSTRLEN] = {0};
		unsigned port = 0;
		url_split(a->path, proto, addr, &port);
		ecs_trace("proto:%s, addr:%s, port:%i", proto, addr, port);
		if (ecs_os_strcmp(proto, "udp") == 0)
		{
			ecs_add(it->world, it->entities[i], EgUdpSocket);
		}
	}
}


void EgCreateUDPSocket(ecs_iter_t *it)
{
	EgURL *a = ecs_term(it, EgURL, 1);
	EgUdpSocket *s = ecs_term(it, EgUdpSocket, 2);
	for (int i = 0; i < it->count; i ++)
	{
		SOCKET s1 = socket(AF_INET, SOCK_DGRAM, 0);
		s[i].fd = s1;
	}
}



void FlecsComponentsSocketImport(ecs_world_t *world)
{
	win32_sock_init();
	ECS_MODULE(world, FlecsComponentsSocket);
	ECS_COMPONENT_DEFINE(world, EgUdpSocket);
	ECS_COMPONENT_DEFINE(world, EgURL);

	ecs_trace("Hej");
	ecs_set_name_prefix(world, "Eg");


	ecs_set_component_actions(world, EgUdpSocket, {
	.ctor = ecs_ctor(EgUdpSocket),
	.dtor = ecs_dtor(EgUdpSocket),
	});


	ecs_set_component_actions(world, EgURL, {
	.ctor = ecs_ctor(EgURL),
	.dtor = ecs_dtor(EgURL),
	.copy = ecs_copy(EgURL),
	.move = ecs_move(EgURL),
	});


	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgUdpSocket), // Make sure to use existing id
	.members = {
	{ .name = "fd", .type = ecs_id(ecs_u64_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgURL), // Make sure to use existing id
	.members = {
	{ .name = "path", .type = ecs_id(ecs_string_t) }
	}
	});



	ECS_TRIGGER(world, EgURLTrigger, EcsOnSet, EgURL);
	ECS_OBSERVER(world, EgCreateUDPSocket, EcsOnAdd, EgURL, EgUdpSocket);



}

