#pragma once
#include "endpoint.h"
#include "error_code.h"
namespace smartroutine {
class TcpSocket {
  public:
    TcpSocket();
    void open(ErrorCode &ec);
    void connect(const Endpoint &peer_endpoint, ErrorCode &ec);
    void bind(const Endpoint &endpoint, ErrorCode &ec);
    void close(ErrorCode &ec);
    bool is_open() const { return opened_; }

  private:
    int socket_;
    bool opened_;
    bool bound_;
    bool connected_;
};
}