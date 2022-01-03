#pragma once
#include "error_code.h"
#include <netinet/in.h>
#include <string>

namespace smartroutine {
enum AddressType { Ipv4, Ipv6 };

class Address {
  public:
    friend Address make_address(const char *address_str, ErrorCode &code);

    std::string to_string() const;
    void copy_addr(void *dst) const;
    bool is_v4() const;
    bool is_v6() const;

  private:
    struct in_addr addr_;
    struct in6_addr addr6_;
    AddressType type_;
};

Address make_address(const char *address_str, ErrorCode &code);
}
