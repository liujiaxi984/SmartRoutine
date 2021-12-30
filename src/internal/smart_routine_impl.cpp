#include "internal/smart_routine_impl.h"
#include "epoll_item.h"
#include "smart_routine.h"
#include "smart_thread.h"
#include <errno.h>
#include <glog/logging.h>
#include <string.h>

extern __thread SmartThread *tls_smart_thread;

void smart_read_impl(ReadContext *context) {
    int ret = read(context->fd_, (char *)context->buf_ + context->curr_,
                   context->count_ - context->curr_);
    if (ret > 0) {
        context->curr_ += ret;
        if (context->curr_ == context->count_) { // read enough
            context->epoll_item_->disable_reading();
            smart_routine_resume(context->coro_);
            return;
        }
    } else if (ret < 0 && errno != EWOULDBLOCK) { // error happened
        context->curr_ = -1;
        context->errno_ = errno;
        context->epoll_item_->disable_reading();
        smart_routine_resume(context->coro_);
    } else if (ret == 0) { // eof
        context->epoll_item_->disable_reading();
        smart_routine_resume(context->coro_);
    }
}

void smart_write_impl(WriteContext *context) {
    int ret = write(context->fd_, (char *)context->buf_ + context->curr_,
                    context->count_ - context->curr_);
    if (ret > 0) {
        context->curr_ += ret;
        if (context->curr_ == context->count_) { // write enough
            context->epoll_item_->disable_writing();
            smart_routine_resume(context->coro_);
        }
    } else if (ret < 0 && errno != EWOULDBLOCK) { // error happened
        context->curr_ = -1;
        context->errno_ = errno;
        context->epoll_item_->disable_writing();
        smart_routine_resume(context->coro_);
    }
}

void smart_connect_impl(ConnectContext *context) {
    int error = 0;
    socklen_t len = sizeof(error);
    int ret = getsockopt(context->sockfd_, SOL_SOCKET, SO_ERROR, &error, &len);
    if (ret < 0) {
        LOG(ERROR) << "getsockopt: " << strerror(errno);
        context->errno_ = errno;
    } else if (error != 0) { // connect fail
        context->errno_ = error;
    } // else connect success

    context->epoll_item_->disable_writing();
    smart_routine_resume(context->coro_);
}

void smart_accept_impl(AcceptContext *context) {
    int ret = accept(context->sockfd_, context->addr_, context->addrlen_);
    if (ret < 0 && errno != EWOULDBLOCK) { // error happened
        context->ret_ = -1;
        context->errno_ = errno;
        context->epoll_item_->disable_reading();
        smart_routine_resume(context->coro_);
    } else if (ret >= 0) { // accept success
        context->ret_ = ret;
        context->epoll_item_->disable_reading();
        smart_routine_resume(context->coro_);
    }
}

void add_back_to_queue(void *args) {
    SmartCoro *coro = (SmartCoro *)args;
    tls_smart_thread->push_task(coro);
}

void enable_epoll_reading(void *args) {
    EPollItem *item = (EPollItem *)args;
    item->enable_reading();
}

void enable_epoll_writing(void *args) {
    EPollItem *item = (EPollItem *)args;
    item->enable_writing();
}