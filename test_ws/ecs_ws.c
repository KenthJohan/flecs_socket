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

ECS_COMPONENT_DECLARE(EgSocketTCP);
ECS_COMPONENT_DECLARE(EgSocketPort);
ECS_COMPONENT_DECLARE(EgSocketMaxconn);
ECS_COMPONENT_DECLARE(EgSocketAcceptThread);
ECS_COMPONENT_DECLARE(EgWebsockMeta);

#define MAX_CLIENTS 8









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
	EgSocketTCP *s = ecs_term(it, EgSocketTCP, 1);
	EgSocketPort *p = ecs_term(it, EgSocketPort, 2);
	EgSocketMaxconn *m = ecs_term(it, EgSocketMaxconn, 3);
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
static void eg_add_prefab(ecs_world_t *world, ecs_entity_t e, ecs_entity_t prefab)
{
	ecs_type_t t = ecs_get_type(world, prefab);
	ecs_entity_t *ids = ecs_vector_first(t, ecs_entity_t);
	for (int32_t i = 0; i < ecs_vector_count(t); ++i)
	{
		//void const * a = ecs_get_id(world, prefab, ids[i]);
		//ecs_add_id(world, e, ids[i]);
		//ecs_set_id(world, e, ids[i], 1, a);
	}
}

static void * eg_acceptor(void * data)
{
	struct eg_acceptor_params * d = data;
	int sock = d->sock;
	ecs_world_t *world = d->world;
	ecs_entity_t entity = d->entity;
	ecs_entity_t prefab = d->prefab;
	//free(data);
	struct sockaddr_in client;
	int len = sizeof(struct sockaddr_in);
	while (1)
	{
		int sock1 = accept(sock, (struct sockaddr *)&client, (socklen_t *)&len);
		if (sock1 < 0)
		{
			EG_ERR_WIN32();
			ecs_remove(world, entity, EgSocketAcceptThread);
			break;
		}
		eg_trace_address(sock1);
		ecs_entity_t e = ecs_new_entity(world, "Bob");
		ecs_set(world, e, EgSocketTCP, {sock1});
		//eg_add_prefab(world, e, prefab);
		ecs_add_pair(world, e, EcsIsA, prefab);
		eg_trace_typestr(world, e);
	}
	return NULL;
}

static void sys_EgSocketAcceptThread(ecs_iter_t *it)
{
	EgSocketTCP          *s = ecs_term(it, EgSocketTCP, 1);
	EgSocketAcceptThread *t = ecs_term(it, EgSocketAcceptThread, 2);
	for (int i = 0; i < it->count; i ++)
	{
		int sock = s[i].sock;
		int prefab = t[i].prefab;
		ecs_trace("Waiting for incoming connections. %i: sock=%i, prefab=%i\n", it->entities[i], sock, prefab);
		pthread_t thread;
		struct eg_acceptor_params * params = malloc(sizeof(struct eg_acceptor_params));
		params->world = it->world;
		params->entity = it->entities[i];
		params->sock = s[i].sock;
		params->prefab = t[i].prefab;
		//printf("prefab %jx %jx\n", params->prefab, t[i].prefab);
		pthread_create(&thread, NULL, eg_acceptor, params);
	}
}


static void sys_EgWebsockMeta(ecs_iter_t *it)
{
	EgSocketTCP   *s = ecs_term(it, EgSocketTCP, 1);
	EgWebsockMeta *w = ecs_term(it, EgWebsockMeta, 2);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_trace("sys_EgWebsockMeta %i %i", s[i].sock, w[i].meta);
	}
}



void ws_flecs_init(ecs_world_t *world)
{
	 ECS_COMPONENT_DEFINE(world, EgSocketTCP);
	 ECS_COMPONENT_DEFINE(world, EgSocketPort);
	 ECS_COMPONENT_DEFINE(world, EgSocketMaxconn);
	 ECS_COMPONENT_DEFINE(world, EgSocketAcceptThread);
	 ECS_COMPONENT_DEFINE(world, EgWebsockMeta);
	 //ECS_SYSTEM(world, system_ws_acceptor, EcsOnUpdate, [in] EgSocketTCP);
	 ECS_OBSERVER(world, sys_EgSocketTCP, EcsOnSet, EgSocketTCP, EgSocketPort, EgSocketMaxconn);
	 ECS_OBSERVER(world, sys_EgSocketAcceptThread, EcsOnSet, EgSocketTCP, EgSocketAcceptThread);
	 ECS_OBSERVER(world, sys_EgWebsockMeta, EcsOnAdd, EgSocketTCP, EgWebsockMeta);
}


