#ifndef LIBANET_NET_TIMERQUEUE_H
#define LIBANET_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "libanet/base/Mutex.h"
#include "libanet/base/Timestamp.h"
#include "libanet/base/noncopyable.h"
#include "libanet/net/Callbacks.h"
#include "libanet/net/Channel.h"

namespace libanet {
namespace net {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : libanet::noncopyable {
public:
	explicit TimerQueue(EventLoop *loop);
	~TimerQueue();

	TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

	void cancel(TimerId timerId);

private:
	typedef std::pair<Timestamp, Timer *> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer *, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;

	void addTimerInLoop(Timer *timer);
	void cancelInLoop(TimerId timerId);

	void handleRead();

	std::vector<Entry> getExpired(Timestamp now);
	void reset(const std::vector<Entry> &expired, Timestamp now);

	bool insert(Timer *timer);

	EventLoop *loop_;
	const int timerfd_;
	Channel timerfdChannel_;

	TimerList timers_;

	ActiveTimerSet activeTimers_;
	bool callingExpiredTimers_;
	ActiveTimerSet cancelingTimers_;
};
} // namespace net
} // namespace libanet

#endif
