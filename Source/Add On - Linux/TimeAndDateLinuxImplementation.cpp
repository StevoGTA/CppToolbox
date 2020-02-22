//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDateLinuxImplementation.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "TimeAndDate.h"

#include <time.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	UniversalTimeInterval	sOffsetInterval = 0.0;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SUniversalTime

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SUniversalTime::getCurrentUniversalTime()
//----------------------------------------------------------------------------------------------------------------------
{
	// (needs verification) gettimeofday is relative to 1/1/2001
	struct	timeval	tv;
	gettimeofday(&tv, nil);

	return (UniversalTime) tv.tv_sec + (UniversalTime) tv.tv_usec / 1000000.0 + sOffsetInterval;
}

//----------------------------------------------------------------------------------------------------------------------
void SUniversalTime::setCurrentUniversalTime(UniversalTime time)
//----------------------------------------------------------------------------------------------------------------------
{
	// (needs verification) gettimeofday is relative to 1/1/2001
	Float64	timeI;
	UniversalTime	timeD = modf(time, &timeI);

	timeval	tv;
	tv.tv_sec = timeI;
	tv.tv_usec = timeD * 1000000.0;
	settimeofday(&tv, nil);

	printf("Setting HW clock\n");
	system("/sbin/hwclock -w");

	printf("Verifying HW clock\n");
	system("/sbin/hwclock -s");
	system("date");
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianDate

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
SGregorianDate::SGregorianDate(UniversalTime time)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Float64	timeI;
	UniversalTime	timeD = modf(time, &timeI);

	time_t	theTimeT = timeI;
	tm*	theTM = localtime(&theTimeT);

	// Store
	mYear = theTM->tm_year + 1900;
	mMonth = theTM->tm_mon + 1;
	mDay = theTM->tm_mday;
	mHour = theTM->tm_hour;
	mMinute = theTM->tm_min;
	mSecond = (Float32) theTM->tm_sec + timeD;
	mDayOfWeek = theTM->tm_wday;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SGregorianDate::getUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	tm	theTM;
	theTM.tm_year = date.mYear - 1900;
	theTM.tm_mon = date.mMonth - 1;
	theTM.tm_mday = date.mDay;
	theTM.tm_hour = date.mHour;
	theTM.tm_min = date.mMinute;
	theTM.tm_sec = date.mSecond;

	time_t	t = mktime(&theTM);

	Float32	timeI;
	Float32	timeD = modff(date.mSecond, &timeI);

	return (UniversalTime) t + timeD + kUniversalTimeInterval1970To2001;
}
