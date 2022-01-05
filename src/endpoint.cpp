#include "endpoint.h"
#include <glog/logging.h>
#include <strings.h>

namespace smartroutine {
Endpoint::Endpoint(Address address, unsigned short port)
    : address_(address), port_(port) {
    convert_to_sockaddr_in();
}

sockaddr *Endpoint::to_sockaddr_in(size_t &addr_size) const {
    if (address_.is_v4()) {
        addr_size = sizeof(sockaddr_in);
        return (sockaddr *)&addr_;
    } else if (address_.is_v6()) {
        addr_size = sizeof(sockaddr_in6);
        return (sockaddr *)&addr6_;
    } else {
        LOG(ERROR) << "unsupported address type";
        return nullptr;
    }
}

void Endpoint::from_sockaddr(const struct sockaddr *address) {
    if (address->sa_family == AF_INET) {
        struct sockaddr_in *addr = (struct sockaddr_in *)address;
        address_.set_ipv4_address(&addr->sin_addr);
        port_ = addr->sin_port;
    } else if (address->sa_family == AF_INET6) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)address;
        address_.set_ipv6_address(&addr6->sin6_addr);
        port_ = addr6->sin6_port;
    } else {
        LOG(ERROR) << "unsupported address type";
    }
}

Endpoint::Endpoint() : port_(-1) {}

Endpoint make_endpoint(const char *ip, unsigned short port, ErrorCode &ec) {
    Endpoint endpoint;
    endpoint.address_ = make_address(ip, ec);
    endpoint.port_ = port;
    endpoint.convert_to_sockaddr_in();
    return endpoint;
}

std::string Endpoint::to_string() const {
    std::string str = address_.to_string();
    str += ":";
    str += std::to_string(port_);
    return str;
}

void Endpoint::convert_to_sockaddr_in() {
    if (address_.is_v4()) {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port_);
        address_.copy_addr((void *)&addr_.sin_addr);
    } else if (address_.is_v6()) {
        bzero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = htons(port_);
        address_.copy_addr((void *)&addr6_.sin6_addr);
    } else {
        LOG(ERROR) << "unsupported address type";
    }
}
}