#pragma once
#include "mutable_buffer.h"
#include <sys/types.h>
namespace smartroutine {

class ConstBuffer {
  public:
    ConstBuffer() : data_(nullptr), size_(0) {}
    ConstBuffer(MutableBuffer &mutable_buffer)
        : ConstBuffer(mutable_buffer.data(), mutable_buffer.size()) {}
    ConstBuffer(const void *data, size_t size) : data_(data), size_(size) {}
    const void *data() const { return data_; }
    size_t size() const { return size_; }

  private:
    const void *data_;
    size_t size_;
};
}