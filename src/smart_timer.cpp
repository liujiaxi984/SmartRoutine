#include "smart_timer.h"
#include <glog/logging.h>
#include <limits.h>
#include <sys/epoll.h>
#include <sys/time.h>

#define EPOLL_CREATE_SIZE 10000

SmartTimer::SmartTimer() : nearest_expire_timestamp_(UINT64_MAX) {}

int SmartTimer::run() {
    if ((epoll_fd_ = epoll_create(EPOLL_CREATE_SIZE)) < 0) {
        LOG(ERROR) << "epoll_create error: " << strerror(errno);
        return -1;
    }
    while (true) {
        struct epoll_event event;
        int ret = epoll_wait(epoll_fd_, &event, 1, 1);
        if (ret < 0) {
            if (errno != EINTR) {
                LOG(ERROR) << "epoll_wait error: " << strerror(errno);
                return -1;
            }
        }
        std::unique_lock<std::mutex> lk(timer_heap_lock_);
        uint64_t now = get_timestamp_now();
        if (nearest_expire_timestamp_ < now) { // timeout happened
            while (!timer_heap_.empty() && timer_heap_.top().timestamp_ < now) {
                timeout_list_.push_back(timer_heap_.top());
                timer_heap_.pop();
            }
            if (!timer_heap_.empty()) {
                nearest_expire_timestamp_ = timer_heap_.top().timestamp_;
            } else {
                nearest_expire_timestamp_ = UINT64_MAX;
            }
        }
        lk.unlock();
        for (const auto &item : timeout_list_) {
            item.callback_(item.args_);
        }
        timeout_list_.clear();
    }
}

int SmartTimer::set_timer_after(TimerCallback callback, void *args,
                                struct timeval expire_time_duration) {
    uint64_t tp = get_timestamp_now();
    tp += expire_time_duration.tv_sec * 1000;
    tp += expire_time_duration.tv_usec;
    return set_timer(callback, args, tp);
}

int SmartTimer::set_timer_until(TimerCallback callback, void *args,
                                struct timeval expire_time_point) {
    uint64_t now = get_timestamp_now();
    uint64_t tp = get_timestamp(&expire_time_point);
    if (tp < now) {
        return -1;
    }
    return set_timer(callback, args, tp);
}

int SmartTimer::set_timer(TimerCallback callback, void *args,
                          uint64_t timestamp) {
    std::lock_guard<std::mutex> lk(timer_heap_lock_);
    if (nearest_expire_timestamp_ > timestamp) {
        nearest_expire_timestamp_ = timestamp;
    }
    timer_heap_.emplace(callback, args, timestamp);
    return 0;
}

uint64_t SmartTimer::get_timestamp_now() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return get_timestamp(&now);
}

uint64_t SmartTimer::get_timestamp(struct timeval *tv) {
    return tv->tv_sec * 1000 + tv->tv_usec;
}