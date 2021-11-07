/*
 * Copyright (C) 2016-2021 Davidson Francis <davidsondfgl@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ws.h>
#include <flecs.h>
#include "ecs_ws.h"





int main(int argc, char * argv[])
{
	eg_ws_init();
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ws_flecs_init(world);
	ecs_log_set_level(1);


	ecs_entity_t e = ecs_set_name(world, 0, "MyEntity");
	ecs_add(world, e, EgSocketTCP);
	ecs_set(world, e, EgSocketPort, {8080});
	ecs_set(world, e, EgSocketMaxconn, {8});
	ecs_set(world, e, EgSocketAcceptThread, {0});


	while(1)
	{
		ecs_progress(world, 0.0f);
		ecs_os_sleep(1, 0);
	}

	return (0);
}
