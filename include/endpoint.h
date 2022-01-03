#pragma once
#include "address.h"
namespace smartroutine {

class Endpoint {
  public:
    Endpoint(Address address, unsigned short port)
        : address_(address), port_(port) {
        if (address_.is_v4()) {
            bzero(&addr_, sizeof(addr_));
            addr_.sin_family = AF_INET;
            addr_.sin_port = htons(port_);
            address_.copy_addr((void *)&addr_.sin_addr);
        } else {
            bzero(&addr6_, sizeof(addr6_));
            addr6_.sin6_family = AF_INET6;
            addr6_.sin6_port = htons(port_);
            address_.copy_addr((void *)&addr6_.sin6_addr);
        }
    }
    unsigned short port() const { return port_; }
    sockaddr *to_sockaddr_in(size_t &addr_size) const {
        if (address_.is_v4()) {
            addr_size = sizeof(sockaddr_in);
            return (sockaddr *)&addr_;
        } else {
            addr_size = sizeof(sockaddr_in6);
            return (sockaddr *)&addr6_;
        }
    }

  private:
    sockaddr_in addr_;
    sockaddr_in6 addr6_;
    Address address_;
    unsigned short port_;
};
}