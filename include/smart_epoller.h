#pragma once
#include "epoll_item.h"
#include <list>

class SmartEPoller {
  public:
    SmartEPoller();
    ~SmartEPoller();
    int watch_event(EPollItem *item);
    int eventloop();
    int init();

  private:
    int epoll_fd_;
    int maxevents_;
    struct epoll_event *evlist_;
    std::list<EPollItem *> ready_list_;
    int epoll_timeout_;
};