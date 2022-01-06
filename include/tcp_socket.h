#pragma once
#include "common/buffer.h"
#include "common/dynamic_buffer.h"
#include "common/error_code.h"
#include "endpoint.h"

namespace smartroutine {
class TcpSocket {
  public:
    TcpSocket();
    TcpSocket(TcpSocket &&other);
    virtual ~TcpSocket();
    void open(ErrorCode &ec);
    void connect(const Endpoint &peer_endpoint, ErrorCode &ec);
    void bind(const Endpoint &endpoint, ErrorCode &ec);
    void close(ErrorCode &ec);
    bool is_open() const { return opened_; }
    bool is_bound() const { return bound_; }
    bool is_connected_() const { return connected_; }
    void set_accepted_fd(int fd);
    ssize_t read(const MutableBuffer &buffer, ErrorCode &ec);
    ssize_t read_until(DynamicBuffer &buffer, std::string delim, ErrorCode &ec);
    ssize_t write(const ConstBuffer &buffer, ErrorCode &ec);

  protected:
    int socket_;
    bool opened_;
    bool bound_;
    bool connected_;
};
}