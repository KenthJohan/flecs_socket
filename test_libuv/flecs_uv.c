#include <pthread.h>
#include "flecs_net.h"
#include "flecs_uv.h"
#include "circularbuf.h"


ECS_COMPONENT_DECLARE(UvLoop);
static ECS_CTOR(UvLoop, ptr, {ptr->loop = NULL;})
static ECS_DTOR(UvLoop, ptr, {if(ptr->loop){ecs_os_free(ptr->loop);}})
static ECS_COPY(UvLoop, dst, src, {dst->loop = src->loop;})
static ECS_MOVE(UvLoop, dst, src, {dst->loop = src->loop;src->loop = NULL;})

ECS_COMPONENT_DECLARE(UvTcp);
static ECS_CTOR(UvTcp, ptr, {ecs_trace("UvTcp::ECS_CTOR");ptr->stream = NULL;})
static ECS_DTOR(UvTcp, ptr, {ecs_trace("UvTcp::ECS_DTOR");if(ptr->stream){ecs_os_free(ptr->stream);}})
static ECS_COPY(UvTcp, dst, src, {ecs_trace("UvTcp::ECS_COPY");dst->stream = src->stream;})
static ECS_MOVE(UvTcp, dst, src, {ecs_trace("UvTcp::ECS_MOVE");dst->stream = src->stream;src->stream = NULL;})

ECS_COMPONENT_DECLARE(UvUdp);
static ECS_CTOR(UvUdp, ptr, {ecs_trace("UvUdp::ECS_CTOR");ptr->stream = NULL;})
static ECS_DTOR(UvUdp, ptr, {ecs_trace("UvUdp::ECS_DTOR");if(ptr->stream){ecs_os_free(ptr->stream);}})
static ECS_COPY(UvUdp, dst, src, {ecs_trace("UvUdp::ECS_COPY");dst->stream = src->stream;})
static ECS_MOVE(UvUdp, dst, src, {ecs_trace("UvUdp::ECS_MOVE");dst->stream = src->stream;src->stream = NULL;})



























void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	struct uv_tcp_ecs * c = (void*)handle;
	buf->base = circular_buf_head(&c->buf);
	buf->len = circular_buf_freespace(&c->buf);
	ecs_trace("alloc_buffer len: %i", buf->len);
}

void echo_write(uv_write_t *req, int status)
{
	if (status)
	{
		ecs_err("Write error %s\n", uv_strerror(status));
	}
	free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, uv_buf_t const *buf)
{
	struct uv_tcp_ecs * c = (void*)client;
	//ecs_trace("nread:%i, UV_EOF=%i\n", nread, UV_EOF);
	//ecs_trace("buf: %p %lli", buf->base, buf->len);
	if (nread < 0)
	{
		if (nread != UV_EOF)
		{
			ecs_err("Read error %s\n", uv_err_name(nread));
			uv_close((uv_handle_t*) client, NULL);
		}
	}
	else if (nread > 0)
	{
		printf("\n======================\n%.*s\n", nread, buf->base);
		c->buf.head += nread;
	}
}





void on_new_connection(uv_stream_t *server, int status)
{
	uv_loop_t *loop = server->loop;
	ecs_world_t *world = ((struct uv_loop_ecs *)loop)->world;
	ecs_entity_t parent = ((struct uv_tcp_ecs *)server)->entity;
	ecs_trace("on_new_connection %i", status);
	if (status < 0)
	{
		ecs_err("New connection error %s\n", uv_strerror(status));
		return;
	}



	struct uv_tcp_ecs * client = ecs_os_calloc_t(struct uv_tcp_ecs);
	circular_buf_init(&client->buf);
	uv_tcp_init (loop, (uv_tcp_t*) client);
	if (uv_accept(server, (uv_stream_t*) client) == 0)
	{
		client->world = world;
		client->entity = ecs_new(world, 0);
		struct sockaddr_storage addr = { 0 };
		int alen = sizeof(addr);
		int r = uv_tcp_getpeername((uv_tcp_t*) client, (struct sockaddr *)&addr, &alen);
		if (r)
		{
			ecs_err("Listen error %s\n", uv_strerror(r));
		}
		ecs_set_name_sockaddr(world, client->entity, "tcp://", &addr);
		ecs_add_pair(world, client->entity, EcsChildOf, parent);
		ecs_set(world, client->entity, UvTcp, {client});
		ecs_set_ptr(world, client->entity, sockaddr_storage, &addr);
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
		loop[i].loop = ecs_os_calloc(sizeof(struct uv_loop_ecs));
		int r = uv_loop_init((uv_loop_t*)loop[i].loop);
		if (r)
		{
			ecs_err("uv_loop_init error %s\n", uv_strerror(r));
		}
	}
}




