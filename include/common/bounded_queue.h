#pragma once
#include <cstdint>

template <typename T> class BoundedQueue {
  public:
    BoundedQueue(uint32_t capacity)
        : capacity_(capacity), head_(0), tail_(0), is_empty_(true) {
        data_ = new T[capacity_];
    }
    ~BoundedQueue() { delete data_; }
    bool push(const T &value) {
        if (full())
            return false;
        data_[tail_++] = value;
        tail_ %= capacity_;
        is_empty_ = false;
        return true;
    }
    bool empty() { return is_empty_; }
    bool full() { return tail_ == head_ && !is_empty_; }
    T front() { return data_[head_]; }
    bool pop() {
        head_++;
        head_ %= capacity_;
        if (tail_ == head_)
            is_empty_ = true;
        return true;
    }

  private:
    T *data_;
    uint32_t capacity_;
    uint32_t head_;
    uint32_t tail_;
    bool is_empty_;
};
