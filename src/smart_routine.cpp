#include "smart_routine.h"
#include "internal/smart_routine_impl.h"
#include "smart_runtime.h"
#include "smart_thread.h"
#include <fcntl.h>
#include <glog/logging.h>
#include <memory>
#include <string.h>

extern __thread smartroutine::SmartThread *tls_smart_thread;

int smart_routine_start(void *(*fn)(void *), void *args) {
    smartroutine::SmartCoro *coro = new smartroutine::SmartCoro(fn, args);
    return smart_routine_resume(coro);
}

int smart_routine_yield() {
    if (tls_smart_thread != nullptr)
        return tls_smart_thread->yield(true, smartroutine::add_back_to_queue,
                                       tls_smart_thread->get_current_coro());
    else
        return pthread_yield();
}

int smart_routine_resume(smartroutine::SmartCoro *coro) {
    if (tls_smart_thread != nullptr)
        return tls_smart_thread->push_task(coro);
    else
        return smartroutine::SmartRuntime::get_instance().push_task(coro);
}

int smart_socket(int domain, int type, int protocol) {
    if (tls_smart_thread != nullptr) {
        int fd = socket(domain, type, protocol);
        if (fd >= 0) {
            int ret = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
            if (ret < 0) {
                LOG(ERROR) << "fcntl error: " << strerror(errno);
            }
        }
        return fd;
    } else
        return socket(domain, type, protocol);
}

ssize_t smart_read(int fd, void *buf, size_t count) {
    if (tls_smart_thread != nullptr) {
        smartroutine::SmartEPoller &epoller =
            smartroutine::SmartRuntime::get_instance().get_epoller();
        smartroutine::EPollItem item(fd, &epoller);
        smartroutine::ReadContext context(
            fd, buf, count, tls_smart_thread->get_current_coro(), &item);
        item.set_read_callback(
            std::bind(smartroutine::smart_read_impl, &context));
        tls_smart_thread->yield(true, smartroutine::enable_epoll_reading,
                                &item);
        if (context.curr_ < 0)
            errno = context.errno_;
        return context.curr_;
    } else
        return read(fd, buf, count);
}

ssize_t smart_write(int fd, const void *buf, size_t count) {
    if (tls_smart_thread != nullptr) {
        smartroutine::SmartEPoller &epoller =
            smartroutine::SmartRuntime::get_instance().get_epoller();
        smartroutine::EPollItem item(fd, &epoller);
        smartroutine::WriteContext context(
            fd, buf, count, tls_smart_thread->get_current_coro(), &item);
        item.set_write_callback(
            std::bind(smartroutine::smart_write_impl, &context));
        tls_smart_thread->yield(true, smartroutine::enable_epoll_writing,
                                &item);
        if (context.curr_ < 0)
            errno = context.errno_;
        return context.curr_;
    } else
        return write(fd, buf, count);
}

int smart_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (tls_smart_thread != nullptr) {
        smartroutine::SmartEPoller &epoller =
            smartroutine::SmartRuntime::get_instance().get_epoller();
        smartroutine::EPollItem item(sockfd, &epoller);
        smartroutine::AcceptContext context(
            sockfd, addr, addrlen, tls_smart_thread->get_current_coro(), &item);
        item.set_read_callback(
            std::bind(smartroutine::smart_accept_impl, &context));
        tls_smart_thread->yield(true, smartroutine::enable_epoll_reading,
                                &item);
        if (context.ret_ < 0) {
            errno = context.errno_;
            return -1;
        } else {
            return context.ret_;
        }
    } else
        return accept(sockfd, addr, addrlen);
}

int smart_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (tls_smart_thread != nullptr) {
        int ret = connect(sockfd, addr, addrlen);
        if (ret < 0 && errno == EINPROGRESS) {
            smartroutine::SmartEPoller &epoller =
                smartroutine::SmartRuntime::get_instance().get_epoller();
            smartroutine::EPollItem item(sockfd, &epoller);
            smartroutine::ConnectContext context(
                sockfd, tls_smart_thread->get_current_coro(), &item);
            item.set_write_callback(
                std::bind(smartroutine::smart_connect_impl, &context));
            tls_smart_thread->yield(true, smartroutine::enable_epoll_writing,
                                    &item);
            if (context.errno_ != 0) {
                errno = context.errno_;
                return -1;
            } else {
                return 0;
            }
        } else {
            return ret;
        }
    } else
        return connect(sockfd, addr, addrlen);
}