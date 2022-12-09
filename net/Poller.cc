#include "libanet/net/Poller.h"
#include "libanet/net/Channel.h"

using namespace libanet;
using namespace libanet::net;

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {}

Poller::~Poller() = default;

bool
Poller::hasChannel(Channel *channel) const {
	assertInLoopThread();
	ChannelMap::const_iterator it = channels_.find(channel->fd());
	return it != channels_.end() && it->second == channel;
}