#include "eg_thread.h"

ECS_COMPONENT_DECLARE(EgThread);


ECS_CTOR(EgThread, ptr, {
ecs_trace("EgThread::ECS_CTOR");
ptr->arg = ecs_os_calloc(sizeof(eg_callback_arg_t));
});

ECS_DTOR(EgThread, ptr, {
ecs_trace("EgThread::ECS_DTOR");
if(ptr->arg){ecs_os_free(ptr->arg);ptr->arg=NULL;}
});

ECS_MOVE(EgThread, dst, src, {
ecs_trace("EgThread::ECS_MOVE");
dst->arg = src->arg;
src->arg = NULL;
})

ECS_COPY(EgThread, dst, src, {
ecs_trace("EgThread::ECS_COPY %p %p", dst->arg, src->arg);
//ecs_os_memcpy(dst->arg, src->arg, sizeof(eg_callback_arg_t));
*dst->arg = *src->arg;
});




void eg_thread_start(ecs_world_t * world, ecs_entity_t e, ecs_os_thread_callback_t callback)
{
	ecs_bool_t is_added;
	EgThread * t = ecs_get_mut(world, e, EgThread, &is_added);
	if (t->arg == NULL)
	{
		ecs_warn("Null arg");
		return;
	};
	t->arg->world = world;
	t->arg->entity = e;
	if (callback)
	{
		t->arg->callback = callback;
	}
	if (t->arg->callback == NULL)
	{
		ecs_warn("Null callback");
		return;
	};

	t->state = 1;
	t->arg->state = t->state;


	ecs_progress(world, 0);
	ecs_progress(world, 0);
	ecs_progress(world, 0);
	EgThread const * t2 = ecs_get(t->arg->world, t->arg->entity, EgThread);
	ecs_trace("%p", t2);


	t->arg->thread = ecs_os_thread_new(t->arg->callback, t->arg);
}




void FlecsComponentsThreadImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsThread);
	ECS_COMPONENT_DEFINE(world, EgThread);

	ecs_set_name_prefix(world, "Eg");

	ecs_set_component_actions(world, EgThread, {
	.ctor = ecs_ctor(EgThread),
	.dtor = ecs_dtor(EgThread),
	.copy = ecs_copy(EgThread),
	.move = ecs_move(EgThread),
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgThread),
	.members = {
	{ .name = "state", .type = ecs_id(ecs_u32_t) },
	{ .name = "arg", .type = ecs_id(ecs_uptr_t) }
	}
	});

}
