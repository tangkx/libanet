#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "libanet/net/TimerQueue.h"
#include "libanet/base/Logging.h"
#include "libanet/net/EventLoop.h"
#include "libanet/net/Timer.h"
#include "libanet/net/TimerId.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace libanet {
namespace net {
namespace detail {

int
createTimerfd() {
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0) {
		LOG_SYSFATAL << "Failed in timerfd_create";
	}
	return timerfd;
}

struct timespec
howMuchTimeFromNow(Timestamp when) {
	int64_t microseconds = when.microSecondsSinceEpoch() -
						   Timestamp::now().microSecondsSinceEpoch();
	if (microseconds < 100) {
		microseconds = 100;
	}

	struct timespec ts;
	ts.tv_sec =
		static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>(
		(microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);

	return ts;
}

void
readTimerfd(int timerfd, Timestamp now) {
	uint64_t howmany;
	ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
	LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at "
			  << now.toString();
	if (n != sizeof(howmany)) {
		LOG_ERROR << "TimerQueue::handleRead() reads " << n
				  << " bytes instead of 8";
	}
}

void
resetTimerfd(int timerfd, Timestamp expiration) {
	struct itimerspec newValue;
	struct itimerspec oldValue;
	memZero(&newValue, sizeof(newValue));
	memZero(&oldValue, sizeof(oldValue));
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
	if (ret) {
		LOG_SYSERR << "timerfd_settime()";
	}
}

} // namespace detail
} // namespace net
} // namespace libanet

using namespace libanet;
using namespace libanet::net;
using namespace libanet::net::detail;

TimerQueue::TimerQueue(EventLoop *loop)
: loop_(loop), timerfd_(createTimerfd()), timerfdChannel_(loop, timerfd_),
  timers_(), callingExpiredTimers_(false) {

	timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
	timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();
	::close(timerfd_);
	for (const Entry &timer : timers_) {
		delete timer.second;
	}
}