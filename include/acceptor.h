#pragma once
#include "tcp_socket.h"
namespace smartroutine {

class Acceptor : public TcpSocket {
  public:
    void listen(int backlog, ErrorCode &ec);
    void accept(TcpSocket &peer, Endpoint &peer_endpoint, ErrorCode &ec);

  private:
    bool listening_;
};
}