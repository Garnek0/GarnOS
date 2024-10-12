#include <garn/time.h>
#include <stdint.h>

timespec64_t kernelTime;

void time_set(timespec_t time){
	kernelTime.sec = (uint64_t)time.sec;
	kernelTime.nsec = time.nsec;
}

void time_set64(timespec64_t time){
	kernelTime = time;
}

timespec_t time_get(){
	timespec_t timespec32;
	timespec32.sec = (uint32_t)kernelTime.sec;
	timespec32.nsec = kernelTime.nsec;

	return timespec32;
}

timespec64_t time_get64(){
	return kernelTime;
}

timespec_t time_conv_to_unix(systime_t time){
	timespec64_t timespec64 = time_conv_to_unix64(time);

	timespec_t timespec;
	timespec.sec = (uint32_t)timespec64.sec;
	timespec.nsec = timespec64.nsec;

	return timespec;
}

static int _time_days_from_civil(int y, int m, int d){
	y -= m <= 2;
	int era = (y >= 0 ? y : y - 399) / 400;
	int yoe = y - era * 400;
	int doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
	int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return era * 146097 + doe - 719468;
}

void _time_civil_from_days(int64_t daysSinceEpoch, int* year, int* month, int* day){
	uint64_t time = daysSinceEpoch + 719468;
	int era = (time >= 0 ? time : time - 146096) / 146097;
	unsigned int doe = time - era * 146097;
	unsigned int yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
	int y = yoe + era * 400;
	unsigned int doy = doe - (365*yoe + yoe/4 - yoe/100);
	unsigned int mp = (5*doy + 2)/153;
	unsigned int d = doy - (153*mp+2)/5 + 1;
	unsigned int m = mp + (mp < 10 ? 3 : -9);

	*year = y + (m <= 2);
	*month = m;
	*day = d;
}

timespec64_t time_conv_to_unix64(systime_t time){
	int days = _time_days_from_civil(time.year, time.month, time.dayOfMonth);

	timespec64_t timespec;
	timespec.sec = (uint64_t)((days * 86400) + (time.hours * 60 * 60) + (time.minutes * 60) + time.seconds);
	timespec.nsec = 0;

	return timespec;
}

systime_t time_conv_to_systime(timespec_t time){
	
}

systime_t time_conv_to_systime64(timespec64_t time){
	int year;
	unsigned int month;
	unsigned int day;

	uint64_t unixtime = time.sec;
	systime_t systime;

	int64_t daysSinceEpoch = unixtime / (60*60*24);
	_time_civil_from_days(daysSinceEpoch, &year, &month, &day);

	systime.seconds = unixtime % 60;
	systime.minutes = (unixtime / 60) % 60;
	systime.hours = (unixtime / (60*60)) % 24;
	systime.dayOfMonth = day;
	systime.month = month;
	systime.year = year;

	return systime;
}

