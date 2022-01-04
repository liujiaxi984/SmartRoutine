#pragma once
#include <sys/types.h>

namespace smartroutine {
class MutableBuffer {
  public:
    MutableBuffer() : data_(nullptr), size_(0) {}
    MutableBuffer(void *data, size_t size) : data_(data), size_(size) {}

    void *data() const { return data_; }
    size_t size() const { return size_; }

  private:
    void *data_;
    size_t size_;
};
}