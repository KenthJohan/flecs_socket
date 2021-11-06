#pragma once


#include <stdint.h>


typedef struct {
	int fd;
} ecs_fd_t;

typedef struct {
	char bytes[128];
} ecs_sockaddr_t;



void ecs_fd_init();
ecs_fd_t ecs_fd_path(const char *url, int flags);
int ecs_fd_read(ecs_fd_t fd, char *data, int size);
int ecs_fd_write(ecs_fd_t fd, char *data, int size);
int ecs_fd_readfrom(ecs_fd_t fd, char *data, int size);


ecs_fd_t ecs_socket_accept(ecs_fd_t s, ecs_sockaddr_t *addr);
void ecs_socket_connect(ecs_fd_t client, const char *url);
void ecs_socket_listen(ecs_fd_t server);


void ecs_sockaddr_to_string(ecs_sockaddr_t *addr, char *str, int length);

