#include "eg_net.h"
#include "eg_thread.h"
#include "eg_socket.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>


ECS_COMPONENT_DECLARE(EgNetProtocol1);

// |e|e|
// 0 : entity1, c
// 1 : entity2,
// 2 : entity3,
// 3 : entity4,
// | 1 | 1  | 500 bytes
// | e | c | payload


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
void * the_thread(eg_callback_arg_t * arg)
{
	ecs_bool_t is_added;
	EgThread * t = ecs_get_mut(arg->world, arg->entity, EgThread, &is_added);
	//eg_thread_action(t, eg_thread_status_running);
	while(1)
	{
		if (arg->state == eg_thread_status_stopping)
		{
			break;
		};
		ecs_sleepf(1.0f);
		ecs_trace("Hello");
	}
	//eg_thread_action(t, eg_thread_status_standby);

	/*
	//ecs_os_thread_t thread = ecs_os_thread_new(http_server_thread, srv);
	SOCKET s;
	char buf[BUFLEN];
	int recv_len;
	int slen;
	struct sockaddr_in si_other;
	while(1)
	{
		printf("Waiting for data...");
		fflush(stdout);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			ecs_fatal("recvfrom()");
		}

		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n" , buf);

		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			ecs_fatal("sendto()");
		}
	}
	*/

}




void observer1(ecs_iter_t *it)
{
	EgNetProtocol1 *p = ecs_term(it, EgNetProtocol1, 1);
	EgSocketUDP *s = ecs_term(it, EgSocketUDP, 2);
	EgThread *t = ecs_term(it, EgThread, 3);
	for (int i = 0; i < it->count; i ++)
	{
		eg_thread_set_callback(t + i, (ecs_os_thread_callback_t)the_thread);
		eg_thread_action(t + i, eg_thread_status_starting);
	}
}




void FlecsComponentsNetImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsNet);
	ecs_set_name_prefix(world, "Eg");
	ECS_COMPONENT_DEFINE(world, EgNetProtocol1);
	ECS_OBSERVER(world, observer1, EcsOnAdd, EgNetProtocol1, EgSocketUDP, EgThread);
}
