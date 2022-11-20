#include "libanet/base/CountDownLatch.h"

using namespace libanet;

CountDownLatch::CountDownLatch(int count)
: mutex_(), condition_(mutex_), count_(count) {}

void
CountDownLatch::wait() {
	MutexLockGuard lock(mutex_);
	while (count_ > 0) {
		condition_.wait();
	}
}

void
CountDownLatch::countDown() {
	MutexLockGuard lock(mutex_);
	--count_;
	if (count_ == 0) {
		condition_.notify();
	}
}

int
CountDownLatch::getCount() const {
	MutexLockGuard lock(mutex_);
	return count_;
}
