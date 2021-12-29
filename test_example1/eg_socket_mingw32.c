#include "eg_socket.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>

#include "eg_basic.h"

ECS_COMPONENT_DECLARE(EgSocketUDP);
ECS_COMPONENT_DECLARE(EgSocketTCP);
ECS_COMPONENT_DECLARE(EgURL);




ECS_CTOR(EgSocketUDP, ptr, {
//ecs_trace("EgUdpSocket::ECS_CTOR");
ptr->fd = INVALID_SOCKET;
});

ECS_DTOR(EgSocketUDP, ptr, {
//ecs_trace("EgUdpSocket::ECS_DTOR");
if (ptr->fd != INVALID_SOCKET){closesocket(ptr->fd);}
});

ECS_CTOR(EgSocketTCP, ptr, {
//ecs_trace("EgSocketTCP::ECS_CTOR");
ptr->fd = INVALID_SOCKET;
});

ECS_DTOR(EgSocketTCP, ptr, {
//ecs_trace("EgSocketTCP::ECS_DTOR");
if (ptr->fd != INVALID_SOCKET){closesocket(ptr->fd);}
});

ECS_CTOR(EgURL, ptr, {
//ecs_trace("EgURL::ECS_CTOR");
ptr->path = NULL;
});

ECS_DTOR(EgURL, ptr, {
//ecs_trace("EgURL::ECS_DTOR");
if(ptr->path)
{
ecs_os_free(ptr->path);
}
});

ECS_MOVE(EgURL, dst, src, {
//ecs_trace("EgURL::ECS_MOVE");
dst->path = src->path;
src->path = NULL;
})

ECS_COPY(EgURL, dst, src, {
//ecs_trace("EgURL::ECS_COPY");
ecs_os_strset(&dst->path, src->path);
});










void win32_print_error(char * filename, int line, char const * fmt)
{
	wchar_t lpBuffer[256];
	DWORD dwMessageId = GetLastError();
	LPCVOID lpSource = NULL;
	DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	DWORD nSize = (sizeof(lpBuffer) / sizeof(wchar_t));
	va_list *Arguments = NULL;
	FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
	//ecs_trace("%s:%i: GetLastError: %S (%li): %s\n", filename, line, lpBuffer, dwMessageId, fmt);
}


void win32_sock_init()
{
	/* If on Windows, test if winsock needs to be initialized */
	SOCKET testsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ((SOCKET_ERROR == (int)testsocket) && (WSANOTINITIALISED == WSAGetLastError()))
	{
		WSADATA data = { 0 };
		int rv = WSAStartup(MAKEWORD(2, 2), &data);
		if (rv){WIN32_PRINT_ERROR("WSAStartup");goto error;}
	}
	else
	{
		int rv;
		rv = closesocket(testsocket);
		if (rv){WIN32_PRINT_ERROR("closesocket");goto error;}
		rv = shutdown(testsocket, SD_BOTH);
		if (rv){WIN32_PRINT_ERROR("shutdown");goto error;}
	}
	return;
error:
	exit(1);
}



void url_split(const char *url, char * proto, char * addr, unsigned * port)
{
	sscanf(url, "%[^://]://%[^:]:%u", proto, addr, port);
}



int bind_address(int sockfd, char addr[INET6_ADDRSTRLEN], unsigned port)
{
	struct sockaddr_storage storage;
	struct sockaddr_in * a = (void*)&storage;
	memset(a, 0, sizeof(struct sockaddr_in));
	a->sin_family = AF_INET;
	a->sin_addr.s_addr = htonl(INADDR_ANY);
	a->sin_port = htons((unsigned short)port);
	int r = bind(sockfd, (struct sockaddr *)a, sizeof(struct sockaddr_in));
	WIN32_PRINT_ERROR("bind");
	return r;
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
		SOCKET s = INVALID_SOCKET;
		if (ecs_os_strcmp(proto, "udp") == 0)
		{
			s = socket(AF_INET, SOCK_DGRAM, 0);
			ecs_set(it->world, it->entities[i], EgSocketUDP, {s});
		}
		else if (ecs_os_strcmp(proto, "tcp") == 0)
		{
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			ecs_set(it->world, it->entities[i], EgSocketTCP, {s});
		}
		bind_address(s, addr, port);
	}
}











void FlecsComponentsSocketImport(ecs_world_t *world)
{
	win32_sock_init();
	ECS_MODULE(world, FlecsComponentsSocket);
	ECS_COMPONENT_DEFINE(world, EgSocketTCP);
	ECS_COMPONENT_DEFINE(world, EgSocketUDP);
	ECS_COMPONENT_DEFINE(world, EgURL);

	ecs_trace("Hej");
	ecs_set_name_prefix(world, "Eg");


	ecs_set_component_actions(world, EgSocketUDP, {
	.ctor = ecs_ctor(EgSocketUDP),
	.dtor = ecs_dtor(EgSocketUDP),
	});


	ecs_set_component_actions(world, EgURL, {
	.ctor = ecs_ctor(EgURL),
	.dtor = ecs_dtor(EgURL),
	.copy = ecs_copy(EgURL),
	.move = ecs_move(EgURL),
	});


	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgSocketUDP),
	.members = {
	{ .name = "fd", .type = ecs_id(ecs_u64_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgSocketTCP),
	.members = {
	{ .name = "fd", .type = ecs_id(ecs_u64_t) }
	}
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgURL),
	.members = {
	{ .name = "path", .type = ecs_id(ecs_string_t) }
	}
	});



	ECS_TRIGGER(world, EgURLTrigger, EcsOnSet, EgURL);




}

