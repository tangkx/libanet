#ifndef LIBANET_NET_SOCKET_H
#define LIBANET_NET_SOCKET_H

#include "libanet/base/noncopyable.h"

struct tcp_info;

namespace libanet {
namespace net {

class InetAddress;

class Socket : noncopyable {
public:
	explicit Socket(int sockfd) : sockfd_(sockfd) {}

	~Socket();

	int fd() const {
		return sockfd_;
	}

	bool getTcpInfo(struct tcp_info *) const;
	bool getTcpInfoString(char *buf, int len) const;

	void bindAddress(const InetAddress &localaddr);

	void listen();

	int accept(InetAddress *peeraddr);

	void shutdownWrite();

	void setReuseAddr(bool on);

	void serReusePort(bool on);

	void setKeepAlive(bool on);

private:
	const int sockfd_;
};
} // namespace net
} // namespace libanet

#endif