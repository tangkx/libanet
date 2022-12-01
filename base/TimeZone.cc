#include "libanet/base/TimeZone.h"
#include "libanet/base/Date.h"
#include "libanet/base/noncopyable.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

using namespace libanet;

struct TimeZone::Data {

	struct Transition {
		int64_t utctime;
		int64_t localtime;
		int localtimeIdx;

		Transition(int64_t t, int64_t l, int localIdx)
		: utctime(t), localtime(l), localtimeIdx(localIdx) {}
	};

	struct LocalTime {
		int32_t utcOffset;
		bool isDst;
		int desigIdx;

		LocalTime(int32_t offset, bool dst, int idx)
		: utcOffset(offset), isDst(dst), desigIdx(idx) {}
	};

	void addLocalTime(int32_t utcOffset, bool isDst, int desigIdx) {
		localtimes.push_back(LocalTime(utcOffset, isDst, desigIdx));
	}

	void addTransition(int64_t utcTime, int localtimeIdx) {
		LocalTime lt = localtimes.at(localtimeIdx);
		transitions.push_back(
			Transition(utcTime, utcTime + lt.utcOffset, localtimeIdx));
	}

	const LocalTime *findLocalTime(int64_t utcTime) const;
	const LocalTime *findLocalTime(const struct DateTime &local,
								   bool postTransition) const;

	struct CompareUtcTime {
		bool operator()(const Transition &lhs, const Transition &rhs) {
			return lhs.utctime < rhs.utctime;
		}
	};

	struct CompareLocalTime {
		bool operator()(const Transition &lhs, const Transition &rhs) {
			return lhs.localtime < rhs.localtime;
		}
	};

	std::vector<Transition> transitions;
	std::vector<LocalTime> localtimes;
	string abbreviation;
	string tzstring;
};

namespace libanet {

const int kSecondsPerDay = 24 * 60 * 60;

namespace detail {

class File : noncopyable {

public:
	File(const char *file) : fp_(::fopen(file, "rb")) {}
	~File() {
		if (fp_) {
			::fclose(fp_);
		}
	}

	bool valid() const {
		return fp_;
	}

	string readBytes(int n) {
		char buf[n];
		ssize_t nr = ::fread(buf, 1, n, fp_);
		if (nr != n)
			throw std::logic_error("no enough data");

		return string(buf, n);
	}

	string readToEnd() {
		char buf[4096];
		string result;
		ssize_t nr = 0;
		while ((nr = ::fread(buf, 1, sizeof(buf), fp_)) > 0) {
			result.append(buf, nr);
		}

		return result;
	}

	int64_t readInt64() {
		int64_t x = 0;
		ssize_t nr = ::fread(&x, 1, sizeof(int64_t), fp_);
		if (nr != sizeof(int64_t)) {
			throw std::logic_error("bad int64_t data");
		}

		return be64toh(x);
	}

	int32_t readInt32() {
		int32_t x = 0;
		ssize_t nr = ::fread(&x, 1, sizeof(int32_t), fp_);
		if (nr != sizeof(int32_t)) {
			throw std::logic_error("bad int32_t data");
		}

		return be32toh(x);
	}

	uint8_t readUint8() {
		uint8_t x = 0;
		ssize_t nr = ::fread(&x, 1, sizeof(uint8_t), fp_);
		if (nr != sizeof(uint8_t)) {
			throw std::logic_error("bad uint8_t data");
		}

		return be64toh(x);
	}

