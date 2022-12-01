#include "libanet/base/Logging.h"
#include "libanet/base/CurrentThread.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace libanet {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno) {
    // return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

}