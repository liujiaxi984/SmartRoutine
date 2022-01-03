#pragma once
#include "time.h"
#include "timer_item.h"
#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <vector>
namespace smartroutine {

class TimerComp {
  public:
    bool operator()(const TimerItem &lhs, const TimerItem &rhs) const {
        return lhs.timestamp_ > rhs.timestamp_;
    }
};

typedef std::priority_queue<TimerItem, std::vector<TimerItem>, TimerComp>
    TimerHeap;

class SmartTimer {
  public:
    SmartTimer();
    /**
     * @brief call callback after at least expire_time_duration has passed from
     * now
     *
     * @param callback
     * @param args
     * @param expire_time_duration
     * @return int
     */
    int set_timer_after(TimerCallback callback, void *args,
                        struct timeval expire_time_duration);

    /**
     * @brief call callback when expire_time_point has been reached
     *
     * @param callback
     * @param args
     * @param expire_time_point
     * @return int
     */
    int set_timer_until(TimerCallback callback, void *args,
                        struct timeval expire_time_point);
    int run();

  private:
    static uint64_t get_timestamp_now();
    static uint64_t get_timestamp(struct timeval *tv);
    int set_timer(TimerCallback callback, void *args, uint64_t timestamp);

  private:
    int epoll_fd_;
    std::mutex timer_heap_lock_;
    TimerHeap timer_heap_;
    uint64_t nearest_expire_timestamp_;
    std::list<TimerItem> timeout_list_;
};
}