#pragma once
#include "common/bounded_queue.h"
#include "smart_coro.h"
#include <functional>
#include <list>
namespace smartroutine {

typedef std::function<void(void *)> DelayCallback;

class SmartThread {
  public:
    SmartThread();
    ~SmartThread();
    int init();
    int main_loop();
    static void coro_runner(intptr_t placeholder);
    SmartCoro *get_task();
    int push_task(SmartCoro *coro);
    int yield(bool with_callback, DelayCallback runner_callback = nullptr,
              void *runner_callback_args = nullptr);
    SmartCoro *get_current_coro();

  private:
    int switch_to(SmartCoro *next_coro);
    static void destroy_coro(void *args);

  private:
    SmartCoro *main_coro_;
    SmartCoro *current_coro_;
    BoundedQueue<SmartCoro *> task_queue_;
    DelayCallback delay_callback_;
    void *delay_callback_args_;
};
}