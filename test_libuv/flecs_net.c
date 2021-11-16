#include "flecs_net.h"


ECS_COMPONENT_DECLARE(sockaddr_storage);
ECS_COMPONENT_DECLARE(IpAddr);
ECS_COMPONENT_DECLARE(Port);



int sockaddr_storage_get_port(struct sockaddr_storage * addr)
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

int sockaddr_storage_get_name(const struct sockaddr_storage* src, char* dst, size_t size)
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



static void sockaddr_OnSet1(ecs_iter_t *it)
{
	sockaddr_storage *addr = ecs_term(it, sockaddr_storage, 1);
	IpAddr *ip = ecs_term(it, IpAddr, 2);
	Port *port = ecs_term(it, Port, 3);
	for (int i = 0; i < it->count; i ++)
	{
		struct sockaddr_in* a = (struct sockaddr_in*) (addr + i);
		uv_ip4_addr(ip[i], port[i], a);
	}
}

static void sockaddr_OnSet2(ecs_iter_t *it)
{
	sockaddr_storage *addr = ecs_term(it, sockaddr_storage, 1);
	for (int i = 0; i < it->count; i ++)
	{
		int port = sockaddr_storage_get_port(addr + i);
		char * ipstr = malloc(UV_IF_NAMESIZE);
		sockaddr_storage_get_name(addr + i, ipstr, UV_IF_NAMESIZE);
		ecs_set(it->world, it->entities[i], Port, {port});
		ecs_set(it->world, it->entities[i], IpAddr, {ipstr});
	}
}


void FlecsNetImport(ecs_world_t *world)
{
	ECS_COMPONENT_DEFINE(world, sockaddr_storage);
	ECS_COMPONENT_DEFINE(world, IpAddr);
	ECS_COMPONENT_DEFINE(world, Port);
	ECS_OBSERVER(world, sockaddr_OnSet1, EcsMonitor, sockaddr_storage, IpAddr, Port);
	ECS_TRIGGER(world, sockaddr_OnSet2, EcsOnSet, sockaddr_storage);

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(IpAddr),
	.members = {{ .name = "IpAddr", .type = ecs_id(ecs_string_t) },}
	});
	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(Port),
	.members = {{ .name = "Port", .type = ecs_id(ecs_i32_t) },}
	});
}
