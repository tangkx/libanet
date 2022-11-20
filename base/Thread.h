#ifndef LIBANET_BASE_THREAD_H
#define LIBANET_BASE_THREAD_H

#include "libanet/base/Atomic.h"
#include "libanet/base/CountDownLatch.h"
#include "libanet/base/Types.h"

#include <functional>
#include <memory>
#include <pthread.h>

namespace libanet {

class Thread : noncopyable {
public:
	typedef std::function<void()> ThreadFunc;

	explicit Thread(ThreadFunc, const string &name = string());

	~Thread();

	void start();
	int join();

	bool started() const {
		return started_;
	}

	pid_t tid() const {
		return tid_;
	}

	const string &name() const {
		return name_;
	}

	static int numCreated() {
		return numCreated_.get();
	}

private:
	void setDefaultName();

	bool started_;
	bool joined_;
	pthread_t pthreadId_;
	pid_t tid_;
	ThreadFunc func_;
	string name_;
	CountDownLatch latch_;

	static AtomicInt32 numCreated_;
};

} // namespace libanet

#endif