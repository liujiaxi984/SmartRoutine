#include "smart_coro.h"
#include "smart_thread.h"
#include <algorithm>
#include <gflags/gflags.h>
#include <unistd.h>

DEFINE_uint64(stack_size, 128 * 1024, "coroutine stack size");

SmartCoro::SmartCoro(smart_pfn_t fn, void *args) : fn_(fn), args_(args) {
    const static uint PAGESIZE = getpagesize();
    const uint PAGESIZE_M1 = PAGESIZE - 1;
    const uint MIN_STACKSIZE = PAGESIZE * 2;

    // Align stacksize
    const int stack_size =
        (std::max((uint)FLAGS_stack_size, MIN_STACKSIZE) + PAGESIZE_M1) &
        ~PAGESIZE_M1;

    context_.sp_ = malloc(stack_size);
    context_.stack_size_ = stack_size;
    void *bp = context_.sp_ + stack_size;
    context_.context_ = make_fcontext(bp, stack_size, SmartThread::coro_runner);
}