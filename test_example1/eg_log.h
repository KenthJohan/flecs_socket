#pragma once
#include "flecs.h"

ecs_world_t * eg_log_init();
static void eg_log_msg(int32_t level, const char *file, int32_t line, const char *msg);
