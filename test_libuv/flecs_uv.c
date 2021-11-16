#include <pthread.h>
#include "flecs_net.h"
#include "flecs_uv.h"

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

ECS_COMPONENT_DECLARE(UvStream);
static ECS_CTOR(UvStream, ptr, {ptr->stream = NULL;})
static ECS_DTOR(UvStream, ptr, {if(ptr->stream){ecs_os_free(ptr->stream);}})
static ECS_COPY(UvStream, dst, src, {dst->stream = src->stream;})
static ECS_MOVE(UvStream, dst, src, {dst->stream = src->stream;src->stream = NULL;})

ECS_COMPONENT_DECLARE(uv_buf_t);
static ECS_CTOR(uv_buf_t, ptr, {ptr->base = NULL;ptr->len = 0;})
static ECS_DTOR(uv_buf_t, ptr, {ecs_trace("uv_buf_t::ECS_DTOR");if(ptr->base){ecs_os_free(ptr->base);}})
static ECS_COPY(uv_buf_t, dst, src, {dst->base = ecs_os_memdup(src->base, src->len);dst->len = src->len;})
static ECS_MOVE(uv_buf_t, dst, src, {dst->base = src->base;dst->len = src->len;src->base = NULL; src->len = 0;})
static ECS_ON_SET(uv_buf_t, ptr, {
ecs_trace("uv_buf_t::ECS_ON_SET %p %i", ptr->base, ptr->len);
})

ECS_COMPONENT_DECLARE(TestComponent);
//ECS_DECLARE(MyTag);







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



/*
int get_name(uv_tcp_t *src, char dst[], int size)
{
	struct sockaddr_storage addr = { 0 };
	int alen = sizeof(addr);
	char ipstr[UV_IF_NAMESIZE];
	int r = uv_tcp_getpeername(src, (struct sockaddr *)&addr, &alen);
	sockaddr_storage_get_name(&addr, ipstr, UV_IF_NAMESIZE);
	int port = sockaddr_storage_get_port(&addr);
	int n = snprintf(dst, size, "tcp://%s:%d", ipstr, port);
	for(int i = 0; i < n; ++i) {dst[i] = dst[i] == '.' ? ',':dst[i];}
	return r;
}
*/
























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

void echo_read(uv_stream_t *client, ssize_t nread, uv_buf_t const *buf)
{
	uv_loop_t *loop = client->loop;
	ecs_world_t *world = loop->data;
	ecs_entity_t parent = ((struct worldent *)client->data)->entity;

	//ecs_trace("nread:%i, UV_EOF=%i\n", nread, UV_EOF);
	ecs_trace("buf: %p %lli", buf->base, buf->len);
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
		ecs_entity_t e = ecs_new(world, uv_buf_t);
		ecs_set(world, e, uv_buf_t, {nread, buf->base});
		ecs_add_pair(world, e, EcsChildOf, parent);


		//uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
		//uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
		//uv_write(req, client, &wrbuf, 1, echo_write);
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
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		return;
	}



	struct uv_tcp_ecs * client = ecs_os_calloc_t(struct uv_tcp_ecs);
	uv_tcp_init (loop, (uv_tcp_t*)client);
	if (uv_accept(server, (uv_stream_t*) client) == 0)
	{
		ecs_entity_t e = ecs_new(world, 0);
		ecs_set_name(world, e, "HEJ");
		struct sockaddr_storage addr = { 0 };
		int alen = sizeof(addr);
		int r = uv_tcp_getpeername((uv_tcp_t*) client, (struct sockaddr *)&addr, &alen);
		if (r)
		{
			ecs_err("Listen error %s\n", uv_strerror(r));
		}
		ecs_add_pair(world, e, EcsChildOf, parent);
		ecs_set(world, e, UvTcp, {client});
		ecs_set_ptr(world, e, sockaddr_storage, &addr);
		//client->data = worldent_malloc(NULL, e);
		//uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
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
	uv_buf_t *buf = ecs_term(it, uv_buf_t, 2);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_trace("%*s", buf->len, buf->base);
		ecs_delete(it->world, it->entities[i]);

		uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
		char text[] = "HTTP/1.1 200 OK\r\n\r\n";
		uv_buf_t wrbuf = uv_buf_init(text, sizeof(text));
		//uv_buf_t wrbuf = uv_buf_init(buf[i].base, buf[i].len);
		uv_write(req, (uv_stream_t*)tcp[0].stream, &wrbuf, 1, echo_write);
		//ecs_delete(it->world, it-)

		//uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
		//uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
		//uv_write(req, client, &wrbuf, 1, echo_write);

		//printf("HTTP/1.1 200 OK\r\n\r\n");
		//printf("HTTP/1.1 200 OK\r\n\r\n");
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
	ECS_COMPONENT_DEFINE(world, uv_buf_t);
	ecs_set_component_actions(world, uv_buf_t, {
	.ctor = ecs_ctor(uv_buf_t),
	.dtor = ecs_dtor(uv_buf_t),
	.copy = ecs_copy(uv_buf_t),
	.move = ecs_move(uv_buf_t),
	.on_set = ecs_on_set(uv_buf_t)
	});
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(uv_buf_t),
	.members = {
	{ .name = "len", .type = ecs_id(ecs_i64_t) },
	{ .name = "base", .type = ecs_id(ecs_uptr_t) }
	}
	});

	//ECS_COMPONENT_DEFINE(world, TestComponent);
	//ECS_TAG_DEFINE(world, MyTag);

	ECS_TRIGGER(world, UvLoop_OnAdd, EcsOnAdd, UvLoop);
	ECS_SYSTEM(world, UvLoop_OnUpdate, EcsOnUpdate, UvLoop);
	ECS_OBSERVER(world, UvTcp_Server_OnSet, EcsMonitor, UvLoop(parent), UvTcp, sockaddr_storage);
	ECS_OBSERVER(world, UvTcp_OnRemove, EcsOnRemove, UvTcp);

	//ECS_OBSERVER(world, uv_buf_t_OnSet, EcsOnAdd, UvTcp(parent), uv_buf_t);

	//ECS_SYSTEM(world, sys_TestComponent, EcsOnUpdate, UvTcp(parent), UvTcp);

}