	off_t skip(ssize_t bytes) {
		return ::fseek(fp_, bytes, SEEK_CUR);
	}

private:
	FILE *fp_;
};

bool
readDataBlock(File &f, struct TimeZone::Data *data, bool v1) {
	const int time_size = v1 ? sizeof(int32_t) : sizeof(int64_t);
	const int32_t isutccnt = f.readInt32();
	const int32_t isstdcnt = f.readInt32();
	const int32_t leapcnt = f.readInt32();
	const int32_t timecnt = f.readInt32();
	const int32_t typecnt = f.readInt32();
	const int32_t charcnt = f.readInt32();

	if (leapcnt != 0)
		return false;
	if (isutccnt != 0 && isutccnt != typecnt)
		return false;
	if (isstdcnt != 0 && isstdcnt != typecnt)
		return false;

	std::vector<int64_t> trans;
	trans.reserve(timecnt);
	for (int i = 0; i < timecnt; ++i) {
		if (v1) {
			trans.push_back(f.readInt32());
		} else {
			trans.push_back(f.readInt64());
		}
	}

	std::vector<int> localtimes;
	localtimes.reserve(timecnt);
	for (int i = 0; i < timecnt; ++i) {
		uint8_t local = f.readUint8();
		localtimes.push_back(local);
	}

	data->localtimes.reserve(typecnt);
	for (int i = 0; i < typecnt; ++i) {
		int32_t gmtoff = f.readInt32();
		uint8_t isdst = f.readUint8();
		uint8_t abbrind = f.readUint8();

		data->addLocalTime(gmtoff, isdst, abbrind);
	}

	for (int i = 0; i < timecnt; ++i) {
		int localIdx = localtimes[i];
		data->addTransition(trans[i], localIdx);
	}

	data->abbreviation = f.readBytes(charcnt);
	f.skip(leapcnt * (time_size + 4));
	f.skip(isstdcnt);
	f.skip(isutccnt);

	if (!v1) {
		data->tzstring = f.readToEnd();
	}

	return true;
}

bool
readTimeZoneFile(const char *zonefile, struct TimeZone::Data *data) {
	File f(zonefile);
	if (f.valid()) {
		try {
			string head = f.readBytes(4);
			if (head != "TZif")
				throw std::logic_error("bad head");
			string version = f.readBytes(1);
			f.readBytes(15);

			const int32_t isgmtcnt = f.readInt32();
			const int32_t isstdcnt = f.readInt32();
			const int32_t leapcnt = f.readInt32();
			const int32_t timecnt = f.readInt32();
			const int32_t typecnt = f.readInt32();
			const int32_t charcnt = f.readInt32();

			if (version == "2") {
				size_t skip = sizeof(int32_t) * timecnt + timecnt +
							  6 * typecnt + charcnt + 8 * leapcnt + isstdcnt +
							  isgmtcnt;
				f.skip(skip);

				head = f.readBytes(4);
				if (head != "TZif")
					throw std::logic_error("bad head");
				f.skip(16);
				return readDataBlock(f, data, false);
			} else {
				f.skip(-4 * 6);
				return readDataBlock(f, data, true);
			}
		} catch (std::logic_error &e) {
			fprintf(stderr, "%s\n", e.what());
		}
	}

	return false;
}

inline void
fillHMS(unsigned seconds, struct DateTime *dt) {
	dt->second = seconds % 60;
	unsigned minutes = seconds / 60;
	dt->minute = minutes % 60;
	dt->hour = minutes / 60;
}

DateTime
BreakTime(int64_t t) {
	struct DateTime dt;
	int seconds = static_cast<int>(t % kSecondsPerDay);
	int days = static_cast<int>(t / kSecondsPerDay);

	if (seconds < 0) {
		seconds += kSecondsPerDay;
		--days;
	}

	detail::fillHMS(seconds, &dt);
	Date date(days + Date::kJulianDayOf1970_01_01);
	Date::YearMonthDay ymd = date.yearMonthDay();
	dt.year = ymd.year;
	dt.month = ymd.month;
	dt.day = ymd.day;

	return dt;
}

} // namespace detail
} // namespace libanet

const TimeZone::Data::LocalTime *
TimeZone::Data::findLocalTime(int64_t utcTime) const {
	const LocalTime *local = NULL;
	if (transitions.empty() || utcTime < transitions.front().utctime) {
		local = &localtimes.front();
	} else {
		Transition sentry(utcTime, 0, 0);
		std::vector<Transition>::const_iterator transI = std::upper_bound(
			transitions.begin(), transitions.end(), sentry, CompareUtcTime());
		assert(transI != transitions.begin());
		if (transI != transitions.end()) {
			--transI;
			local = &localtimes[transI->localtimeIdx];
		} else {
			local = &localtimes[transitions.back().localtimeIdx];
		}
	}

	return local;
}

