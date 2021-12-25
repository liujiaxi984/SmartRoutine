#pragma once
#include "common/bounded_queue.h"
#include "smart_coro.h"
#include <list>

class SmartThread {
  public:
    SmartThread();
    ~SmartThread();
    int main_loop();
    static void coro_runner(intptr_t placeholder);
    SmartCoro *get_task();
    int push_task(SmartCoro *coro);
    int switch_to(SmartCoro *next_coro);

  private:
    SmartCoro *main_coro_;
    SmartCoro *current_coro_;
    BoundedQueue<SmartCoro *> task_queue_;
};