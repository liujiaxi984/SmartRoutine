#pragma once
#include "epoll_item.h"
namespace smartroutine {

typedef std::function<void *(void *)> TimerCallback;

class TimerItem {
  public:
    TimerItem(TimerCallback callback, void *args, uint64_t timestamp)
        : callback_(callback), args_(args), timestamp_(timestamp) {}
    TimerCallback callback_;
    void *args_;
    uint64_t timestamp_;
};
}