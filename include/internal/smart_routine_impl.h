#pragma once
#include "epoll_item.h"
#include "smart_coro.h"
#include <memory>
#include <unistd.h>

struct ReadContext {
    ReadContext(int fd, void *buf, size_t count, SmartCoro *coro,
                EPollItem *epoll_item)
        : fd_(fd), buf_(buf), count_(count), coro_(coro),
          epoll_item_(epoll_item) {}
    int fd_;
    void *buf_;
    size_t count_;
    SmartCoro *coro_;
    EPollItem *epoll_item_;
    size_t curr_ = 0;
    int errno_ = 0;
};

struct WriteContext {
    WriteContext(int fd, const void *buf, size_t count, SmartCoro *coro,
                 EPollItem *epoll_item)
        : fd_(fd), buf_(buf), count_(count), coro_(coro),
          epoll_item_(epoll_item) {}
    int fd_;
    const void *buf_;
    size_t count_;
    SmartCoro *coro_;
    EPollItem *epoll_item_;
    size_t curr_ = 0;
    int errno_ = 0;
};

struct ConnectContext {};

void smart_read_impl(ReadContext *context);

void smart_write_impl(WriteContext *context);

void smart_connect_impl(ConnectContext *context);

/**
 * @brief deferred adding back to task queue
 *
 * @param args SmartCoro *coro
 */
void add_back_to_queue(void *args);

/**
 * @brief deferred adding to eventloop to avoid race condition between callback
 * and coroutine switch
 *
 * @param args EPollItem *item
 */
void enable_epoll_reading(void *args);

/**
 * @brief deferred adding to eventloop to avoid race condition between callback
 * and coroutine switch
 *
 * @param args EPollItem *item
 */
void enable_epoll_writing(void *args);