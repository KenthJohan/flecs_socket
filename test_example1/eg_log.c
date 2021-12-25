#include "eg_log.h"


typedef struct
{
	char const * value;
} EgMsg;

typedef struct
{
	char const * value;
} EgPath;

typedef struct
{
	ecs_i32_t value;
} EgLine;

typedef struct
{
	ecs_i32_t value;
} EgLevel;




static ECS_COPY(EgMsg, dst, src, {
printf("EgMsg::ECS_COPY %s %s\n", dst, src);
ecs_os_strset((char**)&dst->value, src->value);
})

static ECS_MOVE(EgMsg, dst, src, {
printf("EgMsg::ECS_MOVE %s %s\n", dst, src);
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgMsg, ptr, {
printf("EgMsg::ECS_DTOR\n");
ecs_os_free((char*)ptr->value);
})


static ECS_COPY(EgPath, dst, src, {
printf("EgPath::ECS_COPY %s %s\n", dst, src);
ecs_os_strset((char**)&dst->value, src->value);
})

static ECS_MOVE(EgPath, dst, src, {
printf("EgPath::ECS_MOVE %s %s\n", dst, src);
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgPath, ptr, {
printf("EgPath::ECS_DTOR\n");
ecs_os_free((char*)ptr->value);
})














ECS_COMPONENT_DECLARE(EgMsg);
ECS_COMPONENT_DECLARE(EgLine);
ECS_COMPONENT_DECLARE(EgPath);
ECS_COMPONENT_DECLARE(EgLevel);
ECS_DECLARE(EgInfo);
ECS_DECLARE(EgWarning);
ECS_DECLARE(EgError);
ECS_DECLARE(EgFatal);


/*
char const * get_color(int level)
{
	switch (level)
	{
	case -1: return "";
	case -2: return ECS_YELLOW;
	case -3: return ECS_RED;
	case -4: return ECS_RED;
	default: return ECS_MAGENTA;
	}
}
*/

void EgPrint_Info(ecs_iter_t *it)
{
	EgMsg *m = ecs_term(it, EgMsg, 1);
	EgLine *l = ecs_term(it, EgLine, 2);
	EgPath *p = ecs_term(it, EgPath, 3);
	EgLevel *lvl = ecs_term(it, EgLevel, 4);
	for (int i = 0; i < it->count; i ++)
	{
		int line = l[i].value;
		//int level = lvl[i].value;
		char const * file = p[i].value;
		char const * msg = m[i].value;
		fprintf(stdout, ECS_MAGENTA"info"ECS_NORMAL": %s:%d: %s\n", file, line, msg);
		ecs_delete(it->world, it->entities[i]);
	}
}

void EgPrint_Info1(ecs_iter_t *it)
{
	EgLine *l = ecs_term(it, EgLine, 1);
	for (int i = 0; i < it->count; i ++)
	{
		int line = l[i].value;
		fprintf(stdout, "%d\n", line);
		ecs_delete(it->world, it->entities[i]);
	}
}



ecs_world_t *world;

static void eg_log_msg(int32_t level, const char *file, int32_t line, const char *msg)
{
	//ecs_strbuf_t b = ECS_STRBUF_INIT;
	ecs_entity_t e = ecs_new_id(world);
	ecs_set(world, e, EgLevel, {level});
	ecs_set(world, e, EgLine, {line});
	char * a = ecs_os_strdup("BANANA");
	ecs_set(world, e, EgPath, {a});
	/*
	ecs_set(world, e, EgMsg, {msg});
	if(level >= 0)
	{
		ecs_add(world, e, EgInfo);
	}
	else if (level == -2)
	{
		ecs_add(world, e, EgWarning);
	}
	else if (level == -3)
	{
		ecs_add(world, e, EgError);
	}
	else if (level == -4)
	{
		ecs_add(world, e, EgFatal);
	}
	*/
}


/*
void * the_thread1(void * arg)
{
	while (ecs_progress(world, 0))
	{

	}
}
*/

ecs_world_t * eg_log_init()
{
	world = ecs_init();
	ECS_COMPONENT_DEFINE(world, EgMsg);
	ECS_COMPONENT_DEFINE(world, EgLine);
	ECS_COMPONENT_DEFINE(world, EgPath);
	ECS_COMPONENT_DEFINE(world, EgLevel);
	ECS_TAG_DEFINE(world, EgInfo);
	ECS_TAG_DEFINE(world, EgWarning);
	ECS_TAG_DEFINE(world, EgError);
	ECS_TAG_DEFINE(world, EgFatal);

	ecs_set_component_actions(world, EgPath, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgPath),
	.copy = ecs_copy(EgPath),
	.dtor = ecs_dtor(EgPath)
	});
	/*
	ecs_set_component_actions(world, EgMsg, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgMsg),
	.copy = ecs_copy(EgMsg),
	.dtor = ecs_dtor(EgMsg)
	});
	*/



	ECS_SYSTEM(world, EgPrint_Info, EcsOnUpdate, EgMsg, EgLine, EgPath, EgLevel);
	ECS_SYSTEM(world, EgPrint_Info1, EcsOnUpdate, EgLine);

	ecs_os_api.log_ = eg_log_msg;

	//ecs_os_thread_t t = ecs_os_thread_new(the_thread1, NULL);
	return world;
}



