#pragma once
#include "smart_coro.h"
#include <sys/socket.h>
#include <unistd.h>

int smart_routine_start(void *(*fn)(void *), void *args);

int smart_routine_yield();

int smart_routine_resume(SmartCoro *coro);

// TODO: Maybe hook glibc api
int smart_socket(int domain, int type, int protocol);

ssize_t smart_read(int fd, void *buf, size_t count);

ssize_t smart_write(int fd, const void *buf, size_t count);