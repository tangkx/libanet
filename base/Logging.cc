#include "libanet/base/Logging.h"
#include "libanet/base/CurrentThread.h"
#include "libanet/base/TimeZone.h"
#include "libanet/base/Timestamp.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace libanet {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char *
strerror_tl(int savedErrno) {
	// return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::LogLevel
initLogLevel() {
	if (::getenv("LIBANET_LOG_TRACE"))
		return Logger::TRACE;
	else if (::getenv("LIBANET_LOG_DEBUG"))
		return Logger::DEBUG;
	else
		return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char *LogLevelName[Logger::NUM_LOG_LEVELS] = {
	"TRACE ", "DEBUG ", "INFO ", "WARN ", "ERROR ", "FATAL ",
};

class T {

public:
	T(const char *str, unsigned len) : str_(str), len_(len) {
		assert(strlen(str) == len_);
	}

	const char *str_;
	const unsigned len_;
};

inline LogStream &
operator<<(LogStream &s, T v) {
	s.append(v.str_, v.len_);
	return s;
}

inline LogStream &
operator<<(LogStream &s, const Logger::SourceFile &v) {
	s.append(v.data_, v.size_);
	return s;
}

void
defaultOutput(const char *msg, int len) {
	size_t n = fwrite(msg, 1, len, stdout);
	(void)n;
}

void
defaultFlush() {
	fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
TimeZone g_logTimeZone;

} // namespace libanet

using namespace libanet;
