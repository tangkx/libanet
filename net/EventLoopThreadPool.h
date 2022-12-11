#ifndef LIBANET_NET_EVENTLOOPTHREADPOOL_H
#define LIBANET_NET_EVENTLOOPTHREADPOOL_H

#include "libanet/base/Types.h"
#include "libanet/base/noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

namespace libanet {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : libanet::noncopyable {
public:
	typedef std::function<void(EventLoop *)> ThreadInitCallback;

	EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg);
	~EventLoopThreadPool();
	void setThreadNum(int numThreads) {}
	void start(const ThreadInitCallback &cb = ThreadInitCallback());

	EventLoop *getNextLoop();
	EventLoop *getLoopForHash(size_t hashCode);

	std::vector<EventLoop *> getAllLoops();

	bool started() const {
		return started_;
	}

	const string &name() const {
		return name_;
	}

private:
	EventLoop *baseLoop_;
	string name_;
	bool started_;
	int numThread_;
	int next_;
	std::vector<std::unique_ptr<EventLoopThread>> threads_;
	std::vector<EventLoop *> loops_;
};

} // namespace net
} // namespace libanet

#endif