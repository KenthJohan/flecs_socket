#include "flecs_basic.h"


ECS_COMPONENT_DECLARE(EgThread);
ECS_COMPONENT_DECLARE(EgBuffer);




ECS_CTOR(EgBuffer, ptr, {
ecs_trace("EgBuffer::ECS_CTOR");
ptr->memory = NULL;
ptr->size = 0;
});

ECS_DTOR(EgBuffer, ptr, {
ecs_trace("EgBuffer::ECS_DTOR");
if(ptr->memory){ecs_os_free(ptr->memory);}
ptr->size = 0;
});

ECS_MOVE(EgBuffer, dst, src, {
ecs_trace("EgBuffer::ECS_MOVE");
dst->memory = src->memory;
dst->size = src->size;
src->memory = NULL;
src->size = 0;
})

ECS_COPY(EgBuffer, dst, src, {
ecs_trace("EgBuffer::ECS_COPY");
if(dst->memory){ecs_os_free(dst->memory);}
ecs_os_memcpy(dst->memory, src->memory, src->memory);
dst->size = src->size;
});





void FlecsComponentsBasicImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsBasic);
	ECS_COMPONENT_DEFINE(world, EgBuffer);
	ECS_COMPONENT_DEFINE(world, EgThread);

	ecs_set_name_prefix(world, "Eg");


	ecs_set_component_actions(world, EgBuffer, {
	.ctor = ecs_ctor(EgBuffer),
	.dtor = ecs_dtor(EgBuffer),
	.copy = ecs_copy(EgBuffer),
	.move = ecs_move(EgBuffer),
	});

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgBuffer),
	.members = {
	{ .name = "memory", .type = ecs_id(ecs_uptr_t) },
	{ .name = "size", .type = ecs_id(ecs_u32_t) }
	}
	});

}

