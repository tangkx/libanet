#ifndef LIBANET_NET_POLLPOLLER_H
#define LIBANET_NET_POLLPOLLER_H

#include "libanet/net/Poller.h"

#include <vector>

struct pollfd;

namespace libanet {
namespace net {

class PollPoller : public Poller {
public:
	PollPoller(EventLoop *loop);
	~PollPoller() override;

	Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
	void updateChannel(Channel *channel) override;
	void removeChannel(Channel *channel) override;

private:
	void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

	typedef std::vector<struct pollfd> PollFdList;
	PollFdList pollfds_;
};

} // namespace net
} // namespace libanet

#endif