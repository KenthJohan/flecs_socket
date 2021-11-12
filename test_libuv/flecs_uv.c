#include "flecs_uv.h"

ECS_COMPONENT_DECLARE(uv_loop_t);
ECS_COMPONENT_DECLARE(uv_tcp_t);
ECS_COMPONENT_DECLARE(uv_stream_t);
//ECS_DECLARE(MyTag);




void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
	if (status) {
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
	}
	free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	if (nread < 0) {
		if (nread != UV_EOF) {
			fprintf(stderr, "Read error %s\n", uv_err_name(nread));
			uv_close((uv_handle_t*) client, NULL);
		}
	} else if (nread > 0) {
		uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
		uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
		uv_write(req, client, &wrbuf, 1, echo_write);
	}

	if (buf->base) {
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

void on_new_connection(uv_stream_t *server, int status)
{
	ecs_world_t *world = ((struct worldent *)server->data)->world;
	ecs_entity_t entity = ((struct worldent *)server->data)->entity;
	ecs_progress(world, 0.0f);
	ecs_trace("on_new_connection %i", status);
	if (status < 0)
	{
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		return;
	}
	//ecs_defer_begin(world);
	ecs_entity_t e = ecs_new_entity(world, "TCP Client");
	ecs_add_pair(world, e, EcsChildOf, entity);
	//ecs_defer_end(world);
}










static void obs_loop(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: obs_loop");
	uv_loop_t *a = ecs_term(it, uv_loop_t, 1);
	for (int i = 0; i < it->count; i ++)
	{
		uv_loop_init(a+i);
	}
}

static void obs_tcp(ecs_iter_t *it)
{
	bool shared = !ecs_term_is_owned(it, 1);
	ecs_trace("FLECSUV: obs_tcp %i", shared);
	uv_loop_t *l = ecs_term(it, uv_loop_t, 1);
	uv_tcp_t *a = ecs_term(it, uv_tcp_t, 2);
	for (int i = 0; i < it->count; i ++)
	{
		uv_tcp_init(l+i, a+i);
		struct sockaddr_in addr;
		uv_ip4_addr("0.0.0.0", 7000, &addr);
		uv_tcp_bind(a+i, (const struct sockaddr*)&addr, 0);
		a[i].data = worldent_malloc(it->world, it->entities[i]);
		int r = uv_listen((uv_stream_t*)(a+i), 128, on_new_connection);
		if (r)
		{
			fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		}
	}
}

static void obs_accept(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: obs_accept");
	uv_loop_t *loop = ecs_term(it, uv_loop_t, 1);
	uv_stream_t *server = ecs_term(it, uv_stream_t, 2);
	for (int i = 0; i < it->count; i ++)
	{
		uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
		uv_tcp_init(loop+i, client);
		if (uv_accept(server+i, (uv_stream_t*) client) == 0)
		{
			uv_read_start((uv_stream_t*)client, alloc_buffer, echo_read);
		}
		else
		{
			uv_close((uv_handle_t*) client, NULL);
		}
	}
}

static void sys_run(ecs_iter_t *it)
{
	ecs_trace("FLECSUV: sys_run");
	uv_loop_t *a = ecs_term(it, uv_loop_t, 1);
	for (int i = 0; i < it->count; i ++)
	{
		uv_run(a+i, UV_RUN_NOWAIT);
	}
}





void flecs_uv_init(ecs_world_t *world)
{
	ecs_trace("Init FLECS UV");
	ECS_COMPONENT_DEFINE(world, uv_loop_t);
	ECS_COMPONENT_DEFINE(world, uv_tcp_t);
	ECS_COMPONENT_DEFINE(world, uv_stream_t);
	//ECS_TAG_DEFINE(world, MyTag);
	ECS_OBSERVER(world, obs_loop, EcsOnAdd, uv_loop_t);
	ECS_OBSERVER(world, obs_tcp, EcsOnAdd, uv_loop_t(parent), uv_tcp_t);
	ECS_OBSERVER(world, obs_accept, EcsOnAdd, uv_loop_t(parent), uv_stream_t(parent));
	ECS_SYSTEM(world, sys_run, EcsOnUpdate, uv_loop_t);
}
