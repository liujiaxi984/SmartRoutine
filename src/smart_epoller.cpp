#include "smart_epoller.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/epoll.h>

#define EPOLL_CREATE_SIZE 10000
#define DEFAULT_EVLIST_SIZE 10000
#define DEFAULT_EPOLL_TIMEOUT 1000

DEFINE_int64(evlist_size, DEFAULT_EVLIST_SIZE,
             "The maximum number of events that can be "
             "returned from epoll_wait");

SmartEPoller::SmartEPoller()
    : maxevents_(DEFAULT_EVLIST_SIZE), evlist_(nullptr),
      epoll_timeout_(DEFAULT_EPOLL_TIMEOUT) {
} // TODO: set epoll_timeout_ based on timer's need

int SmartEPoller::init() {
    if (epoll_create(EPOLL_CREATE_SIZE) < 0) {
        LOG(ERROR) << "epoll_create error: " << strerror(errno);
        return -1;
    }
    if (FLAGS_evlist_size > 0) {
        maxevents_ = FLAGS_evlist_size;
    }
    evlist_ = new epoll_event[maxevents_];
}

SmartEPoller::~SmartEPoller() {
    if (evlist_ != nullptr)
        delete[] evlist_;
}

int SmartEPoller::watch_event(EPollItem *item) {
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    if (item->is_watching()) {
        if (item->is_disabled()) {
            epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, item->fd(), NULL);
            item->clear_watching();
        } else {
            event.events = item->events();
            event.data.ptr = item;
            epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, item->fd(), &event);
        }
    } else {
        event.events = item->events();
        event.data.ptr = item;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, item->fd(), &event);
        item->set_watching();
    }
}

int SmartEPoller::eventloop() {
    while (true) {
        int ready_num =
            epoll_wait(epoll_fd_, evlist_, maxevents_, epoll_timeout_);
        if (ready_num < 0) {
            if (errno != EINTR) {
                LOG(ERROR) << "epoll_wait error: " << strerror(errno);
                return -1;
            }
        } else if (ready_num == 0) {
            DLOG(INFO) << "epoll_wait timeout";
        } else {
            ready_list_.clear();
            for (int i = 0; i < ready_num; i++) {
                EPollItem *epoll_item = (EPollItem *)evlist_[i].data.ptr;
                epoll_item->set_revents(evlist_[i].events);
                ready_list_.push_back(epoll_item);
            }
        }
        for (EPollItem *ready_item : ready_list_) {
            ready_item->handle_events();
        }
    }
}