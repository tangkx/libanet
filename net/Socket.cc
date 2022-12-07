#include "libanet/net/Socket.h"
#include "libanet/base/Logging.h"
#include "libanet/net/InetAddress.h"
#include "libanet/net/SocketOps.h"
#include "netinet/in.h"
#include "netinet/tcp.h"
#include <stdio.h>

using namespace libanet;
using namespace libanet::net;

Socket::~Socket() {
	sockets::close(sockfd_);
}

bool
Socket::getTcpInfo(struct tcp_info *tcpi) const {
	socklen_t len = sizeof(*tcpi);
	memZero(tcpi, len);
	return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}