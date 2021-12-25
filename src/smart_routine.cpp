#include "smart_routine.h"
#include "smart_runtime.h"
#include "smart_thread.h"

extern __thread SmartThread *tls_smart_thread;

int smart_routine_start(void *(*fn)(void *), void *args) {
    SmartCoro *coro = new SmartCoro(fn, args);
    if (tls_smart_thread != nullptr)
        if (tls_smart_thread->push_task(coro) == 0)
            return 0;
    SmartRuntime::get_instance().push_task(coro);
    return 0;
}

int smart_routine_yeild() {}