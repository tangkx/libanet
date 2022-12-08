#ifndef LIBANET_NET_POLLER_H
#define LIBANET_NET_POLLER_H

#include <map>
#include <vector>

#include "libanet/base/Timestamp.h"
#include "libanet/base/noncopyable.h"
#include "libanet/net/EventLoop.h"

namespace libanet {
namespace net {
class Channel;

class Poller : noncopyable {
public:
	typedef std::vector<Channel *> ChannelList;

	Poller(EventLoop *loop);
	virtual ~Poller();

	virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

	virtual void updateChannel(Channel *channel) = 0;

	virtual void removeChannel(Channel *channel) = 0;

	virtual bool hasChannel(Channel *channel) const;

	static Poller *newDefaultPoller(EventLoop *loop);

	void assertInLoopThread() const {
		owerLoop_->assertInLoopThread();
	}

protected:
	typedef std::map<int, Channel *> ChannelMap;
	ChannelMap channel_s;

private:
	EventLoop *owerLoop_;
};

} // namespace net
} // namespace libanet

#endif