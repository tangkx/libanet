#include "libanet/base/Thread.h"
#include "libanet/base/CurrentThread.h"
#include "libanet/base/Exception.h"
#include "libanet/base/Logging.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace libanet {
namespace detail {

pid_t
gettid() {
	return static_cast<pid_t>(::syscall(SYS_gettid));
}

void
afterFork() {
	libanet::CurrentThread::t_cachedTid = 0;
	libanet::CurrentThread::t_threadName = "main";
	CurrentThread::tid();
}

class ThreadNameInitializer {

public:
	ThreadNameInitializer() {
		libanet::CurrentThread::t_threadName = "main";
		CurrentThread::tid();
		pthread_atfork(NULL, NULL, &afterFork);
	}
};

ThreadNameInitializer init;

struct ThreadData {
	typedef libanet::Thread::ThreadFunc ThreadFunc;
	ThreadFunc func_;
	string name_;
	pid_t *tid_;
	CountDownLatch *latch_;

	ThreadData(ThreadFunc func, const string &name, pid_t *tid,
			   CountDownLatch *latch)
	: func_(std::move(func)), name_(name), tid_(tid), latch_(latch) {}

	void runInThread() {
		*tid_ = libanet::CurrentThread::tid();
		tid_ = NULL;
		latch_->countDown();
		latch_ = NULL;

		libanet::CurrentThread::t_threadName =
			name_.empty() ? "libanetThread" : name_.c_str();
		::prctl(PR_SET_NAME, libanet::CurrentThread::t_threadName);
		try {
			func_();
			libanet::CurrentThread::t_threadName = "finished";
		} catch (const Exception &ex) {
			libanet::CurrentThread::t_threadName = "crashed";
			fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
			fprintf(stderr, "reason: %s\n", ex.what());
			fprintf(stderr, "stack trace: %s\n", ex.what());
			abort();
		} catch (const std::exception &ex) {
			libanet::CurrentThread::t_threadName = "crashed";
			fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
			fprintf(stderr, "reason: %s\n", ex.what());
			abort();
		} catch (...) {
			libanet::CurrentThread::t_threadName = "crashed";
			fprintf(stderr, "unknown exception caught in Thread %s\n",
					name_.c_str());
			throw;
		}
	}
};

void *
startThread(void *obj) {
	ThreadData *data = static_cast<ThreadData *>(obj);
	data->runInThread();
	delete data;
	return NULL;
}

} // namespace detail

void
CurrentThread::cacheTid() {
	if (t_cachedTid == 0) {
		t_cachedTid = detail::gettid();
		t_tidStringLength =
			snprintf(t_tidString, sizeof(t_tidString), "%5d", t_cachedTid);
	}
}

bool
CurrentThread::isMainThread() {
	return tid() == ::getpid();
}

void
CurrentThread::sleepUsec(int64_t usec) {
	struct timespec ts = {0, 0};
}

} // namespace libanet