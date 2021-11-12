#pragma once

#include <pthread.h>
#include <stdio.h>
#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
typedef int socklen_t;
#endif

#include <flecs.h>
#include "ecs_ws.h"

ECS_COMPONENT_DECLARE(EgSocket);
ECS_COMPONENT_DECLARE(EgTCP);
ECS_COMPONENT_DECLARE(EgUDP);
ECS_COMPONENT_DECLARE(EgPort);
ECS_COMPONENT_DECLARE(EgMaxconn);
ECS_COMPONENT_DECLARE(EgAcceptThread);
ECS_COMPONENT_DECLARE(EgWS);
ECS_COMPONENT_DECLARE(EgReqHTTP);

#define MAX_CLIENTS 8



struct eg_threadparams
{
	ecs_world_t *world;
	ecs_entity_t entity;
};

struct eg_threadparams * eg_threadparams_malloc(ecs_world_t *world, ecs_entity_t entity)
{
	struct eg_threadparams * params = malloc(sizeof(struct eg_threadparams));
	params->world = world;
	params->entity = entity;
	return params;
}





void eg_ws_init()
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode (hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode (hOut, dwMode);
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ecs_fatal("WSAStartup failed!");
	}

	/**
	 * Sets stdout to be non-buffered.
	 *
	 * According to the docs from MSDN (setvbuf page), Windows do not
	 * really supports line buffering but full-buffering instead.
	 *
	 * Quote from the docs:
	 * "... _IOLBF For some systems, this provides line buffering.
	 *  However, for Win32, the behavior is the same as _IOFBF"
	 */
	setvbuf(stdout, NULL, _IONBF, 0);
#endif
}

static void eg_socket_seti(int sock, int level, int optname, int value)
{
	int r = setsockopt(sock, level, optname, (const char *)&value, sizeof(value));
	if (r < 0)
	{
		ecs_fatal("setsockopt(SO_REUSEADDR) failed");
	}
}

#define EG_ERR_WIN32() eg_err_win32(__FILE__, __LINE__)
static void eg_err_win32(char * filename, int line)
{
	wchar_t buf[256];
	DWORD e = GetLastError();
	DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	FormatMessageW(dwFlags, NULL, e, dwLanguageId, buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
	_ecs_err(filename, line, "GetLastError: %S (%li)\n", buf, e);
}

static void eg_trace_address(int sock)
{
	struct sockaddr_in addr;
	char addrstr[INET_ADDRSTRLEN];
	int len = sizeof(struct sockaddr_in);
	if (getsockname(sock, (struct sockaddr *)&addr, &len) < 0)
	{
		return;
	}
	if (!inet_ntop(AF_INET, &addr.sin_addr, addrstr, INET_ADDRSTRLEN))
	{
		return;
	}
	ecs_trace("%s:%i", addrstr, ntohs(addr.sin_port));
}

static void eg_trace_typestr(ecs_world_t *world, ecs_entity_t e)
{
	char *type_str = ecs_type_str(world, ecs_get_type(world, e));
	const char *name = ecs_get_name(world, e);
	ecs_trace("%s: [%s]\n", name, type_str);
	ecs_os_free(type_str);
}

static void sys_EgSocketTCP(ecs_iter_t *it)
{
	EgSocket  *s = ecs_term(it, EgSocket,  1);
	EgTCP     *a = ecs_term(it, EgTCP,     2);
	EgPort    *p = ecs_term(it, EgPort,    3);
	EgMaxconn *m = ecs_term(it, EgMaxconn, 4);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_trace("EgSocketTCP_OnAdd %i", i);
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0)
		{
			ecs_err("WSAStartup failed!");
			continue;
		}
		eg_socket_seti(sock, SOL_SOCKET, SO_REUSEADDR, 1);
		struct sockaddr_in server;
		server.sin_family      = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port        = htons(p[i].port);
		if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			ecs_err("Bind failed");
			continue;
		}
		listen(sock, m[i].backlog);
		eg_trace_address(sock);
		s[i].sock = sock;
	}
}

struct eg_acceptor_params
{
	ecs_world_t *world;
	ecs_entity_t entity;
	ecs_entity_t prefab;
	int sock;
};

//How to copy all components and values from one entity and paste them into another entity?
static void eg_copy_components(ecs_world_t *world, ecs_entity_t dst, ecs_entity_t src)
{
	eg_trace_typestr(world, src);
	ecs_type_t t = ecs_get_type(world, src);
	ecs_entity_t *ids = ecs_vector_first(t, ecs_entity_t);
	int32_t count = ecs_vector_count(t);
	for (int32_t i = 0; i < count; ++i)
	{
		if (ids[i] == EcsPrefab) {continue;}
		if (ECS_HAS_RELATION(ids[i], ecs_id(EcsIdentifier))) {continue;}
		const EcsComponent *component_ptr = ecs_get(world, ids[i], EcsComponent);
		void const * value = ecs_get_id(world, src, ids[i]);
		ecs_set_id(world, dst, ids[i], component_ptr->size, value);
	}
}

