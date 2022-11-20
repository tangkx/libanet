#ifndef LIBANET_BASE_MUTEX_H
#define LIBANET_BASE_MUTEX_H

#include "libanet/base/noncopyable.h"
#include <pthread.h>
#include <assert.h>

#define MCHECK(ret)                                                            \
	({                                                                         \
		__typeof__(ret) errnum = (ret);                                        \
		assert(errnum == 0);                                                   \
		(void)errnum;                                                          \
	})

namespace libanet {

class MutexLock : noncopyable {
public:
	MutexLock() : holder_(0) {
		MCHECK(pthread_mutex_init(&mutex_, NULL));
	}

	~MutexLock() {
		assert(holder_ == 0);
		MCHECK(pthread_mutex_destroy(&mutex_));
	}

	bool isLockedByThisThread() const {}

	void assertLocked() const {
		assert(isLockedByThisThread());
	}

	void lock() {
		MCHECK(pthread_mutex_lock(&mutex_));
		assignHolder();
	}

	void unlock() {
		unassignHolder();
		MCHECK(pthread_mutex_unlock(&mutex_));
	}

	pthread_mutex_t *getPthreadMutex() {
		return &mutex_;
	}

private:
	friend class Condition;

	class UnassignGuard : noncopyable {
	public:
		explicit UnassignGuard(MutexLock &owner) : owner_(owner) {
			owner_.unassignHolder();
		}

		~UnassignGuard() {
			owner_.assignHolder();
		}

	private:
		MutexLock &owner_;
	};

	void assignHolder() {}
	void unassignHolder() {
		holder_ = 0;
	}

	pthread_mutex_t mutex_;
	pid_t holder_;
};

class MutexLockGuard : noncopyable {

public:
	explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) {
		mutex_.lock();
	}

	~MutexLockGuard() {
		mutex_.unlock();
	}

private:
	MutexLock &mutex_;
};

} // namespace libanet

#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name")

#endif