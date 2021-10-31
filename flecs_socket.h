#pragma once


#include <stdint.h>


typedef struct {
	int fd;
} ecs_fd_t;

typedef struct {
	char bytes[128];
} ecs_sockaddr_t;


void ecs_fd_init();
ecs_fd_t ecs_fd_listen(const char *ipaddr, uint16_t port);
ecs_fd_t ecs_fd_accept(ecs_fd_t s, ecs_sockaddr_t *addr);
