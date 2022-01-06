#pragma once
#include "common/dynamic_buffer.h"
#include "smart_coro.h"
#include <sys/socket.h>
#include <unistd.h>
int smart_routine_start(void *(*fn)(void *), void *args);

int smart_routine_yield();

int smart_routine_resume(smartroutine::SmartCoro *coro);

// TODO: Maybe hook glibc api
int smart_socket(int domain, int type, int protocol);

ssize_t smart_read(int fd, void *buf, size_t count);

ssize_t smart_read_until(int fd, smartroutine::DynamicBuffer &buffer,
                         std::string delim, smartroutine::ErrorCode &ec);

ssize_t smart_write(int fd, const void *buf, size_t count);

int smart_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int smart_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
