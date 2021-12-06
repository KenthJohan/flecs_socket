#include "flecs_geometry.h"


ECS_COMPONENT_DECLARE(EgRectangle);



void FlecsComponentsGeometryImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsGeometry);
	ECS_COMPONENT_DEFINE(world, EgRectangle);

	ecs_set_name_prefix(world, "Eg");

	ecs_struct_init(world, &(ecs_struct_desc_t) {
	.entity.entity = ecs_id(EgRectangle),
	.members = {
	{ .name = "width", .type = ecs_id(ecs_f32_t) },
	{ .name = "height", .type = ecs_id(ecs_f32_t) }
	}
	});



}

