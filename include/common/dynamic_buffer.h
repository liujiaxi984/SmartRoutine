#pragma once
#include <sys/types.h>

#define MAX_BUFFER_SIZE 1024 * 1024 * 8

namespace smartroutine {
class DynamicBuffer {
  public:
    size_t max_capacity() { return MAX_BUFFER_SIZE; }
    size_t capacity() { return capacity_; }

  private:
    size_t read_index_;
    size_t write_index_;
    size_t capacity_;
};
}