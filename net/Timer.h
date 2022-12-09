#ifndef LIBANET_NET_TIMER_H
#define LIBANET_NET_TIMER_H

#include "libanet/base/Atomic.h"
#include "libanet/base/Timestamp.h"
#include "libanet/base/noncopyable.h"
#include "libanet/net/Callbacks.h"

namespace libanet {
namespace net {

class Timer : libanet::noncopyable {
public:
	Timer(TimerCallback cb, Timestamp when, double interval)
	: callback_(std::move(cb)), expiration_(when), interval_(interval),
	  repeat_(interval > 0.0), sequence_(s_numCreated_.incrementAndGet()) {}

	void run() const {
		callback_();
	}

	Timestamp expiration() const {
		return expiration_;
	}

	bool repeat() const {
		return repeat_;
	}

	int64_t sequence() const {
		return sequence_;
	}

	void restart(Timestamp now);

	static int64_t numCreated() {
		return s_numCreated_.get();
	}

private:
	const TimerCallback callback_;
	Timestamp expiration_;
	const double interval_;
	const bool repeat_;
	const int64_t sequence_;

	static AtomicInt64 s_numCreated_;
};

} // namespace net
} // namespace libanet

#endif