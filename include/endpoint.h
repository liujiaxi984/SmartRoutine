#pragma once
#include "address.h"
namespace smartroutine {

class Endpoint {
  public:
    Endpoint(Address address, unsigned short port);
    unsigned short port() const { return port_; }
    sockaddr *to_sockaddr_in(size_t &addr_size) const;
    void from_sockaddr(const struct sockaddr *address);

  private:
    union {
        sockaddr_in addr_;
        sockaddr_in6 addr6_;
    };
    Address address_;
    unsigned short port_;
};
}