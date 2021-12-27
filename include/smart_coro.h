#pragma once
#include "internal/libcontext.h"

typedef void *(*EntryFn)(void *);

struct CoroContext {
    void *sp_;
    unsigned int stack_size_;
    fcontext_t fcontext_;
};

class SmartCoro {
    friend class SmartThread;

  public:
    SmartCoro(EntryFn fn, void *args);
    ~SmartCoro();

  private:
    EntryFn fn_;
    void *args_;
    CoroContext context_;
};