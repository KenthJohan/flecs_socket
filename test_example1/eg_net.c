#include "eg_net.h"
#include "eg_socket.h"
#include "eg_basic.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>


ECS_COMPONENT_DECLARE(EgNetProtocol1);

#define ECS_FD_PRINTERROR() ecs_fd_printerror(__FILE__, __LINE__)





void parse_package(ecs_world_t * world, ecs_entity_t ev[256], ecs_entity_t cv[256], uint8_t msg[], ecs_size_t size)
{
	while(size >= 2)
	{
		uint8_t ie = msg[0]; // Indirect entity
		uint8_t ic = msg[1]; // Indirect component
		msg += 2; // Goto payload
		size -= 2;
		ecs_entity_t e = ev[ie]; // Get entity
		ecs_entity_t c = cv[ic];
		const EcsComponent * comp = ecs_get(world, c, EcsComponent);
		if (comp)
		{
			ecs_warn("Component %016jx not found", c);
			exit(1);
		}
		if (size < comp->size)
		{
			ecs_warn("Msg size %i is less than component size %i", size, comp->size);
			exit(1);
		}
		ecs_set_id(world, e, c, comp->size, msg);
		msg += comp->size;
	}
}


#define BUFLEN 100
void * the_thread(void * arg)
{
	struct eg_we * we = arg;
	SOCKET s = ecs_get(we->world, we->entity, EgSocketUDP)->fd;
	char buf[BUFLEN];
	int recv_len;
	int slen;
	struct sockaddr_in si_other;


	we->state = EG_STATE_RUNNING;
	while(we->action != EG_ACTION_STOP)
	{
		ecs_trace("Waiting for data...");


		we->state = EG_STATE_PENDING;
		//try to receive some data, this is a blocking call
		slen = sizeof(struct sockaddr_in);
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			WIN32_PRINT_ERROR("recvfrom");
		}

		//print details of the client/peer and the data received
		ecs_trace("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		ecs_trace("Data: %s\n" , buf);

		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			WIN32_PRINT_ERROR("sendto");
		}
	}


	we->state = EG_STATE_STOPPED;
}


void EgNetProtocol1_start(ecs_iter_t *it)
{
	EgNetProtocol1 *a = ecs_term(it, EgNetProtocol1, 1);
	EgSocketUDP *s = ecs_term(it, EgSocketUDP, 2);
	for (int i = 0; i < it->count; i ++)
	{
		a[i].we = ecs_os_calloc_t(struct eg_we);
		a[i].we->world = it->world;
		a[i].we->entity = it->entities[i];
		a[i].we->action = EG_ACTION_RUN;
		ecs_os_thread_new(the_thread, a[i].we);
	}
}

void EgNetProtocol1_system(ecs_iter_t *it)
{
	EgNetProtocol1 *p = ecs_term(it, EgNetProtocol1, 1);
	EgState *s = ecs_term(it, EgState, 2);
	EgAction *a = ecs_term(it, EgAction, 3);
	for (int i = 0; i < it->count; i ++)
	{
		p[i].action = a[i].action;
		s[i].state = p[i].state;
		s[i].text = eg_state_str[s[i].state];
		a[i].text = eg_action_str[a[i].action];
	}
}


void FlecsComponentsNetImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsNet);
	ecs_set_name_prefix(world, "Eg");
	ECS_COMPONENT_DEFINE(world, EgNetProtocol1);

	ECS_OBSERVER(world, EgNetProtocol1_start, EcsOnAdd, EgNetProtocol1, EgSocketUDP);
	ECS_SYSTEM(world, EgNetProtocol1_system, EcsOnUpdate, EgNetProtocol1, EgState, EgAction);


	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgNetProtocol1),
	.members = {
	{"state", ecs_id(ecs_u32_t)},
	{"action", ecs_id(ecs_u32_t)},
	{"e", ecs_id(ecs_uptr_t)},
	{"c", ecs_id(ecs_uptr_t)},
	{"s", ecs_id(ecs_uptr_t)},
	{"we", ecs_id(ecs_uptr_t)},
	}
	});

}
