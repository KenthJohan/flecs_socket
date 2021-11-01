#pragma once


#include <stdint.h>


typedef struct {
	int fd;
} ecs_fd_t;

typedef struct {
	char bytes[128];
} ecs_sockaddr_t;

typedef struct {
	char text[65];
} ecs_sockaddr_str_t;


void ecs_fd_init();
ecs_fd_t ecs_fd_bind(const char *ipaddr);
ecs_fd_t ecs_fd_accept(ecs_fd_t s, ecs_sockaddr_t *addr);
void ecs_fd_connect(ecs_fd_t client, const char *url);
void ecs_fd_listen(ecs_fd_t server);
