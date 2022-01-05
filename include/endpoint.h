#pragma once
#include "address.h"
namespace smartroutine {

class Endpoint {
  public:
    friend Endpoint make_endpoint(const char *ip, unsigned short port,
                                  ErrorCode &code);
    Endpoint();
    Endpoint(Address address, unsigned short port);
    unsigned short port() const { return port_; }
    sockaddr *to_sockaddr_in(size_t &addr_size) const;
    void from_sockaddr(const struct sockaddr *address);
    std::string to_string() const;

  private:
    void convert_to_sockaddr_in();

  private:
    union {
        sockaddr_in addr_;
        sockaddr_in6 addr6_;
    };
    Address address_;
    unsigned short port_;
};

Endpoint make_endpoint(const char *ip, unsigned short port, ErrorCode &ec);
}