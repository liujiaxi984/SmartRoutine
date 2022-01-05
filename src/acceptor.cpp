#include "acceptor.h"
#include "smart_routine.h"
#include <glog/logging.h>
namespace smartroutine {

Acceptor::Acceptor() : listening_(false) {}

void Acceptor::listen(int backlog, ErrorCode &ec) {
    if (!opened_ || listening_) {
        ec = InvalidArgument;
        return;
    }
    int ret = ::listen(socket_, backlog);
    if (ret < 0) {
        if (errno == EADDRINUSE)
            ec = AddressInUse;
        else {
            LOG(ERROR) << "listen error: " << strerror(errno);
        }
    } else {
        listening_ = true;
    }
}

void Acceptor::accept(TcpSocket &peer, Endpoint &peer_endpoint, ErrorCode &ec) {
    if (!opened_ || !listening_) {
        ec = InvalidArgument;
        return;
    }
    struct sockaddr_in6 addr;
    bzero(&addr, sizeof(addr));
    socklen_t len = sizeof(addr);

    int fd = smart_accept(socket_, (struct sockaddr *)&addr, &len);
    if (fd < 0) {
        if (errno == ECONNABORTED)
            ec = ConnectionAborted;
        else if (errno == EMFILE || errno == ENFILE)
            ec = InsufficientResources;
        else if (errno == EINVAL)
            ec = InvalidArgument;
        else if (errno == ENOBUFS || errno == ENOMEM)
            ec = OutOfMemory;
        else if (errno == EPERM)
            ec = PermissionDenied; // Firewall rules forbid connection
        else {
            LOG(ERROR) << "smart_accept error: " << strerror(errno);
        }
    } else {
        peer.set_accepted_fd(fd);
        peer_endpoint.from_sockaddr((struct sockaddr *)&addr);
    }
}

void Acceptor::set_address_reuse(ErrorCode &ec) {
    if (!opened_) {
        ec = InvalidArgument;
        return;
    }
    int reuse_addr = 1;
    int ret = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
                         sizeof(reuse_addr));
    if (ret < 0)
        LOG(ERROR) << "setsockopt error: " << strerror(errno);
}
}