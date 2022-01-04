#pragma once
#include "common/error_code.h"
#include <netinet/in.h>
#include <string>

namespace smartroutine {
enum AddressType { None, Ipv4, Ipv6 };

class Address {
  public:
    Address();
    friend Address make_address(const char *address_str, ErrorCode &code);
    void set_ipv4_address(const struct in_addr *addr);
    void set_ipv6_address(const struct in6_addr *addr6);

    std::string to_string() const;
    void copy_addr(void *dst) const;
    bool is_v4() const;
    bool is_v6() const;

  private:
    union {
        struct in_addr addr_;
        struct in6_addr addr6_;
    };
    AddressType type_;
};

Address make_address(const char *address_str, ErrorCode &code);
}