const TimeZone::Data::LocalTime *
TimeZone::Data::findLocalTime(const struct DateTime &lt,
							  bool postTransition) const {
	const int64_t localtime = fromUtcTime(lt);
	if (transitions.empty() || localtime < transitions.front().localtime) {
		return &localtimes.front();
	}

	Transition sentry(0, localtime, 0);
	std::vector<Transition>::const_iterator transI = std::upper_bound(
		transitions.begin(), transitions.end(), sentry, CompareUtcTime());
	assert(transI != transitions.begin());
	if (transI == transitions.end()) {
		return &localtimes[transitions.back().localtimeIdx];
	}

	Transition prior_trans = *(transI - 1);
	int64_t prior_second =
		transI->utctime - 1 + localtimes[prior_trans.localtimeIdx].utcOffset;

	if (prior_second < localtime) {
		if (postTransition) {
			return &localtimes[transI->localtimeIdx];
		} else {
			return &localtimes[prior_trans.localtimeIdx];
		}
	}

	--transI;
	if (transI != transitions.begin()) {
		prior_trans = *(transI - 1);
		prior_second = transI->utctime - 1 +
					   localtimes[prior_trans.localtimeIdx].utcOffset;
	}

	if (localtime <= prior_second) {
		if (postTransition) {
			return &localtimes[transI->localtimeIdx];
		} else {
			return &localtimes[prior_trans.localtimeIdx];
		}
	}

	return &localtimes[transI->localtimeIdx];
}

TimeZone
TimeZone::UTC() {
	return TimeZone(0, "UTC");
}

TimeZone
TimeZone::loadZoneFile(const char *zonefile) {
	std::unique_ptr<Data> data(new Data);
	if (!detail::readTimeZoneFile(zonefile, data.get())) {
		data.reset();
	}

	return TimeZone(std::move(data));
}

TimeZone::TimeZone(std::unique_ptr<Data> data) : data_(std::move(data)) {}

TimeZone::TimeZone(int eastOfUtc, const char *name)
: data_(new TimeZone::Data) {
	data_->addLocalTime(eastOfUtc, false, 0);
	data_->abbreviation = name;
}

struct DateTime
TimeZone::toLocalTime(int64_t seconds, int *utcOffset) const {
	struct DateTime localTime;
	assert(data_ != NULL);

	const Data::LocalTime *local = data_->findLocalTime(seconds);

	if (local) {
		localTime = detail::BreakTime(seconds + local->utcOffset);
		if (utcOffset) {
			*utcOffset = local->utcOffset;
		}
	}

	return localTime;
}

int64_t
TimeZone::fromLocalTime(const struct DateTime &localtime,
						bool postTransition) const {
	assert(data_ != NULL);
	const Data::LocalTime *local =
		data_->findLocalTime(localtime, postTransition);
	const int64_t localSeconds = fromUtcTime(localtime);
	if (local) {
		return localSeconds - local->utcOffset;
	}

	return localSeconds;
}

DateTime
TimeZone::toUtcTime(int64_t secondsSinceEpoch) {
	return detail::BreakTime(secondsSinceEpoch);
}

int64_t
TimeZone::fromUtcTime(const DateTime &dt) {
	Date date(dt.year, dt.month, dt.day);
	int secondsInDay = dt.hour * 3600 + dt.minute * 60 + dt.second;
	int64_t days = date.julianDayNumber() - Date::kJulianDayOf1970_01_01;
	return days * kSecondsPerDay + secondsInDay;
}

DateTime::DateTime(const struct tm &t)
: year(t.tm_year + 1900), month(t.tm_mon + 1), day(t.tm_mday), hour(t.tm_hour),
  minute(t.tm_min), second(t.tm_sec) {}

string
DateTime::toIsoString() const {
	char buf[64];
	snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", year, month,
			 day, hour, minute, second);

	return buf;
}