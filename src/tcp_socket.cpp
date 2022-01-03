#include "tcp_socket.h"
#include "smart_routine.h"
#include <glog/logging.h>
#include <strings.h>
#include <sys/socket.h>

namespace smartroutine {
TcpSocket::TcpSocket()
    : socket_(-1), opened_(false), bound_(false), connected_(false) {}

void TcpSocket::open(ErrorCode &ec) {
    if (opened_) {
        ec = InvalidArgument; // socket already opened
        return;
    }
    socket_ = smart_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ < 0) {
        if (errno == EMFILE || errno == ENFILE)
            ec = InsufficientResources;
        else if (errno == ENOBUFS || errno == ENOMEM)
            ec = OutOfMemory;
        else {
            LOG(ERROR) << "smart_socket error: " << strerror(errno);
        }
    } else {
        opened_ = true;
    }
}

void TcpSocket::bind(const Endpoint &endpoint, ErrorCode &ec) {
    if (!opened_ || bound_) {
        ec = InvalidArgument;
        return;
    }
    size_t addr_size;
    const sockaddr *addr = endpoint.to_sockaddr_in(addr_size);
    int ret = ::bind(socket_, addr, addr_size);
    if (ret < 0) {
        if (errno == EACCES)
            ec = PermissionDenied;
        else if (errno == EADDRINUSE)
            ec = AddressInUse; // address is in use or port == 0 but all
                               // ephemeral ports are in use
        else {
            LOG(ERROR) << "smart_socket error: " << strerror(errno);
        }
    } else {
        bound_ = true;
    }
}

void TcpSocket::connect(const Endpoint &peer_endpoint, ErrorCode &ec) {
    if (!opened_ || connected_) {
        ec = InvalidArgument;
        return;
    }
    size_t addr_size;
    const sockaddr *addr = peer_endpoint.to_sockaddr_in(addr_size);
    int ret = smart_connect(socket_, addr, addr_size);
    if (ret < 0) {
        if (errno == EACCES || errno == EPERM)
            ec = PermissionDenied;
        else if (errno == EADDRINUSE)
            ec = AddressInUse;
        else {
            LOG(ERROR) << "smart_socket error: " << strerror(errno);
        }
    } else {
        connected_ = true;
    }
}

void TcpSocket::close(ErrorCode &ec) {
    if (!opened_) {
        ec = InvalidArgument; // socket already opened
        return;
    }
    if (socket_ >= 0)
        ::close(socket_);
}
}