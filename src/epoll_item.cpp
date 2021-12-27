#include "epoll_item.h"
#include "smart_epoller.h"

void EPollItem::handle_events() {
    if (revents_ & EPOLLHUP) {
        if (close_callback_)
            close_callback_();
    }
    if (revents_ & (EPOLLERR)) {
        if (error_callback_)
            error_callback_();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (read_callback_)
            read_callback_();
    }
    if (revents_ & EPOLLOUT) {
        if (write_callback_)
            write_callback_();
    }
}

void EPollItem::update() { epoller_->watch_event(this); }
