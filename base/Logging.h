#ifndef LIBANET_BASE_LOGGING_H
#define LIBANET_BASE_LOGGING_H

#include "libanet/base/LogStream.h"
#include "libanet/base/Timestamp.h"

namespace libanet {

class TimeZone;

class Logger {

public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    class SourceFile {
        
    public:
        template<int N>
        SourceFile(const char (&arr)[N])
            :data_(arr), size_(N - 1) {
            
            const char* slash = strrchr(data_, '/');
            if (slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data - arr);
            }
        }

        explicit SourceFile(const char* filename)
            : data_(filename) {

            const char* slash = strrchr(filename, '/');
            if (slash) {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

        const char* data_;
        int size_;
    };

Logger(SourceFile file, int line);
Logger(SourceFile file, int line, LogLevel level);
Logger(SourceFile file, int line, LogLevel level, const char* func);
Logger(SourceFile file, int line, bool toAbort);
~Logger();

LogStream& stream() { return impl_.stream_; }

static LogLevel logLevel();
static void setLogLevel(LogLevel level);

typedef void (*OutputFunc)(const char* msg, int len);
typedef void (*FlushFunc)();
static void setOutput(OutputFunc);
static void setFlush(FlushFunc);
static void setTimeZone(const TimeZone& tz);

private:
    class Impl {
    
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void formatTime();
        void finish();

        Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;

    };

    Impl impl_;

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}

#define LOG_TRACE if (libanet::Logger::logLevel() <= libanet::Logger::TRACE) \
    libanet::Logger(__FILE__, __LINE__, libanet::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (libanet::Logger::logLevel() <= libanet::Logger::DEBUG) \
    libanet::Logger(__FILE__, __LINE__, libanet::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (libanet::Logger::logLevel() <= libanet::Logger::INFO) \
    libanet::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN libanet::Logger(__FILE__, __LINE__, libanet::Logger::WARN).stream()
#define LOG_ERROR libanet::Logger(__FILE__, __LINE__, libanet::Logger::ERROR).stream()
#define LOG_FATAL libanet::Logger(__FILE__, __LINE__, libanet::Logger::FATAL).stream()
#define LOG_SYSERR libanet::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL libanet::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

#define CHECK_NOTNULL(val) \
    ::libanet::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non Null", (val))

template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr) {
    if (ptr == NULL) {
        Logger(file, line, Logger::FATAL).stream() << names;
    }

    return ptr;
}

}
#endif