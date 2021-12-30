#include "eg_thread.h"
#include "eg_basic.h"

ECS_COMPONENT_DECLARE(EgThread);


static void * the_thread(void * arg)
{
	ecs_world_t * world = ((struct eg_we *)arg)->world;
	ecs_entity_t entity = ((struct eg_we *)arg)->entity;
	ecs_world_t *async = ecs_async_stage_new(world);
	while(1)
	{
		ecs_os_sleep(1, 0);
		ecs_trace("Hello");
		ecs_entity_t e = ecs_new(async, 0);
		ecs_set(async, e, EgState, {EG_STATE_INITIAL, NULL});
	}
}



void FlecsComponentsEgThreadImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgThread);
	ECS_COMPONENT_DEFINE(world, EgThread);

	ecs_set_name_prefix(world, "Eg");

	ecs_entity_t e = ecs_new(world, 0);
	struct eg_we * we = ecs_os_calloc_t(struct eg_we);
	we->world = world;
	we->entity = e;

	ecs_os_thread_t thread = ecs_os_thread_new(the_thread, we);
	//ecs_set(world, 0, EgThread, {world, e, thread});

	//the_thread(we);
	//ecs_entity_t e1 = ecs_new(world, 0);
	//ecs_set(world, e1, EgState, {EG_STATE_INITIAL, "hej"});
	//ecs_entity_t e2 = ecs_new(world, 0);
	//ecs_set(world, e2, EgState, {EG_STATE_INITIAL, "hej"});

}
