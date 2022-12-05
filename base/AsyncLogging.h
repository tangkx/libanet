#ifndef LIBANET_BASE_ASYNCLOGGING_H
#define LIBANET_BASE_ASYNCLOGGING_H

#include "libanet/base/BlockingQueue.h"
#include "libanet/base/BoundedBlockingQueue.h"
#include "libanet/base/CountDownLatch.h"
#include "libanet/base/LogStream.h"
#include "libanet/base/Mutex.h"
#include "libanet/base/Thread.h"

#include <atomic>
#include <vector>

namespace libanet {

class AsyncLogging : noncopyable {

public:
	AsyncLogging(const string &basename, off_t rollSize, int flushInterval = 3);

	~AsyncLogging() {
		if (running_) {
			stop();
		}
	}

	void append(const char *logline, int len);

	void start() {
		running_ = true;
		thread_.start();
		latch_.wait();
	}

	void stop() {
		running_ = false;
		cond_.notify();
		thread_.join();
	}

private:
	void threadFunc();
	typedef libanet::detail::FixedBuffer<libanet::detail::kLargeBuffer> Buffer;
	typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
	typedef BufferVector::value_type BufferPtr;

	const int flushInterval_;
	std::atomic<bool> running_;
	const string basename_;
	const off_t rollSize_;
	libanet::Thread thread_;
	libanet::CountDownLatch latch_;
	libanet::MutexLock mutex_;
	libanet::Condition cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
};
} // namespace libanet

#endif