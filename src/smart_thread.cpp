#include "smart_thread.h"
#include "smart_runtime.h"
#include <gflags/gflags.h>
#include <unistd.h>

__thread SmartThread *tls_smart_thread = nullptr;

DEFINE_uint64(task_queue_length, 25, "thread task queue length");

DEFINE_uint64(acquire_tasks_batch_size, 10,
              "how many tasks acquire from runtime each time");

void SmartThread::coro_runner(intptr_t placeholder) {
    SmartCoro *curr_coro = tls_smart_thread->current_coro_;
    if (curr_coro) {
        curr_coro->fn_(curr_coro->args_);
    }
}

SmartThread::SmartThread() : task_queue_(FLAGS_task_queue_length) {
    main_coro_ = new SmartCoro(nullptr, nullptr);
    current_coro_ = main_coro_;
}

SmartThread::~SmartThread() {
    while (!task_queue_.empty()) {
        delete task_queue_.front();
        task_queue_.pop();
    }
    delete main_coro_;
}

int SmartThread::main_loop() {
    SmartRuntime::get_instance().wait_on_task_list();
    while (true) {
        SmartCoro *coro = get_task();
        if (coro == nullptr) {
            // FIXME: Maybe it can steal from other threads
            usleep(1000 * 1000);
            continue;
        }
    }
}

int SmartThread::switch_to(SmartCoro *next_coro) {}

SmartCoro *SmartThread::get_task() {
    if (task_queue_.empty()) {
        std::list<SmartCoro *> tmp_list;
        int ret = SmartRuntime::get_instance().get_tasks(
            FLAGS_acquire_tasks_batch_size, tmp_list);
        if (ret == 0)
            return nullptr;
        for (auto coro : tmp_list) {
            task_queue_.push(coro);
        }
    }
    auto coro = task_queue_.front();
    task_queue_.pop();
    return coro;
}

int SmartThread::push_task(SmartCoro *coro) {
    bool success = task_queue_.push(coro);
    if (!success)
        return SmartRuntime::get_instance().push_task(coro);
    return 0;
}