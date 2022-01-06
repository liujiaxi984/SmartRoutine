#include "common/dynamic_buffer.h"
#include <cstring>

namespace smartroutine {

DEFINE_uint64(max_dynamic_buffer_size, 1024 * 1024 * 64,
              "default max dynamic buffer capacity");

DEFINE_uint64(initial_dynamic_buffer_size, 1024,
              "default initial dynamic buffer capacity");

DEFINE_uint64(move_forward_threshold, 1024 * 1024,
              "threshold to reuse prependable bytes");

DynamicBuffer::DynamicBuffer()
    : read_index_(0), write_index_(0),
      max_capacity_(FLAGS_max_dynamic_buffer_size) {
    buffer_ = malloc(FLAGS_initial_dynamic_buffer_size);
    capacity_ = FLAGS_initial_dynamic_buffer_size;
}

void DynamicBuffer::append(const void *data, size_t data_length,
                           ErrorCode &ec) {
    if (data_length > writable_bytes()) {
        size_t max_writable_length = max_writable_bytes();
        if (data_length > max_writable_length) {
            ec = ExceedBufferBound;
            return;
        }
        move_forward();
        max_writable_length = max_writable_bytes();
        if (data_length > max_writable_length)
            expand_capacity();
    }

    std::memcpy(write_section_start(), data, data_length);
    write_index_ += data_length;
}

void DynamicBuffer::consume(void *data, size_t data_length, ErrorCode &ec) {
    if (data_length > readable_bytes()) {
        ec = ExceedBufferBound;
        return;
    }
    std::memcpy(data, read_section_start(), data_length);
    read_index_ -= data_length;
}

void DynamicBuffer::expand_capacity() {
    size_t new_capacity =
        capacity_ * 2 > max_capacity_ ? max_capacity_ : capacity_ * 2;
    void *new_buffer = malloc(new_capacity);
    std::memcpy(new_buffer, buffer_, capacity_);
    free(buffer_);
    buffer_ = new_buffer;
    capacity_ = new_capacity;
}

void DynamicBuffer::move_forward() {
    size_t prependable_length = prependable_bytes();
    std::memmove(buffer_, read_section_start(), capacity_ - prependable_length);
    write_index_ -= prependable_length;
    read_index_ = 0;
    capacity_ += prependable_length;
}

DynamicBuffer::~DynamicBuffer() {
    if (buffer_)
        free(buffer_);
}
void *DynamicBuffer::write_section_start() {
    char *p = (char *)buffer_;
    return p + write_index_;
}

void *DynamicBuffer::read_section_start() {
    char *p = (char *)buffer_;
    return p + read_index_;
}

size_t DynamicBuffer::max_writable_bytes() const {
    size_t prependable_length =
        prependable_bytes() >= FLAGS_move_forward_threshold
            ? prependable_bytes()
            : 0;
    return max_capacity_ - capacity_ + writable_bytes() + prependable_length;
}

ConstBuffer DynamicBuffer::read_buffer() {
    return ConstBuffer(read_section_start(), readable_bytes());
}
}