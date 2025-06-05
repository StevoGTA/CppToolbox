//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDate-Apple.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "TimeAndDate.h"

#include "CCoreFoundation.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	UniversalTimeInterval	sOffsetInterval = 0.0;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SUniversalTime

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SUniversalTime::getCurrent()
//----------------------------------------------------------------------------------------------------------------------
{
	// CFAbsoluteTimeGetCurrent is relative to 1/1/2001
	return (UniversalTime) ::CFAbsoluteTimeGetCurrent() + sOffsetInterval;
}

//----------------------------------------------------------------------------------------------------------------------
void SUniversalTime::setCurrent(UniversalTime time)
//----------------------------------------------------------------------------------------------------------------------
{
	// CFAbsoluteTimeGetCurrent is relative to 1/1/2001
	sOffsetInterval = time - (UniversalTime) ::CFAbsoluteTimeGetCurrent();
}

#if defined(TARGET_OS_MACOS)
//----------------------------------------------------------------------------------------------------------------------
UniversalTime SUniversalTime::get(const UTCDateTime& utcDateTime)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get current time to set as default
	CFAbsoluteTime	time = ::CFAbsoluteTimeGetCurrent();

	// Convert to time
	::UCConvertUTCDateTimeToCFAbsoluteTime(&utcDateTime, &time);

	return (UniversalTime) time;
}
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianDate

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
SGregorianDate::SGregorianDate(UniversalTime time)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFCalendarRef	calendarRef = ::CFCalendarCopyCurrent();
	int				second;
	::CFCalendarDecomposeAbsoluteTime(calendarRef, (CFAbsoluteTime) time, "yMdHmsE", &mYear, &mMonth, &mDay, &mHour,
			&mMinute, &second, &mDayOfWeek);
	mSecond = (Float32) second + time - floor(time);
	::CFRelease(calendarRef);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SGregorianDate::getUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose time
	CFCalendarRef	calendarRef = ::CFCalendarCopyCurrent();
	UniversalTime	time;
	::CFCalendarComposeAbsoluteTime(calendarRef, &time, "yMdHms", mYear, mMonth, mDay, mHour, mMinute, (int) mSecond);
	time += mSecond - floor(mSecond);
	::CFRelease(calendarRef);

	return time;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
SGregorianDate SGregorianDate::operator+(const Units& units) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Float32	seconds = mSecond + units.mSeconds;

	// Compose time
	CFCalendarRef	calendarRef = ::CFCalendarCopyCurrent();
	UniversalTime	time;
	::CFCalendarComposeAbsoluteTime(calendarRef, &time, "yMdHms", mYear + units.mYears, mMonth + units.mMonths,
			mDay + units.mDays, mHour + units.mHours, mMinute + units.mMinutes, seconds);
	time += mSecond - floor(seconds);
	::CFRelease(calendarRef);

	return SGregorianDate(time);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getCurrentTimeZoneName()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFTimeZoneRef	timeZoneRef = ::CFTimeZoneCopyDefault();
	CFLocaleRef		localeRef = ::CFLocaleCopyCurrent();
	CFStringRef		stringRef =
							::CFTimeZoneCopyLocalizedName(timeZoneRef,
									::CFTimeZoneIsDaylightSavingTime(timeZoneRef, ::CFAbsoluteTimeGetCurrent()) ?
											kCFTimeZoneNameStyleShortDaylightSaving :
											kCFTimeZoneNameStyleShortStandard,
									localeRef);
	::CFRelease(timeZoneRef);
	::CFRelease(localeRef);

	// Compose string
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval SGregorianDate::getCurrentTimeZoneOffset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Get offset
	CFTimeZoneRef	timeZoneRef = ::CFTimeZoneCopyDefault();
	CFTimeInterval	offset = ::CFTimeZoneGetSecondsFromGMT(timeZoneRef, ::CFAbsoluteTimeGetCurrent());
	::CFRelease(timeZoneRef);

	return offset;
}
