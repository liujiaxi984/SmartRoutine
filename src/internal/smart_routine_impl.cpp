#include "internal/smart_routine_impl.h"
#include "epoll_item.h"
#include "smart_routine.h"
#include "smart_thread.h"
#include <cstring>
#include <errno.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <unistd.h>

extern __thread smartroutine::SmartThread *tls_smart_thread;

namespace smartroutine {

void smart_read_impl(ReadContext *context) {
    int ret = read(context->fd_, (char *)context->buf_ + context->ret_,
                   context->count_ - context->ret_);
    if (ret > 0) {
        context->ret_ += ret;
        if (context->ret_ == context->count_) { // read enough
            resume_read_coro(context->coro_, context->epoll_item_);
            return;
        }
    } else if (ret < 0 && errno != EWOULDBLOCK) { // error happened
        context->ret_ = -1;
        context->errno_ = errno;
        resume_read_coro(context->coro_, context->epoll_item_);
    } else if (ret == 0) { // eof
        resume_read_coro(context->coro_, context->epoll_item_);
    }
}

void smart_write_impl(WriteContext *context) {
    int ret = write(context->fd_, (char *)context->buf_ + context->ret_,
                    context->count_ - context->ret_);
    if (ret > 0) {
        context->ret_ += ret;
        if (context->ret_ == context->count_) { // write enough
            resume_write_coro(context->coro_, context->epoll_item_);
        }
    } else if (ret < 0 && errno != EWOULDBLOCK) { // error happened
        context->ret_ = -1;
        context->errno_ = errno;
        resume_write_coro(context->coro_, context->epoll_item_);
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

    resume_write_coro(context->coro_, context->epoll_item_);
}

void smart_accept_impl(AcceptContext *context) {
    int ret = accept(context->sockfd_, context->addr_, context->addrlen_);
    if (ret < 0 && errno != EWOULDBLOCK) { // error happened
        context->ret_ = -1;
        context->errno_ = errno;
        resume_read_coro(context->coro_, context->epoll_item_);
    } else if (ret >= 0) { // accept success
        context->ret_ = ret;
        resume_read_coro(context->coro_, context->epoll_item_);
    }
}

void smart_read_until_impl(ReadUntilContext *context) {
    const size_t BUFFER_SIZE = 64;
    char tmp_buf[BUFFER_SIZE];
    bool need_resume = false;
    DynamicBuffer &dynamic_buffer = context->buffer_;
    ErrorCode &ec = context->ec_;
    int ret = read(context->fd_, tmp_buf, BUFFER_SIZE);
    if (ret > 0) {
        context->ret_ += ret;
        dynamic_buffer.append(tmp_buf, ret, ec);
        if (ec) { // buffer writable size is not enough, it might cause data
                  // loss
            context->ret_ = -1;
            need_resume = true;
        }
        ConstBuffer const_buffer = dynamic_buffer.read_buffer();
        const char *search_buffer =
            (char *)const_buffer.data() + context->last_search_end_;
        size_t search_buffer_length =
            const_buffer.size() - context->last_search_end_;
        std::pair<smartroutine::PartialSearchResult, size_t> search_result =
            partial_search(search_buffer, search_buffer_length,
                           context->delim_);
        switch (search_result.first) {
        case Match:
            need_resume = true;
            break;
        case PartialMatch:
            context->last_search_end_ += search_result.second;
            break;
        case NotMatch:
            context->last_search_end_ = const_buffer.size();
            break;
        default:
            break;
        }
    } else if (ret < 0 && errno != EWOULDBLOCK) { // error happened
        context->ret_ = -1;
        LOG(ERROR) << "::read error: " << strerror(errno);
        need_resume = true;
    } else if (ret == 0) { // eof
        need_resume = true;
    }

    if (need_resume)
        resume_read_coro(context->coro_, context->epoll_item_);
}

std::pair<PartialSearchResult, size_t>
partial_search(const char *search_buffer, size_t search_buffer_length,
               std::string &search_str) {
    for (unsigned int i = 0; i < search_buffer_length; i++) {
        unsigned int test_i = i;
        for (unsigned int j = 0; j < search_str.size(); j++, test_i++) {
            if (search_buffer[test_i] == search_str[j]) {
                if (j == search_str.size() - 1) {
                    return std::pair<PartialSearchResult, size_t>(Match, i);
                } else if (test_i == search_buffer_length - 1) {
                    return std::pair<PartialSearchResult, size_t>(PartialMatch,
                                                                  i);
                }
            } else {
                break;
            }
        }
    }
    return std::pair<PartialSearchResult, size_t>(NotMatch, 0);
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

void resume_read_coro(SmartCoro *coro, EPollItem *epoll_item) {
    epoll_item->disable_reading();
    smart_routine_resume(coro);
}

void resume_write_coro(SmartCoro *coro, EPollItem *epoll_item) {
    epoll_item->disable_writing();
    smart_routine_resume(coro);
}
}