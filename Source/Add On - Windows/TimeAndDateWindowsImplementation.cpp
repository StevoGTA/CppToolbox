//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDateWindowsImplementation.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "TimeAndDate.h"

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

	return timebuffer.time + (UniversalTime) timebuffer.millitm / 1000.0 + kUniversalTimeInterval1970To2001 +
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
			time - (timebuffer.time + (UniversalTime)timebuffer.millitm / 1000.0 + kUniversalTimeInterval1970To2001);
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
			__time64_t	time64 = (__time64_t) time;
	struct	tm			theTM;
	_localtime64_s(&theTM, &time64);

	// Store
	mYear = theTM.tm_year + 1900;
	mMonth = theTM.tm_mon + 1;
	mDay = theTM.tm_mday;
	mHour = theTM.tm_hour;
	mMinute = theTM.tm_min;
	mSecond = (Float32) theTM.tm_sec + (Float32) fmod(time, 1);
	mDayOfWeek = theTM.tm_wday;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SGregorianDate::getUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	{
	// Setup
	struct	tm	theTM;
	theTM.tm_year = date.mYear - 1900;
	theTM.tm_mon = date.mMonth - 1;
	theTM.tm_mday = date.mDay;
	theTM.tm_hour = date.mHour;
	theTM.tm_min = date.mMinute;
	theTM.tm_sec = (int) date.mSecond;

	__time64_t	time64 = _mktime64(&theTM);

	return (UniversalTime) time64 + fmod(date.mSecond, 1) + kUniversalTimeInterval1970To2001;
}
