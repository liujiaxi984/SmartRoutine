#pragma once
#include "tcp_socket.h"
namespace smartroutine {

class Acceptor : public TcpSocket {
  public:
    Acceptor();
    void listen(int backlog, ErrorCode &ec);
    void accept(TcpSocket &peer, Endpoint &peer_endpoint, ErrorCode &ec);
    void set_address_reuse(ErrorCode &ec);

  private:
    bool listening_;
};
}