static void * eg_acceptor(void * data)
{
	ecs_world_t *world = ((struct eg_threadparams * )data)->world;
	ecs_entity_t entity = ((struct eg_threadparams * )data)->entity;
	free(data);

	//EgAcceptThread * a = ecs_get(world, entity, EgAcceptThread);

	int sock = ecs_get(world, entity, EgSocket)->sock;
	ecs_entity_t prefab = ecs_get(world, entity, EgAcceptThread)->prefab;
	struct sockaddr_in client;
	int len = sizeof(struct sockaddr_in);
	while (1)
	{
		int sock1 = accept(sock, (struct sockaddr *)&client, (socklen_t *)&len);
		if (sock1 < 0)
		{
			EG_ERR_WIN32();
			ecs_remove(world, entity, EgAcceptThread);
			break;
		}
		eg_trace_address(sock1);
		ecs_entity_t e = ecs_new_entity(world, "Bob");
		ecs_set(world, e, EgSocket, {sock1});
		eg_trace_typestr(world, prefab);
		eg_copy_components(world, e, prefab);
		//ecs_add_pair(world, e, EcsIsA, prefab);
	}
	return NULL;
}

static void sys_EgSocketAcceptThread(ecs_iter_t *it)
{
	EgSocket       *s = ecs_term(it, EgSocket,       1);
	EgTCP          *a = ecs_term(it, EgTCP,          2);
	EgAcceptThread *t = ecs_term(it, EgAcceptThread, 3);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_trace("Waiting for incoming connections. %i: sock=%i, prefab=%i\n", it->entities[i], s[i].sock, t[i].prefab);
		pthread_t thread;
		//printf("prefab %jx %jx\n", params->prefab, t[i].prefab);
		pthread_create(&thread, NULL, eg_acceptor, eg_threadparams_malloc(it->world, it->entities[i]));
	}
}




static void * eg_wsreader(void * data)
{
	ecs_world_t *world = ((struct eg_threadparams * )data)->world;
	ecs_entity_t entity = ((struct eg_threadparams * )data)->entity;
	free(data);

	EgReqHTTP const * r = ecs_get(world, entity, EgReqHTTP);
	int sock = ecs_get(world, entity, EgSocket)->sock;
	while (1)
	{
		//ecs_strbuf_t b = ECS_STRBUF_INIT;
		//ecs_strbuf_appendstrn(b, buf, n);

		{
			char buf[2796];
			int n = recv(sock, buf,2796, 0);
			printf("%i, %.*s\n", n, n, buf);
		}
		{
			//int result = recv(connection,&buffer,1,MSG_PEEK);

			char buf[1];
			int n = recv(sock, buf,1, 0);
			printf("%i, %.*s\n", n, n, buf);
		}
	}
	return NULL;
}


static void sys_EgWS(ecs_iter_t *it)
{
	EgSocket   *s = ecs_term(it, EgSocket, 1);
	EgWS       *w = ecs_term(it, EgWS, 2);
	EgReqHTTP  *r = ecs_term(it, EgReqHTTP, 3);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_trace("sys_EgWebsockMeta %i %i", s[i].sock, w[i].unused);
		pthread_t thread;
		pthread_create(&thread, NULL, eg_wsreader, eg_threadparams_malloc(it->world, it->entities[i]));
	}
}



void ws_flecs_init(ecs_world_t *world)
{
	 ECS_COMPONENT_DEFINE(world, EgSocket);
	 ECS_COMPONENT_DEFINE(world, EgTCP);
	 ECS_COMPONENT_DEFINE(world, EgUDP);
	 ECS_COMPONENT_DEFINE(world, EgWS);
	 ECS_COMPONENT_DEFINE(world, EgPort);
	 ECS_COMPONENT_DEFINE(world, EgMaxconn);
	 ECS_COMPONENT_DEFINE(world, EgAcceptThread);
	 ECS_COMPONENT_DEFINE(world, EgReqHTTP);
	 //ECS_SYSTEM(world, system_ws_acceptor, EcsOnUpdate, [in] EgSocketTCP);
	 ECS_OBSERVER(world, sys_EgSocketTCP, EcsOnSet, EgSocket, EgTCP, EgPort, EgMaxconn);
	 ECS_OBSERVER(world, sys_EgSocketAcceptThread, EcsOnSet, EgSocket, EgTCP, EgAcceptThread);
	 ECS_OBSERVER(world, sys_EgWS, EcsOnSet, EgSocket, EgWS, EgReqHTTP);
}


