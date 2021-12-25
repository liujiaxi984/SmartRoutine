#pragma once
#include "internal/libcontext.h"

typedef void *(*smart_pfn_t)(void *);

struct CoroContext {
    void *sp_;
    uint stack_size_;
    fcontext_t context_;
};

class SmartCoro {
    friend class SmartThread;

  public:
    SmartCoro(smart_pfn_t fn, void *args);

  private:
    smart_pfn_t fn_;
    void *args_;
    CoroContext context_;
};