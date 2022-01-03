#include "smart_runtime.h"
#include "smart_thread.h"
#include <gflags/gflags.h>
#include <pthread.h>
#include <thread>

extern __thread smartroutine::SmartThread *tls_smart_thread;

namespace smartroutine {

pthread_once_t SmartRuntime::once_control_ = PTHREAD_ONCE_INIT;

SmartRuntime *SmartRuntime::value_ = NULL;

DEFINE_uint64(runtime_threads_num, 0, "initial runtime threads number");

void *run_smart_thread(void *args) {
    SmartThread *thread = (SmartThread *)args;
    tls_smart_thread = thread;
    thread->init();

    return 0;
}

void *run_epoller(void *args) {
    SmartEPoller *epoller = (SmartEPoller *)args;
    epoller->init();
    epoller->eventloop();

    return 0;
}

void *run_timer(void *args) {
    SmartTimer *timer = (SmartTimer *)args;
    timer->run();

    return 0;
}

SmartRuntime &SmartRuntime::get_instance() {
    pthread_once(&once_control_, init);
    return *value_;
}

void SmartRuntime::init() {
    value_ = new SmartRuntime;
    atexit(exit_clean);
    if (FLAGS_runtime_threads_num == 0) {
        value_->threads_num_ = std::thread::hardware_concurrency();
    } else {
        value_->threads_num_ = FLAGS_runtime_threads_num;
    }
    value_->threads_ = new SmartThread[value_->threads_num_];
    value_->tids_.resize(value_->threads_num_);
    for (unsigned int i = 0; i < value_->threads_num_; i++) {
        pthread_create(&value_->tids_[i], nullptr, run_smart_thread,
                       &value_->threads_[i]);
    }
    pthread_create(&value_->epoller_tid_, nullptr, run_epoller,
                   &value_->epoller_);
    pthread_create(&value_->timer_tid_, nullptr, run_timer, &value_->timer_);
}

int SmartRuntime::push_task(SmartCoro *coro) {
    std::lock_guard<std::mutex> lk(task_list_lock_);
    task_list_.push_back(coro);
    task_list_cond_.notify_one();
    return 0;
}

int SmartRuntime::get_tasks(unsigned int batch_size,
                            std::list<SmartCoro *> &tasks) {
    std::lock_guard<std::mutex> lk(task_list_lock_);
    auto iter = task_list_.begin();
    unsigned int i = 0;
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

SmartEPoller &SmartRuntime::get_epoller() { return epoller_; }

SmartTimer &SmartRuntime::get_timer() { return timer_; }

SmartRuntime::SmartRuntime() : threads_num_(0), threads_(nullptr) {}
SmartRuntime::~SmartRuntime() {
    for (unsigned int i = 0; i < value_->threads_num_; i++) {
        pthread_join(value_->tids_[i], nullptr);
    }
    pthread_join(value_->timer_tid_, nullptr);
    pthread_join(value_->timer_tid_, nullptr);
    delete[] threads_;
    for (auto iter = task_list_.begin(); iter != task_list_.end(); iter++) {
        delete *iter;
    }
}

void SmartRuntime::exit_clean() { delete value_; }
}