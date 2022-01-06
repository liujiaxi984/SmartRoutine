#include "smart_routine.h"
#include "internal/smart_routine_impl.h"
#include "smart_runtime.h"
#include "smart_thread.h"
#include <cstring>
#include <fcntl.h>
#include <glog/logging.h>
#include <memory>

using namespace smartroutine;

extern __thread SmartThread *tls_smart_thread;

int smart_routine_start(void *(*fn)(void *), void *args) {
    SmartCoro *coro = new SmartCoro(fn, args);
    return smart_routine_resume(coro);
}

int smart_routine_yield() {
    if (tls_smart_thread != nullptr)
        return tls_smart_thread->yield(true, add_back_to_queue,
                                       tls_smart_thread->get_current_coro());
    else
        return pthread_yield();
}

int smart_routine_resume(SmartCoro *coro) {
    if (tls_smart_thread != nullptr)
        return tls_smart_thread->push_task(coro);
    else
        return SmartRuntime::get_instance().push_task(coro);
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
        SmartEPoller &epoller = SmartRuntime::get_instance().get_epoller();
        EPollItem item(fd, &epoller);
        ReadContext context(fd, buf, count,
                            tls_smart_thread->get_current_coro(), &item);
        item.set_read_callback(std::bind(smart_read_impl, &context));
        tls_smart_thread->yield(true, enable_epoll_reading, &item);
        if (context.ret_ < 0)
            errno = context.errno_;
        return context.ret_;
    } else
        return read(fd, buf, count);
}

ssize_t smart_write(int fd, const void *buf, size_t count) {
    if (tls_smart_thread != nullptr) {
        SmartEPoller &epoller = SmartRuntime::get_instance().get_epoller();
        EPollItem item(fd, &epoller);
        WriteContext context(fd, buf, count,
                             tls_smart_thread->get_current_coro(), &item);
        item.set_write_callback(std::bind(smart_write_impl, &context));
        tls_smart_thread->yield(true, enable_epoll_writing, &item);
        if (context.ret_ < 0)
            errno = context.errno_;
        return context.ret_;
    } else
        return write(fd, buf, count);
}

int smart_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (tls_smart_thread != nullptr) {
        SmartEPoller &epoller = SmartRuntime::get_instance().get_epoller();
        EPollItem item(sockfd, &epoller);
        AcceptContext context(sockfd, addr, addrlen,
                              tls_smart_thread->get_current_coro(), &item);
        item.set_read_callback(std::bind(smart_accept_impl, &context));
        tls_smart_thread->yield(true, enable_epoll_reading, &item);
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
            SmartEPoller &epoller = SmartRuntime::get_instance().get_epoller();
            EPollItem item(sockfd, &epoller);
            ConnectContext context(sockfd, tls_smart_thread->get_current_coro(),
                                   &item);
            item.set_write_callback(std::bind(smart_connect_impl, &context));
            tls_smart_thread->yield(true, enable_epoll_writing, &item);
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

ssize_t read_until(int fd, DynamicBuffer &buffer, std::string delim,
                   ErrorCode &ec) {
    if (tls_smart_thread != nullptr) {
        SmartEPoller &epoller = SmartRuntime::get_instance().get_epoller();
        EPollItem item(fd, &epoller);
        ReadUntilContext context(fd, buffer, delim,
                                 tls_smart_thread->get_current_coro(), &item);
        item.set_read_callback(std::bind(smart_read_until_impl, &context));
        tls_smart_thread->yield(true, enable_epoll_reading, &item);
        if (context.ret_ < 0)
            ec = context.ec_;
        return context.ret_;
    } else {
        LOG(ERROR) << "pthread doesn't support read_until";
        return -1;
    }
}