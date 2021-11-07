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


void onopen(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
	printf("Connection opened, client: %d | addr: %s\n", fd, cli);
	free(cli);
}


void onclose(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
	printf("Connection closed, client: %d | addr: %s\n", fd, cli);
	free(cli);
}


void onmessage(int fd, const unsigned char *msg, uint64_t size, int type)
{
	char *cli;
	cli = ws_getaddress(fd);
	printf("I receive a message: %s (size: %" PRId64 ", type: %d), from: %s/%d\n",msg, size, type, cli, fd);
	free(cli);
	ws_sendframe(fd, (char *)msg, size, 1, type);
}


int main(int argc, char * argv[])
{
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ws_flecs_init(world);
	ecs_set_threads(world, 4);

	struct ws_events evs;
	evs.onopen    = &onopen;
	evs.onclose   = &onclose;
	evs.onmessage = &onmessage;
	ws_socket(&evs, 8080, 1, world);

	while(1)
	{
		ecs_progress(world, 0.0f);
		ecs_os_sleep(1, 0);
	}

	return (0);
}
