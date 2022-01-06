#pragma once
#include "const_buffer.h"
#include "error_code.h"
#include <gflags/gflags.h>
#include <sys/types.h>
namespace smartroutine {
class DynamicBuffer {
  public:
    DynamicBuffer();
    ~DynamicBuffer();
    size_t max_capacity() const { return max_capacity_; }
    size_t capacity() const { return capacity_; }

    size_t readable_bytes() const { return write_index_ - read_index_; }
    size_t writable_bytes() const { return capacity_ - write_index_; }
    size_t prependable_bytes() const { return read_index_; }

    void append(const void *data, size_t data_length, ErrorCode &ec);
    void consume(void *data, size_t data_length, ErrorCode &ec);
    /**
     * @brief convert read section into a ConstBuffer
     * @warning calling any other non-const member function will invalidate this
     * buffer
     * @return ConstBuffer
     */
    ConstBuffer read_buffer();

  private:
    size_t max_writable_bytes() const;
    void *write_section_start();
    void *read_section_start();
    void expand_capacity();
    void move_forward();

  private:
    void *buffer_;
    size_t read_index_;
    size_t write_index_;
    size_t capacity_;
    const size_t max_capacity_;
};
}