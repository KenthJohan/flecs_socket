#include <pthread.h>
#include "flecs_uv.h"

ECS_COMPONENT_DECLARE(UvLoop);
static ECS_CTOR(UvLoop, ptr, {ptr->loop = NULL;})
static ECS_DTOR(UvLoop, ptr, {if(ptr->loop){ecs_os_free(ptr->loop);}})
static ECS_COPY(UvLoop, dst, src, {dst->loop = src->loop;})
static ECS_MOVE(UvLoop, dst, src, {dst->loop = src->loop;src->loop = NULL;})

ECS_COMPONENT_DECLARE(UvTcp);
static ECS_CTOR(UvTcp, ptr, {ptr->stream = NULL;})
static ECS_DTOR(UvTcp, ptr, {if(ptr->stream){ecs_os_free(ptr->stream);}})
static ECS_COPY(UvTcp, dst, src, {dst->stream = src->stream;})
static ECS_MOVE(UvTcp, dst, src, {dst->stream = src->stream;src->stream = NULL;})

ECS_COMPONENT_DECLARE(UvStream);
static ECS_CTOR(UvStream, ptr, {ptr->stream = NULL;})
static ECS_DTOR(UvStream, ptr, {if(ptr->stream){ecs_os_free(ptr->stream);}})
static ECS_COPY(UvStream, dst, src, {dst->stream = src->stream;})
static ECS_MOVE(UvStream, dst, src, {dst->stream = src->stream;src->stream = NULL;})

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

int uv_ip_name(const struct sockaddr_storage* src, char* dst, size_t size)
{
	switch (src->ss_family)
	{
	case AF_INET:
		return uv_inet_ntop(AF_INET, &((struct sockaddr_in *)src)->sin_addr, dst, size);
		break;
	case AF_INET6:
		return uv_inet_ntop(AF_INET6, &((struct sockaddr_in6 *)src)->sin6_addr, dst, size);
		break;
	}
}


int get_name(uv_tcp_t *src, char dst[], int size)
{
	struct sockaddr_storage addr = { 0 };
	int alen = sizeof(addr);
	char ipstr[UV_IF_NAMESIZE];
	int r = uv_tcp_getpeername(src, (struct sockaddr *)&addr, &alen);
	uv_ip_name(&addr, ipstr, UV_IF_NAMESIZE);
	int port = get_port(&addr);
	snprintf(dst, size, "TCP//%s:%d", ipstr, port);
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

	ecs_entity_t e = ecs_new(world, UvTcp);
	uv_tcp_t * client = malloc(sizeof(uv_tcp_t));
	//uv_tcp_t * client = ecs_get_mut(world, e, uv_tcp_t, NULL);
	uv_tcp_init (loop, client);
	if (uv_accept(server, (uv_stream_t*) client) == 0)
	{
		char ipstr[128];
		get_name(client, ipstr, 128);
		ecs_trace("accepted %s", ipstr);
		ecs_set_name(world, e, ipstr);
		ecs_add_pair(world, e, EcsChildOf, parent);
		ecs_set(world, e, UvTcp, {client});
		uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
	}
	else
	{
		ecs_trace("uv_close %i", status);
		uv_close((uv_handle_t*) &client, NULL);
	}
}










static void UvLoop_OnAdd(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: UvLoop_OnAdd");
	UvLoop *loop = ecs_term(it, UvLoop, 1);
	for (int i = 0; i < it->count; i ++)
	{
		loop[i].loop = uv_default_loop();
	}
}




static void UvTcp_Server_OnSet(ecs_iter_t *it)
{
	UvLoop *loop = ecs_term(it, UvLoop, 1); //Parent
	UvTcp *tcp = ecs_term(it, UvTcp, 2);
	sockaddr_in *addr = ecs_term(it, sockaddr_in, 3);
	for (int i = 0; i < it->count; i ++)
	{
		tcp[i].stream = malloc(sizeof(uv_tcp_t));
		uv_tcp_init (loop[i].loop, tcp[i].stream);
		uv_tcp_bind (tcp[i].stream, (const struct sockaddr*)(addr + i), 0);
		tcp[i].stream->data = worldent_malloc(NULL, it->entities[i]);
		int r = uv_listen((uv_stream_t*)tcp[i].stream, 128, on_new_connection);
		if (r)
		{
			fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		}
	}
}


static void UvLoop_OnUpdate(ecs_iter_t *it)
{
	UvLoop *loop = ecs_term(it, UvLoop, 1);
	for (int i = 0; i < it->count; i ++)
	{
		loop[i].loop->data = it->world;
		uv_run (loop[i].loop, UV_RUN_NOWAIT);
	}
}


static void sys_TestComponent(ecs_iter_t *it)
{
	//ecs_trace("FLECSUV: sys_TestComponent");
	UvTcp *server = ecs_term(it, UvTcp, 1); // Parent
	UvTcp *client = ecs_term(it, UvTcp, 2);
	for (int i = 0; i < it->count; i ++)
	{

	}
}















void flecs_uv_init(ecs_world_t *world)
{
	ecs_trace("Init FLECS UV");
	ECS_COMPONENT_DEFINE(world, UvLoop);
	ecs_set_component_actions(world, UvLoop, {
	.ctor = ecs_ctor(UvLoop),
	.dtor = ecs_dtor(UvLoop),
	.copy = ecs_copy(UvLoop),
	.move = ecs_move(UvLoop)
	});
	ECS_COMPONENT_DEFINE(world, UvTcp);
	ecs_set_component_actions(world, UvTcp, {
	.ctor = ecs_ctor(UvTcp),
	.dtor = ecs_dtor(UvTcp),
	.copy = ecs_copy(UvTcp),
	.move = ecs_move(UvTcp)
	});
	ECS_COMPONENT_DEFINE(world, UvStream);
	ecs_set_component_actions(world, UvStream, {
	.ctor = ecs_ctor(UvStream),
	.dtor = ecs_dtor(UvStream),
	.copy = ecs_copy(UvStream),
	.move = ecs_move(UvStream)
	});

	ECS_COMPONENT_DEFINE(world, sockaddr_in);
	//ECS_COMPONENT_DEFINE(world, TestComponent);
	//ECS_TAG_DEFINE(world, MyTag);

	ECS_TRIGGER(world, UvLoop_OnAdd, EcsOnAdd, UvLoop);
	ECS_OBSERVER(world, UvTcp_Server_OnSet, EcsOnSet, UvLoop(parent), UvTcp, sockaddr_in);

	ECS_SYSTEM(world, UvLoop_OnUpdate, EcsOnUpdate, UvLoop);
	ECS_SYSTEM(world, sys_TestComponent, EcsOnUpdate, UvTcp(parent), UvTcp);

}
