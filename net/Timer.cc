#include "libanet/net/Timer.h"

using namespace libanet;
using namespace libanet::net;

AtomicInt64 Timer::s_numCreated_;

void
Timer::restart(Timestamp now) {
	if (repeat_) {
		expiration_ = addTime(now, interval_);
	} else {
		expiration_ = Timestamp::invalid();
	}
}