#include "smart_coro.h"
#include "smart_thread.h"
#include <algorithm>
#include <cstring>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <unistd.h>
namespace smartroutine {

DEFINE_uint64(stack_size, 128 * 1024, "coroutine stack size");

SmartCoro::SmartCoro(EntryFn fn, void *args) : fn_(fn), args_(args) {
    const static unsigned int PAGESIZE = getpagesize();
    const unsigned int PAGESIZE_M1 = PAGESIZE - 1;
    const unsigned int MIN_STACKSIZE = PAGESIZE * 2;

    // Align stacksize
    const int stack_size =
        (std::max((unsigned int)FLAGS_stack_size, MIN_STACKSIZE) +
         PAGESIZE_M1) &
        ~PAGESIZE_M1;

    context_.sp_ = malloc(stack_size);
    if (context_.sp_ == nullptr) {
        LOG(ERROR) << "malloc error: " << strerror(errno);
    }
    context_.stack_size_ = stack_size;
    void *bp = (char *)context_.sp_ + stack_size;
    context_.fcontext_ =
        make_fcontext(bp, stack_size, SmartThread::coro_runner);
}

SmartCoro::~SmartCoro() { free(context_.sp_); }
}