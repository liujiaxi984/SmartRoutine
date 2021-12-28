#pragma once
#include "smart_epoller.h"
#include "smart_thread.h"
#include "smart_timer.h"
#include <condition_variable>
#include <list>
#include <mutex>
#include <vector>

class SmartRuntime {
  public:
    int push_task(SmartCoro *coro);
    int get_tasks(unsigned int batch_size, std::list<SmartCoro *> &tasks);
    int wait_on_task_list();
    SmartEPoller &get_epoller();
    SmartTimer &get_timer();
    // singleton
    static SmartRuntime &get_instance();

    SmartRuntime(const SmartRuntime &) = delete;
    SmartRuntime &operator=(const SmartRuntime &) = delete;

  private:
    SmartRuntime();
    ~SmartRuntime();
    static void init();
    static void exit_clean();

  private:
    unsigned int threads_num_;
    SmartThread *threads_;
    std::mutex task_list_lock_;
    std::condition_variable task_list_cond_;
    std::list<SmartCoro *> task_list_;
    static SmartRuntime *value_;
    static pthread_once_t once_control_;
    SmartEPoller epoller_;
    pthread_t epoller_tid_;
    SmartTimer timer_;
    pthread_t timer_tid_;
    std::vector<pthread_t> tids_;
};