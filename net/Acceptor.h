#ifndef LIBANET_NET_ACCEPTOR_H
#define LIBANET_NET_ACCEPTOR_H

#include "libanet/base/noncopyable.h"
#include "libanet/net/Channel.h"
#include "libanet/net/Socket.h"
#include <functional>

namespace libanet {
namespace net {

class EventLoop;
class InetAddress;

class Acceptor : libanet::noncopyable {
public:
	typedef std::function<void(int sockfd, const InetAddress &)>
		NewConnectionCallback;

	Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
	~Acceptor();

	void setNewConnectionCallback(const NewConnectionCallback &cb) {
		newConnectionCallback_ = cb;
	}

	void listen();

	bool listening() const {
		return listening_;
	}

private:
	void handleRead();

	EventLoop *loop_;
	Socket acceptSocket_;
	Channel acceptChannel_;
	NewConnectionCallback newConnectionCallback_;
	bool listening_;
	int idleFd_;
};

} // namespace net
} // namespace libanet

#endif