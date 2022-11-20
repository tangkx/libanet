#ifndef LIBANET_BASE_COUNTDOWNLATCH_H
#define LIBANET_BASE_COUNTDOWNLATCH_H

#include "libanet/base/Condition.h"
#include "libanet/base/Mutex.h"

namespace libanet {

class CountDownLatch : noncopyable {
public:
	explicit CountDownLatch(int count);

	void wait();

	void countDown();

	int getCount() const;

private:
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};
} // namespace libanet

#endif