#pragma once
#include <sys/types.h>

namespace smartroutine {
class DynamicBuffer {
  public:
    size_t size();
    size_t max_size();
    size_t capacity();

  private:
};
}