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

ECS_CTOR(EgThread, ptr, {
ecs_trace("EgThread::ECS_CTOR");
ptr->arg = ecs_os_calloc(sizeof(eg_callback_arg_t));
ptr->arg->state = eg_thread_status_undefined;
ptr->debug_text = NULL;
eg_thread_set_state_text(ptr->arg->state, &ptr->debug_text);
});

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
ecs_trace("EgThread::ECS_COPY %p %s", dst->arg, src->debug_text);
//ecs_os_memcpy(dst->arg, src->arg, sizeof(eg_callback_arg_t));
//*dst->arg = *src->arg;
//dst->debug_text = NULL;
//ecs_os_strset((char**)&dst->debug_text, src->debug_text);
});





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


void EgThread_trigger(ecs_iter_t *it)
{
	EgThread *t = ecs_term(it, EgThread, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_assert(t->arg, ECS_INVALID_PARAMETER, NULL);
		t->arg->world = it->world;
		t->arg->entity = it->entities[i];
		eg_thread_action(t + i, eg_thread_status_empty);
	}
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
	{ .name = "debug_text", .type = ecs_id(ecs_string_t) },
	{ .name = "arg", .type = ecs_id(ecs_uptr_t) }
	}
	});


	ECS_TRIGGER(world, EgThread_trigger, EcsOnAdd, EgThread);

}
