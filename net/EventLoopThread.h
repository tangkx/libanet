#ifndef LIBANET_NET_EVENTLOOPTHREAD_H
#define LIBANET_NET_EVENTLOOPTHREAD_H

#include "libanet/base/Condition.h"
#include "libanet/base/Mutex.h"
#include "libanet/base/Thread.h"
#include "libanet/base/noncopyable.h"

namespace libanet {
namespace net {

class EventLoop;

class EventLoopThread : libanet::noncopyable {
public:
	typedef std::function<void(EventLoop *)> ThreadInitCallback;

	EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
					const string &name = string());

	~EventLoopThread();

	EventLoop *startLoop();

private:
	void threadFunc();

	EventLoop *loop_;
	bool exiting_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
	ThreadInitCallback callback_;
};

} // namespace net
} // namespace libanet

#endif