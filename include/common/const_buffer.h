#pragma once
#include <sys/types.h>

namespace smartroutine {

class ConstBuffer {
  public:
    ConstBuffer() : data_(nullptr), size_(0) {}
    ConstBuffer(void *data, size_t size) : data_(data), size_(size) {}
    const void *data() const { return data_; }
    size_t size() const { return size_; }

  private:
    const void *data_;
    size_t size_;
};
}