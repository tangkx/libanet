#ifndef LIBANET_NET_TIMERID_H
#define LIBANET_NET_TIMERID_H

#include "libanet/base/copyable.h"
#include <stddef.h>
#include <stdint.h>

namespace libanet {
namespace net {
class Timer;

class TimerId : public libanet::copyable {
public:
	TimerId() : timer_(NULL), sequence_(0) {}
	TimerId(Timer *timer, int64_t seq) : timer_(timer), sequence_(seq) {}

	friend class TimerQueue;

private:
	Timer *timer_;
	int64_t sequence_;
};

} // namespace net
} // namespace libanet

#endif