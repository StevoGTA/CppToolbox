//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDate-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "TimeAndDate.h"

#include <time.h>
#include <sys/timeb.h>

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
	// _ftime64_s is relative to 1/1/1970
	struct	_timeb timebuffer;
	::_ftime64_s(&timebuffer);

	return timebuffer.time + (UniversalTime) timebuffer.millitm / 1000.0 - kUniversalTimeInterval1970To2001 +
			sOffsetInterval;
}

//----------------------------------------------------------------------------------------------------------------------
void SUniversalTime::setCurrentUniversalTime(UniversalTime time)
//----------------------------------------------------------------------------------------------------------------------
{
	// _ftime64_s is relative to 1/1/1970
	struct	_timeb timebuffer;
	::_ftime64_s(&timebuffer);

	sOffsetInterval =
			time - (timebuffer.time + (UniversalTime)timebuffer.millitm / 1000.0 - kUniversalTimeInterval1970To2001);
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
			__time64_t	time64 = (__time64_t) time + kUniversalTimeInterval1970To2001;
	struct	tm			theTM;
	_localtime64_s(&theTM, &time64);

	// Store
	mYear = theTM.tm_year + 1900;
	mMonth = theTM.tm_mon + 1;
	mDay = theTM.tm_mday;
	mHour = theTM.tm_hour;
	mMinute = theTM.tm_min;
	mSecond = (Float32) theTM.tm_sec + (Float32) ::fmod(time, 1);
	mDayOfWeek = theTM.tm_wday;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SGregorianDate::getUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	struct	tm	theTM;
	theTM.tm_year = mYear - 1900;
	theTM.tm_mon = mMonth - 1;
	theTM.tm_mday = mDay;
	theTM.tm_hour = mHour;
	theTM.tm_min = mMinute;
	theTM.tm_sec = (int) mSecond;

	__time64_t	time64 = _mktime64(&theTM);

	return (UniversalTime) time64 + ::fmod(mSecond, 1) + kUniversalTimeInterval1970To2001;
}
