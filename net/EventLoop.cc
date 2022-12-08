#include "libanet/net/EventLoop.h"
#include "libanet/base/Logging.h"
#include "libanet/base/Mutex.h"
#include "libanet/net/Channel.h"
#include "libanet/net/Poller.h"
#include "libanet/net/SocketOps.h"
#include "libanet/net/TimerQueue.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace libanet;
using namespace libanet::net;

namespace {
__thread EventLoop *t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

int
createEventfd() {
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0) {
		LOG_SYSERR << "Failed in eventfd";
		abort();
	}
	return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
	IgnoreSigPipe() {
		::signal(SIGPIPE, SIG_IGN);
	}
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;

} // namespace

EventLoop *
EventLoop::getEventLoopOfCurrentThread() {
	return t_loopInThisThread;
}

EventLoop::EventLoop()
: looping_(false), quit_(false), eventHandling_(false),
  callingPendingFunctors_(false), iteration_(0),
  threadId_(CurrentThread::tid()), poller_(Poller::newDefaultPoller(this)),
  timerQueue_(new TimerQueue(this)), wakeupFd_(createEventfd()),
  wakeupChannel_(new Channel(this, wakeupFd_)), currentActiveChannel_(NULL) {

	LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
	if (t_loopInThisThread) {
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread
				  << " exists in this thread " << threadId_;
	} else {
		t_loopInThisThread = this;
	}

	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));

	wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
	LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
			  << " destructs in thread " << CurrentThread::tid();
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread = NULL;
}

void
EventLoop::loop() {
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;
	LOG_TRACE << "EventLoop " << this << " start looping";

	while (!quit_) {
		activeChannels_.clear();
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
		++iteration_;
		if (Logger::logLevel() <= Logger::TRACE) {
			printActiveChannels();
		}

		eventHandling_ = true;
		for (Channel *channel : activeChannels_) {
			currentActiveChannel_ = channel;
			currentActiveChannel_->handleEvent(pollReturnTime_);
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		doPendingFunctors();
	}

	LOG_TRACE << "EventLoop " << this << " stop looping";
	looping_ = false;
}

void
EventLoop::quit() {
	quit_ = true;
	if (!isInLoopThread()) {
		wakeup();
	}
}

void
EventLoop::runInLoop(Functor cb) {
	if (isInLoopThread()) {
		cb();
	} else {
		queueInLoop(std::move(cb));
	}
}

void
EventLoop::queueInLoop(Functor cb) {
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));
	}
	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}

size_t
EventLoop::queueSize() const {
	MutexLockGuard lock(mutex_);
	return pendingFunctors_.size();
}

