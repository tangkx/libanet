#include "libanet/net/EventLoopThread.h"
#include "libanet/net/EventLoop.h"

using namespace libanet;
using namespace libanet::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
								 const string &name)
: loop_(NULL), exiting_(),
  thread_(std::bind(&EventLoopThread::threadFunc, this), name), mutex_(),
  cond_(mutex_), callback_(cb) {}

EventLoopThread::~EventLoopThread() {
	exiting_ = true;
	if (loop_ != NULL) {
		loop_->quit();
		thread_.join();
	}
}

EventLoop *
EventLoopThread::startLoop() {
	assert(!thread_.started());
	thread_.start();
	EventLoop *loop = NULL;
	{
		MutexLockGuard lock(mutex_);
		while (loop_ == NULL) {
			cond_.wait();
		}
		loop = loop_;
	}

	return loop;
}

void
EventLoopThread::threadFunc() {
	EventLoop loop;
	if (callback_) {
		callback_(&loop);
	}
	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		cond_.notify();
	}

	loop.loop();
	MutexLockGuard lock(mutex_);
	loop_ = NULL;
}