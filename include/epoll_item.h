#pragma once
#include <functional>
#include <sys/epoll.h>
namespace smartroutine {

class SmartEPoller;

typedef std::function<void()> EventCallback;

class EPollItem {
  public:
    EPollItem(int fd, SmartEPoller *epoller)
        : fd_(fd), epoller_(epoller), watching_(false) {}
    int fd() { return fd_; }
    int events() { return events_; }
    void enable_reading() {
        events_ |= EPOLLIN;
        update();
    }
    void disable_reading() {
        events_ &= ~EPOLLIN;
        update();
    }
    void enable_writing() {
        events_ |= EPOLLOUT;
        update();
    }
    void disable_writing() {
        events_ &= ~EPOLLOUT;
        update();
    }
    void disable_all() {
        events_ = 0;
        update();
    }
    bool is_writing() const { return events_ & EPOLLOUT; }
    bool is_reading() const { return events_ & EPOLLIN; }
    bool is_disabled() const { return events_ == 0; }
    void set_read_callback(EventCallback read_callback) {
        read_callback_ = read_callback;
    }
    void set_write_callback(EventCallback write_callback) {
        write_callback_ = write_callback;
    }
    void set_close_callback(EventCallback close_callback) {
        close_callback_ = close_callback;
    }
    void set_error_callback(EventCallback error_callback) {
        error_callback_ = error_callback;
    }
    void set_revents(int revents) { revents_ = revents; }
    bool is_watching() { return watching_; }
    void set_watching() { watching_ = true; }
    void clear_watching() { watching_ = false; }
    void handle_events();

  private:
    void update();

  private:
    int fd_;
    int events_;
    int revents_;
    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
    SmartEPoller *epoller_;
    bool watching_;
};
}