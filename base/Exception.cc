#include "libanet/base/Exception.h"
#include "libanet/base/CurrentThread.h"

namespace libanet {

Exception::Exception(string msg)
: message_(std::move(msg)), stack_(CurrentThread::stackTrace(false)) {}

} // namespace libanet