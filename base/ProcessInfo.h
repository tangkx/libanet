#ifndef LIBANET_BASE_PROCESSINFO_H
#define LIBANET_BASE_PROCESSINFO_H

#include "libanet/base/StringPiece.h"
#include "libanet/base/Timestamp.h"
#include "libanet/base/Types.h"
#include <sys/types.h>
#include <vector>

namespace libanet {

namespace ProcessInfo {

pid_t pid();
string pidString();
uid_t uid();
string username();
uid_t euid();
Timestamp startTime();
int clockTicksPerSecond();
int pageSize();
bool isDebugBuild();

string hostname();
string procname();
StringPiece procname(const string &stat);

string procStatus();
string procStat();
string threadStat();
string exePath();

int openedFiles();
int maxOpenFiles();

struct CpuTime {
	double userSeconds;
	double systemSeconds;

	CpuTime() : userSeconds(0.0), systemSeconds(0.0) {}
	double total() const {
		return userSeconds + systemSeconds;
	}
};

CpuTime cpuTime();

int numThreads();
std::vector<pid_t> threads();

} // namespace ProcessInfo
} // namespace libanet

#endif