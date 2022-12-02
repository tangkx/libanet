#ifndef LIBANET_BASE_THREADPOOL_H
#define LIBANET_BASE_THREADPOOL_H

#include "libanet/base/Condition.h"
#include "libanet/base/Mutex.h"
#include "libanet/base/Thread.h"
#include "libanet/base/Types.h"

#include <deque>
#include <vector>

namespace libanet {

class ThreadPool : noncopyable {

public:
	typedef std::function<void()> Task;

	explicit ThreadPool(const string &nameArg = string("ThreadPool"));
	~ThreadPool();

	void setMaxQueueSize(int maxSize) {
		maxQueueSize_ = maxSize;
	}

	void setThreadInitCallback(const Task &cb) {
		threadInitCallback_ = cb;
	}

	void start(int numThreads);
	void stop();

	const string &name() const {
		return name_;
	}

	size_t queueSize() const;

	void run(Task f);

private:
	bool isFull() const;
	void runInThread();
	Task take();

	mutable MutexLock mutex_;
	Condition notEmpty_;
	Condition notFull_;
	size_t maxQueueSize_;
	Task threadInitCallback_;
	string name_;
	bool running_;

	std::vector<std::unique_ptr<libanet::Thread>> threads_;
	std::deque<Task> queue_;
};

} // namespace libanet

#endif