#include <pthread.h>
#include "flecs_uv.h"

ECS_COMPONENT_DECLARE(uv_loop_t);
ECS_COMPONENT_DECLARE(uv_tcp_t);
ECS_COMPONENT_DECLARE(uv_stream_t);
ECS_COMPONENT_DECLARE(sockaddr_in);
ECS_COMPONENT_DECLARE(TestComponent);
//ECS_DECLARE(MyTag);




void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status)
{
	if (status)
	{
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
	}
	free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	if (nread < 0)
	{
		if (nread != UV_EOF)
		{
			fprintf(stderr, "Read error %s\n", uv_err_name(nread));
			uv_close((uv_handle_t*) client, NULL);
		}
	}
	else if (nread > 0)
	{
		uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
		uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
		uv_write(req, client, &wrbuf, 1, echo_write);
	}

	if (buf->base)
	{
		free(buf->base);
	}
}

struct worldent
{
	ecs_world_t *world;
	ecs_entity_t entity;
};

struct worldent * worldent_malloc(ecs_world_t *world, ecs_entity_t entity)
{
	struct worldent * params = malloc(sizeof(struct worldent));
	params->world = world;
	params->entity = entity;
	return params;
}

int get_port(struct sockaddr_storage * addr)
{
	switch (addr->ss_family)
	{
	case AF_INET:
		return ntohs(((struct sockaddr_in *)addr)->sin_port);
		break;
	case AF_INET6:
		return ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
		break;
	}
}


int get_name(uv_tcp_t *t, char ipstr[UV_IF_NAMESIZE+10])
{
	struct sockaddr_storage addr = { 0 };
	int alen = sizeof(addr);
	int r = uv_tcp_getpeername(t, (struct sockaddr *)&addr, &alen);
	uv_inet_ntop(addr.ss_family, &addr, ipstr, UV_IF_NAMESIZE);
	char portstr[10];
	snprintf(portstr, 10, ":%i", get_port(&addr));
	ecs_os_strcat(ipstr, portstr);
	return r;
}



void on_new_connection(uv_stream_t *server, int status)
{
	uv_loop_t * loop = server->loop;
	ecs_world_t *world = loop->data;
	ecs_entity_t parent = ((struct worldent *)server->data)->entity;
	ecs_trace("on_new_connection %i", status);
	if (status < 0)
	{
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		return;
	}

	ecs_entity_t e = ecs_new_entity(world, "TCP_Client");
	//uv_tcp_t * client = malloc(sizeof(uv_tcp_t));
	uv_tcp_t * client = ecs_emplace(world, e, uv_tcp_t);
	uv_tcp_init (loop, client);
	if (uv_accept(server, (uv_stream_t*) client) == 0)
	{
		char ipstr[UV_IF_NAMESIZE+10];
		get_name(client, ipstr);
		ecs_trace("accepted %s", ipstr);
		ecs_set_name(world, e, ipstr);
		ecs_add_pair(world, e, EcsChildOf, parent);
		ecs_set_ptr(world, e, uv_tcp_t, client);
		//uv_tcp_t * c = ecs_get_mut(world, e, uv_tcp_t, NULL);
		uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
	}
	else
	{
		ecs_trace("uv_close %i", status);
		uv_close((uv_handle_t*) &client, NULL);
	}
}










static void obs_loop(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: obs_loop");
	uv_loop_t *loop = ecs_term(it, uv_loop_t, 1);
	for (int i = 0; i < it->count; i ++)
	{
		uv_loop_init (loop + i);
	}
}

static void obs_tcpserver(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: obs_tcpserver %i", !ecs_term_is_owned(it, 1));
	uv_loop_t *loop = ecs_term(it, uv_loop_t, 1);
	uv_tcp_t *tcp = ecs_term(it, uv_tcp_t, 2);
	sockaddr_in *addr = ecs_term(it, sockaddr_in, 3);
	for (int i = 0; i < it->count; i ++)
	{
		uv_tcp_init (loop + i, tcp + i);
		uv_tcp_bind (tcp + i, (const struct sockaddr*)(addr + i), 0);
		tcp[i].data = worldent_malloc(NULL, it->entities[i]);
		int r = uv_listen((uv_stream_t*)(tcp+i), 128, on_new_connection);
		if (r)
		{
			fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		}
	}
}


static int counter = 0;
static void sys_run(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: sys_run");
	uv_loop_t *loop = ecs_term(it, uv_loop_t, 1);
	for (int i = 0; i < it->count; i ++)
	{
		loop[i].data = it->world;
		uv_run (loop + i, UV_RUN_NOWAIT);
		ecs_trace("FLECSUV: sys_run %i", counter++);
	}
}


static void sys_TestComponent(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: sys_TestComponent");
	uv_tcp_t *server = ecs_term(it, uv_tcp_t, 1);
	uv_tcp_t *client = ecs_term(it, uv_tcp_t, 2);
	for (int i = 0; i < it->count; i ++)
	{

	}
}


void flecs_uv_init(ecs_world_t *world)
{
	ecs_trace("Init FLECS UV");
	ECS_COMPONENT_DEFINE(world, uv_loop_t);
	ECS_COMPONENT_DEFINE(world, uv_tcp_t);
	ECS_COMPONENT_DEFINE(world, uv_stream_t);
	ECS_COMPONENT_DEFINE(world, sockaddr_in);
	//ECS_COMPONENT_DEFINE(world, TestComponent);
	//ECS_TAG_DEFINE(world, MyTag);

	ECS_OBSERVER(world, obs_loop, EcsOnAdd, uv_loop_t);
	ECS_OBSERVER(world, obs_tcpserver, EcsOnSet, uv_loop_t(parent), uv_tcp_t, sockaddr_in);

	ECS_SYSTEM(world, sys_run, EcsOnUpdate, uv_loop_t);
	ECS_SYSTEM(world, sys_TestComponent, EcsOnUpdate, uv_tcp_t(parent), uv_tcp_t);
}
