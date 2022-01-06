#pragma once
#include "common/dynamic_buffer.h"
#include "epoll_item.h"
#include "smart_coro.h"
#include <memory>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
namespace smartroutine {

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
    size_t ret_ = 0;
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
    size_t ret_ = 0;
    int errno_ = 0;
};

struct ConnectContext {
    ConnectContext(int sockfd, SmartCoro *coro, EPollItem *epoll_item)
        : sockfd_(sockfd), coro_(coro), epoll_item_(epoll_item) {}
    int sockfd_;
    SmartCoro *coro_;
    EPollItem *epoll_item_;
    int errno_ = 0;
};

struct AcceptContext {
    AcceptContext(int sockfd, struct sockaddr *addr, socklen_t *addrlen,
                  SmartCoro *coro, EPollItem *epoll_item)
        : sockfd_(sockfd), addr_(addr), addrlen_(addrlen), coro_(coro),
          epoll_item_(epoll_item) {}
    int sockfd_;
    struct sockaddr *addr_;
    socklen_t *addrlen_;
    SmartCoro *coro_;
    EPollItem *epoll_item_;
    int errno_ = 0;
    int ret_ = -1;
};

struct ReadUntilContext {
    ReadUntilContext(int fd, DynamicBuffer &buffer, std::string delim,
                     SmartCoro *coro, EPollItem *epoll_item)
        : fd_(fd), buffer_(buffer), delim_(delim), coro_(coro),
          epoll_item_(epoll_item) {}
    int fd_;
    DynamicBuffer &buffer_;
    std::string delim_;
    SmartCoro *coro_;
    EPollItem *epoll_item_;
    ErrorCode ec_;
    int ret_ = -1;
    size_t last_search_end_ = 0;
};

void smart_read_impl(ReadContext *context);

void smart_read_until_impl(ReadUntilContext *context);

void smart_write_impl(WriteContext *context);

void smart_connect_impl(ConnectContext *context);

void smart_accept_impl(AcceptContext *context);

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

void resume_read_coro(SmartCoro *coro, EPollItem *epoll_item);
void resume_write_coro(SmartCoro *coro, EPollItem *epoll_item);

enum PartialSearchResult { Match, PartialMatch, NotMatch };

std::pair<PartialSearchResult, size_t>
partial_search(const char *search_buffer, size_t search_buffer_length,
               std::string &search_str);
}