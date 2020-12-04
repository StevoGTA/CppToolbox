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
// MARK: - Local proc declarations

CFDateFormatterStyle	sGetCFDateFormatterStyleForGregorianDateStringStyle(SGregorianDate::StringStyle stringStyle);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SUniversalTime

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SUniversalTime::getCurrentUniversalTime()
//----------------------------------------------------------------------------------------------------------------------
{
	// CFAbsoluteTimeGetCurrent is relative to 1/1/2001
	return (UniversalTime) ::CFAbsoluteTimeGetCurrent() + sOffsetInterval;
}

//----------------------------------------------------------------------------------------------------------------------
void SUniversalTime::setCurrentUniversalTime(UniversalTime time)
//----------------------------------------------------------------------------------------------------------------------
{
	// CFAbsoluteTimeGetCurrent is relative to 1/1/2001
	sOffsetInterval = time - (UniversalTime) ::CFAbsoluteTimeGetCurrent();
}

#if TARGET_OS_MACOS
//----------------------------------------------------------------------------------------------------------------------
UniversalTime SUniversalTime::getUniversalTimeForUTCDateTime(const UTCDateTime& utcDateTime)
//----------------------------------------------------------------------------------------------------------------------
{
	CFAbsoluteTime	time = ::CFAbsoluteTimeGetCurrent();
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

//----------------------------------------------------------------------------------------------------------------------
SGregorianDate::SGregorianDate(const CString& string, StringStyle dateStringStyle, StringStyle timeStringStyle)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create date formatter
	CFLocaleRef			localeRef = CFLocaleCopyCurrent();
	CFDateFormatterRef	dateFormatterRef =
								::CFDateFormatterCreate(kCFAllocatorDefault, localeRef,
										sGetCFDateFormatterStyleForGregorianDateStringStyle(dateStringStyle),
										sGetCFDateFormatterStyleForGregorianDateStringStyle(timeStringStyle));
	::CFRelease(localeRef);

	// Get time
	CFAbsoluteTime	time = ::CFAbsoluteTimeGetCurrent();
	CFStringRef		stringRef = CCoreFoundation::createStringRefFrom(string);
	::CFDateFormatterGetAbsoluteTimeFromString(dateFormatterRef, stringRef, nil, &time);
	::CFRelease(stringRef);
	::CFRelease(dateFormatterRef);

	// Convert to gregorian units
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
	// Setup
	UniversalTime	time;

	// Compose time
	CFCalendarRef	calendarRef = ::CFCalendarCopyCurrent();
	::CFCalendarComposeAbsoluteTime(calendarRef, &time, "yMdHms", mYear, mMonth, mDay, mHour, mMinute, (int) mSecond);
	time += mSecond - floor(mSecond);
	::CFRelease(calendarRef);

	return time;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
CFDateFormatterStyle sGetCFDateFormatterStyleForGregorianDateStringStyle(SGregorianDate::StringStyle stringStyle)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (stringStyle) {
		case SGregorianDate::kStringStyleShort:		return kCFDateFormatterShortStyle;
		case SGregorianDate::kStringStyleMedium:	return kCFDateFormatterMediumStyle;
		case SGregorianDate::kStringStyleLong:		return kCFDateFormatterLongStyle;
		case SGregorianDate::kStringStyleFull:		return kCFDateFormatterFullStyle;
		default:									return kCFDateFormatterNoStyle;
	}
}
