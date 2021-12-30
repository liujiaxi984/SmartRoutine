#include "smart_thread.h"
#include "internal/libcontext.h"
#include "smart_runtime.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <unistd.h>

__thread SmartThread *tls_smart_thread = nullptr;

DEFINE_uint64(task_queue_length, 25, "thread task queue length");

DEFINE_uint64(acquire_tasks_batch_size, 10,
              "how many tasks acquire from runtime each time");

void SmartThread::coro_runner(intptr_t placeholder) {
    SmartThread *tls_st = tls_smart_thread;
    if (tls_st->runner_callback_) {
        tls_st->runner_callback_(tls_st->runner_callback_args_);
        tls_st->runner_callback_ = nullptr;
        tls_st->runner_callback_args_ = nullptr;
    }
    SmartCoro *curr_coro = tls_st->current_coro_;
    if (curr_coro && curr_coro->fn_) {
        curr_coro->fn_(curr_coro->args_);
    }
    tls_st->runner_callback_ = destroy_coro;
    tls_st->runner_callback_args_ = curr_coro;
    tls_st->main_loop();
}

SmartThread::SmartThread()
    : task_queue_(FLAGS_task_queue_length), runner_callback_(nullptr),
      runner_callback_args_(nullptr) {
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

int SmartThread::init() {
    SmartRuntime::get_instance().wait_on_task_list();
    return main_loop();
}

int SmartThread::main_loop() {
    while (true) {
        SmartCoro *coro = get_task();
        if (coro == nullptr) {
            // TODO: Maybe it can steal from other threads
            usleep(1000 * 1000);
            continue;
        }
        switch_to(coro);
        if (runner_callback_) {
            runner_callback_(runner_callback_args_);
            runner_callback_ = nullptr;
            runner_callback_args_ = nullptr;
        }
    }
}

int SmartThread::yield(bool with_callback, RunnerCallback runner_callback,
                       void *runner_callback_args) {
    if (with_callback) {
        runner_callback_ = runner_callback;
        runner_callback_args_ = runner_callback_args;
    }
    SmartCoro *coro = tls_smart_thread->get_task();
    if (coro != nullptr) {
        tls_smart_thread->switch_to(coro);
    } else {
        tls_smart_thread->switch_to(main_coro_);
    }
    return 0;
}

SmartCoro *SmartThread::get_current_coro() { return current_coro_; }

void SmartThread::destroy_coro(void *args) {
    SmartCoro *coro = (SmartCoro *)args;
    delete coro;
}

int SmartThread::switch_to(SmartCoro *next_coro) {
    int saved_errno = errno; // save errno because it is a tls variable
    fcontext_t *curr_fcontext = &current_coro_->context_.fcontext_;
    current_coro_ = next_coro;
    int ret = jump_fcontext(curr_fcontext, next_coro->context_.fcontext_, 0);
    errno = saved_errno;
    return ret;
}

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