static void UvTcp_Server_OnSet(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: UvTcp_Server_OnSet");
	UvLoop *loop = ecs_term(it, UvLoop, 1); //Parent
	UvTcp *tcp = ecs_term(it, UvTcp, 2);
	sockaddr_storage *addr = ecs_term(it, sockaddr_storage, 3);
	for (int i = 0; i < it->count; i ++)
	{
		tcp[i].stream = ecs_os_calloc(sizeof(struct uv_tcp_ecs));
		tcp[i].stream->world = it->world;
		tcp[i].stream->entity = it->entities[i];
		uv_tcp_init ((uv_loop_t*)loop[0].loop, (uv_tcp_t*)tcp[i].stream);
		uv_tcp_bind ((uv_tcp_t*)tcp[i].stream, (const struct sockaddr*)(addr + i), 0);
		int r = uv_listen((uv_stream_t*)tcp[i].stream, 128, on_new_connection);
		if (r)
		{
			ecs_err("Listen error %s\n", uv_strerror(r));
		}
	}
}


static void UvLoop_OnUpdate(ecs_iter_t *it)
{
	UvLoop *loop = ecs_term(it, UvLoop, 1);
	for (int i = 0; i < it->count; i ++)
	{
		loop[i].loop->world = it->world;
		uv_run ((uv_loop_t*)loop[i].loop, UV_RUN_NOWAIT);
	}
}


static void sys_TestComponent(ecs_iter_t *it)
{
	//ecs_trace("FLECSUV: sys_TestComponent");
	//UvTcp *server = ecs_term(it, UvTcp, 1); // Parent
	//UvTcp *client = ecs_term(it, UvTcp, 2);
	for (int i = 0; i < it->count; i ++)
	{

	}
}


void on_close(uv_handle_t* handle)
{

}


static void UvTcp_OnRemove(ecs_iter_t *it)
{
	//UvTcp *tcp = ecs_term(it, UvTcp, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_trace("Closing TCP %s", ecs_get_name(it->world, it->entities[i]));
		//uv_close((uv_handle_t*) tcp[i].stream, on_close);
	}
}


static void uv_buf_t_OnSet(ecs_iter_t *it)
{
	//ecs_trace("FLECSUV: sys_TestComponent");
	UvTcp *tcp = ecs_term(it, UvTcp, 1); // Parent
	for (int i = 0; i < it->count; i ++)
	{
		//ecs_trace("%*s", buf->len, buf->base);
		//ecs_delete(it->world, it->entities[i]);
	}
}













void flecs_uv_init(ecs_world_t *world)
{
	ecs_trace("Init FLECS UV");
	test_circular_buf();

	ECS_COMPONENT_DEFINE(world, UvLoop);
	ECS_COMPONENT_DEFINE(world, UvTcp);
	ECS_COMPONENT_DEFINE(world, UvUdp);

	ecs_set_component_actions(world, UvLoop, {.ctor = ecs_ctor(UvLoop),.dtor = ecs_dtor(UvLoop),.copy = ecs_copy(UvLoop),.move = ecs_move(UvLoop)});
	ecs_set_component_actions(world, UvTcp, {.ctor = ecs_ctor(UvTcp),.dtor = ecs_dtor(UvTcp),.copy = ecs_copy(UvTcp),.move = ecs_move(UvTcp)});
	ecs_set_component_actions(world, UvUdp, {.ctor = ecs_ctor(UvUdp),.dtor = ecs_dtor(UvUdp),.copy = ecs_copy(UvUdp),.move = ecs_move(UvUdp)});

	//ECS_COMPONENT_DEFINE(world, TestComponent);
	//ECS_TAG_DEFINE(world, MyTag);

	ECS_TRIGGER(world, UvLoop_OnAdd, EcsOnAdd, UvLoop);
	ECS_SYSTEM(world, UvLoop_OnUpdate, EcsOnUpdate, UvLoop);
	ECS_OBSERVER(world, UvTcp_Server_OnSet, EcsMonitor, UvLoop(parent), UvTcp, sockaddr_storage);
	ECS_OBSERVER(world, UvTcp_OnRemove, EcsOnRemove, UvTcp);

	//ECS_OBSERVER(world, uv_buf_t_OnSet, EcsOnAdd, UvTcp(parent), uv_buf_t);

	//ECS_SYSTEM(world, sys_TestComponent, EcsOnUpdate, UvTcp(parent), UvTcp);

}
