#include "address.h"
#include <arpa/inet.h>
#include <cstring>

namespace smartroutine {

Address make_address(const char *address_str, ErrorCode &code) {
    Address address;
    if (strchr(address_str, '.') != nullptr) {
        address.type_ = Ipv4;
        int ret = inet_pton(AF_INET, address_str, &address.addr_);
        if (ret == 0) {
            code = InvalidArgument;
        }
    } else if (strchr(address_str, ':') != nullptr) {
        address.type_ = Ipv6;
        int ret = inet_pton(AF_INET6, address_str, &address.addr6_);
        if (ret == 0) {
            code = InvalidArgument;
        }
    } else {
        code = InvalidArgument;
    }

    return address;
}

std::string Address::to_string() const {
    char buf[INET6_ADDRSTRLEN];
    if (type_ == Ipv4) {
        const char *ret = inet_ntop(AF_INET, &addr_, buf, INET6_ADDRSTRLEN);
    } else if (type_ == Ipv6) {
        const char *ret = inet_ntop(AF_INET6, &addr6_, buf, INET6_ADDRSTRLEN);
    }
    std::string str(buf);
    return str;
}

bool Address::is_v4() const { return type_ == Ipv4; }
bool Address::is_v6() const { return type_ == Ipv6; }

void Address::copy_addr(void *dst) const {
    if (type_ == Ipv4) {
        memcpy(dst, &addr_, sizeof(sockaddr_in));
    } else if (type_ == Ipv6) {
        memcpy(dst, &addr6_, sizeof(sockaddr_in6));
    }
}
}