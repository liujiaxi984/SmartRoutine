#include "smart_runtime.h"
#include "smart_thread.h"
#include <gflags/gflags.h>
#include <pthread.h>
#include <thread>

extern __thread SmartThread *tls_smart_thread;

pthread_once_t SmartRuntime::once_control_ = PTHREAD_ONCE_INIT;

SmartRuntime *SmartRuntime::value_ = NULL;

DEFINE_uint64(runtime_threads_num, 0, "initial runtime threads number");

void *run_smart_thread(void *args) {
    SmartThread *thread = (SmartThread *)args;
    tls_smart_thread = thread;
    thread->main_loop();
}

SmartRuntime &SmartRuntime::get_instance() {
    pthread_once(&once_control_, init);
    return *value_;
}

void SmartRuntime::init() {
    if (FLAGS_runtime_threads_num == 0) {
        value_->threads_num_ = std::thread::hardware_concurrency();
    } else {
        value_->threads_num_ = FLAGS_runtime_threads_num;
    }
    value_->threads_ = new SmartThread[value_->threads_num_];
    for (uint i = 0; i < value_->threads_num_; i++) {
        pthread_t tid;
        pthread_create(&tid, nullptr, run_smart_thread, &value_->threads_[i]);
    }
}

int SmartRuntime::push_task(SmartCoro *coro) {
    std::lock_guard<std::mutex> lk(task_list_lock_);
    task_list_.push_back(coro);
    task_list_cond_.notify_one();
    return 0;
}

int SmartRuntime::get_tasks(uint batch_size, std::list<SmartCoro *> &tasks) {
    std::lock_guard<std::mutex> lk(task_list_lock_);
    auto iter = task_list_.begin();
    uint i = 0;
    while (iter != task_list_.end() && i < batch_size) {
        iter++;
        i++;
    }
    tasks.splice(tasks.end(), task_list_, task_list_.begin(), iter);

    return i;
}

int SmartRuntime::wait_on_task_list() {
    std::unique_lock<std::mutex> lk(task_list_lock_);
    task_list_cond_.wait(lk, [this] { return !task_list_.empty(); });
    lk.unlock();

    return 0;
}