#include "eg_thread.h"

ECS_COMPONENT_DECLARE(EgThread);



void eg_thread_set_state_text(ecs_u32_t state, ecs_string_t * text)
{
	switch (state)
	{
	case eg_thread_status_undefined:
		ecs_os_strset(text, "Undefined");
		break;
	case eg_thread_status_empty:
		ecs_os_strset(text, "Empty");
		break;
	case eg_thread_status_standby:
		ecs_os_strset(text, "Standby");
		break;
	case eg_thread_status_starting:
		ecs_os_strset(text, "Starting");
		break;
	case eg_thread_status_running:
		ecs_os_strset(text, "Running");
		break;
	case eg_thread_status_stopping:
		ecs_os_strset(text, "Stopping");
		break;
	}
}


ECS_DTOR(EgThread, ptr, {
ecs_trace("EgThread::ECS_DTOR");
ecs_os_free(ptr->arg);
ecs_os_free(ptr->debug_text);
ptr->arg = NULL;
ptr->debug_text = NULL;
});

ECS_MOVE(EgThread, dst, src, {
ecs_trace("EgThread::ECS_MOVE");
ecs_os_free(dst->arg);
ecs_os_free(dst->debug_text);
dst->arg = src->arg;
dst->debug_text = src->debug_text;
src->arg = NULL;
src->debug_text = NULL;
})

ECS_COPY(EgThread, dst, src, {
ecs_trace("EgThread::ECS_COPY1 %p %p : %p %p", dst, dst->arg, src, src->arg);
if(src->arg)
{
ecs_os_free(dst->arg);
dst->arg = ecs_os_calloc_t(eg_callback_arg_t);
ecs_os_memcpy_t(dst->arg, src->arg, eg_callback_arg_t);
ecs_trace("EgThread::ECS_COPY2 %p %p : %p %p", dst, dst->arg, src, src->arg);
}
ecs_os_strset((char**)&dst->debug_text, src->debug_text);

});



static
void EgThread_onset(
ecs_world_t *world,
ecs_entity_t component,
const ecs_entity_t *entities,
void *ptr,
size_t size,
int32_t count,
void *ctx)
{
	EgThread *th = ptr;
	(void)component;
	(void)size;
	(void)ctx;
	int i;
	for(i = 0; i < count; i ++)
	{
		ecs_trace("EgThread_onset %i", i);
		th[i].arg = ecs_os_calloc_t(eg_callback_arg_t);
		th[i].arg->world = world;
		th[i].arg->entity = entities[i];
		eg_thread_action(th + i, eg_thread_status_empty);
	}
}



void eg_thread_set_callback(EgThread * t, ecs_os_thread_callback_t callback)
{
	ecs_assert(t, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(t->arg, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(t->arg->world, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(t->arg->entity, ECS_INVALID_PARAMETER, NULL);
	t->arg->state = eg_thread_status_standby;
	t->arg->callback = callback;
	eg_thread_action(t, t->arg->state);
}

void eg_thread_action(EgThread * t, uint32_t state)
{
	//ecs_world_t * world = t->arg->world;
	//ecs_entity_t entity = t->arg->entity;
	ecs_assert(t, ECS_INVALID_PARAMETER, NULL);
	ecs_assert(t->arg, ECS_INVALID_PARAMETER, NULL);

	switch (state)
	{
	case eg_thread_status_undefined:
		t->arg->state = state;
		break;
	case eg_thread_status_empty:
		t->arg->state = state;
		break;
	case eg_thread_status_standby:
		t->arg->state = state;
		break;
	case eg_thread_status_starting:
		ecs_assert((t->arg->state != state) && (t->arg->state == eg_thread_status_standby), ECS_INVALID_PARAMETER, NULL);
		t->arg->state = state;
		t->arg->thread = ecs_os_thread_new(t->arg->callback, t->arg);
		break;
	case eg_thread_status_running:
		ecs_assert((t->arg->state != state) && (t->arg->state == eg_thread_status_starting), ECS_INVALID_PARAMETER, NULL);
		t->arg->state = state;
		break;
	case eg_thread_status_stopping:
		ecs_assert((t->arg->state != state) && (t->arg->state == eg_thread_status_running), ECS_INVALID_PARAMETER, NULL);
		t->arg->state = state;
		break;
	}
	eg_thread_set_state_text(state, &t->debug_text);

}




void FlecsComponentsThreadImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsThread);
	ECS_COMPONENT_DEFINE(world, EgThread);

	ecs_set_name_prefix(world, "Eg");

	ecs_set_component_actions(world, EgThread, {
	.ctor = ecs_default_ctor,
	.dtor = ecs_dtor(EgThread),
	.copy = ecs_copy(EgThread),
	.move = ecs_move(EgThread),
	.on_set = EgThread_onset
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgThread),
	.members = {
	{ .name = "debug_text", .type = ecs_id(ecs_string_t) },
	{ .name = "arg", .type = ecs_id(ecs_uptr_t) }
	}
	});

}
