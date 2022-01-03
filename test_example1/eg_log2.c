#include "eg_log.h"


ECS_COMPONENT_DECLARE(EgMsg);
ECS_COMPONENT_DECLARE(EgLine);
ECS_COMPONENT_DECLARE(EgPath);
ECS_COMPONENT_DECLARE(EgLevel);
ECS_DECLARE(EgInfo);
ECS_DECLARE(EgWarning);
ECS_DECLARE(EgError);
ECS_DECLARE(EgFatal);




static ECS_COPY(EgMsg, dst, src, {
//printf("EgMsg::ECS_COPY (%s) (%s)\n", dst->value, src->value);
ecs_os_strset((char**)&dst->value, src->value);
})

static ECS_MOVE(EgMsg, dst, src, {
//printf("EgMsg::ECS_MOVE (%s) (%s)\n", dst->value, src->value);
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgMsg, ptr, {
//printf("EgMsg::ECS_DTOR\n");
ecs_os_free((char*)ptr->value);
})


static ECS_COPY(EgPath, dst, src, {
//printf("EgPath::ECS_COPY (%s) (%s)\n", dst->value, src->value);
ecs_os_strset((char**)&dst->value, src->value);
})

static ECS_MOVE(EgPath, dst, src, {
//printf("EgPath::ECS_MOVE (%s) (%s)\n", dst->value, src->value);
ecs_os_free((char*)dst->value);
dst->value = src->value;
src->value = NULL;
})

static ECS_DTOR(EgPath, ptr, {
//printf("EgPath::ECS_DTOR\n");
ecs_os_free((char*)ptr->value);
})


















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
char const * get_text(int level)
{
	switch (level)
	{
	case -1: return "";
	case -2: return "warning";
	case -3: return "error";
	case -4: return "fatal";
	default: return "info";
	}
}

void EgPrint(ecs_iter_t *it)
{
	EgMsg *m = ecs_term(it, EgMsg, 1);
	EgLine *l = ecs_term(it, EgLine, 2);
	EgPath *p = ecs_term(it, EgPath, 3);
	EgLevel *lvl = ecs_term(it, EgLevel, 4);
	for (int i = 0; i < it->count; i ++)
	{
		int line = l[i].value;
		int level = lvl[i].value;
		char const * file = p[i].value;
		char const * msg = m[i].value;
		char const * color = get_color(level);
		char const * text = get_text(level);
		char indent[32] = {'\0'};
		if (level >= 0)
		{
			if (ecs_os_api.log_indent_)
			{
				int i;
				for (i = 0; i < ecs_os_api.log_indent_; i ++)
				{
					indent[i * 2] = '|';
					indent[i * 2 + 1] = ' ';
				}
				indent[i * 2] = '\0';
			}
		}
		if (level < 0)
		{
			fprintf(stdout, "%s%s"ECS_NORMAL": %s%s:%d: %s\n", color, text, indent, file, line, msg);
		}
		else
		{
			fprintf(stdout, "%s%s"ECS_NORMAL":%s %s\n", color, text, indent, msg);
		}
		ecs_delete(it->world, it->entities[i]);
	}
}





ecs_world_t * g_world;

static void eg_log_msg1(int32_t level, const char *file, int32_t line, const char *msg)
{
	printf("msg1: %s\n", msg);
}


static void eg_log_msg(int32_t level, const char *file, int32_t line, const char *msg)
{
	ecs_os_api_log_t logold = ecs_os_api.log_;
	ecs_os_api.log_ = eg_log_msg1;

	//ecs_strbuf_t b = ECS_STRBUF_INIT;
	//ecs_entity_t e = ecs_new_id(w);
	//ecs_set(w, e, EgLevel, {level});
	/*
	ecs_set(g_world, e, EgLine, {line});
	ecs_set(g_world, e, EgPath, {file});
	ecs_set(g_world, e, EgMsg, {msg});
	if(level >= 0)
	{
		ecs_add(g_world, e, EgInfo);
	}
	else if (level == -2)
	{
		ecs_add(g_world, e, EgWarning);
	}
	else if (level == -3)
	{
		ecs_add(g_world, e, EgError);
	}
	else if (level == -4)
	{
		ecs_add(g_world, e, EgFatal);
	}
	*/


	ecs_os_api.log_ = logold;
}


/*
void * the_thread1(void * arg)
{
	while (ecs_progress(world, 0))
	{

	}
}
*/

void FlecsComponentsEgLogImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgLog);
	ecs_set_name_prefix(world, "Eg");
	g_world = world;


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
	ecs_set_component_actions(world, EgMsg, {
	.ctor = ecs_default_ctor,
	.move = ecs_move(EgMsg),
	.copy = ecs_copy(EgMsg),
	.dtor = ecs_dtor(EgMsg)
	});
	/*
	*/



	//ECS_SYSTEM(world, EgPrint, EcsOnUpdate, EgMsg, EgLine, EgPath, EgLevel);
	//ECS_SYSTEM(world, EgPrint_Info1, EcsOnUpdate, EgLine);

	ecs_os_api.log_ = eg_log_msg;

	//ecs_os_thread_t t = ecs_os_thread_new(the_thread1, NULL);
}



