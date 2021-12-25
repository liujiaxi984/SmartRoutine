#pragma once
#include "smart_thread.h"
#include <condition_variable>
#include <list>
#include <mutex>

class SmartRuntime {
  public:
    int push_task(SmartCoro *coro);
    int get_tasks(uint batch_size, std::list<SmartCoro *> &tasks);
    int wait_on_task_list();

    static SmartRuntime &get_instance();

    SmartRuntime(const SmartRuntime &) = delete;
    SmartRuntime &operator=(const SmartRuntime &) = delete;

  private:
    SmartRuntime();
    ~SmartRuntime();
    static void init();

  private:
    uint threads_num_;
    SmartThread *threads_;
    std::mutex task_list_lock_;
    std::condition_variable task_list_cond_;
    std::list<SmartCoro *> task_list_;
    static SmartRuntime *value_;
    static pthread_once_t once_control_;
};