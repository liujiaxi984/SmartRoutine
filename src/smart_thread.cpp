#include "smart_thread.h"
#include "internal/libcontext.h"
#include "smart_runtime.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <unistd.h>

__thread smartroutine::SmartThread *tls_smart_thread = nullptr;

namespace smartroutine {

DEFINE_uint64(task_queue_length, 25, "thread task queue length");

DEFINE_uint64(acquire_tasks_batch_size, 10,
              "how many tasks acquire from runtime each time");

void SmartThread::coro_runner(intptr_t placeholder) {
    SmartThread *tls_st = tls_smart_thread;
    if (tls_st->delay_callback_) {
        tls_st->delay_callback_(tls_st->delay_callback_args_);
        tls_st->delay_callback_ = nullptr;
        tls_st->delay_callback_args_ = nullptr;
    }
    SmartCoro *curr_coro = tls_st->current_coro_;
    if (curr_coro && curr_coro->fn_) {
        curr_coro->fn_(curr_coro->args_);
    }
    DLOG(INFO) << "routine done " << curr_coro;
    tls_st->delay_callback_ = destroy_coro;
    tls_st->delay_callback_args_ = curr_coro;
    tls_st->main_loop();
    // tls_st->switch_to(tls_st->main_coro_);
}

SmartThread::SmartThread()
    : task_queue_(FLAGS_task_queue_length), delay_callback_(nullptr),
      delay_callback_args_(nullptr) {
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
        if (delay_callback_) {
            delay_callback_(delay_callback_args_);
            delay_callback_ = nullptr;
            delay_callback_args_ = nullptr;
        }
    }
}

int SmartThread::yield(bool with_callback, DelayCallback runner_callback,
                       void *runner_callback_args) {
    if (with_callback) {
        delay_callback_ = runner_callback;
        delay_callback_args_ = runner_callback_args;
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
    DLOG(INFO) << tls_smart_thread << " destroy_coro " << coro;
    delete coro;
}

int SmartThread::switch_to(SmartCoro *next_coro) {
    // DLOG(INFO) << tls_smart_thread << " switch_to " << next_coro;
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
}