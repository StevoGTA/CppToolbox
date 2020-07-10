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

CFDateFormatterStyle	sGetCFDateFormatterStyleForGregorianDateStringStyle(EGregorianDateStringStyle format);

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
SGregorianDate::SGregorianDate(const CString& string, EGregorianDateStringStyle dateStyle,
		EGregorianDateStringStyle timeStyle)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create date formatter
	CFLocaleRef			localeRef = CFLocaleCopyCurrent();
	CFDateFormatterRef	dateFormatterRef =
								::CFDateFormatterCreate(kCFAllocatorDefault, localeRef,
										sGetCFDateFormatterStyleForGregorianDateStringStyle(dateStyle),
										sGetCFDateFormatterStyleForGregorianDateStringStyle(timeStyle));
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
CFDateFormatterStyle sGetCFDateFormatterStyleForGregorianDateStringStyle(EGregorianDateStringStyle format)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (format) {
		case kGregorianDateStringStyleShort:	return kCFDateFormatterShortStyle;
		case kGregorianDateStringStyleMedium:	return kCFDateFormatterMediumStyle;
		case kGregorianDateStringStyleLong:		return kCFDateFormatterLongStyle;
		case kGregorianDateStringStyleFull:		return kCFDateFormatterFullStyle;
		default:								return kCFDateFormatterNoStyle;
	}
}




#if defined(__OBJC__)
//		static	NSCalendarDate*	calendarDateForGregorianDate(const SGregorianDate& date,
//										NSTimeZone* timeZone)
//									{ return [NSCalendarDate dateWithYear:date.mYear
//											month:date.mMonth day:date.mDay hour:date.mHour
//											minute:date.mMinute second:(NSUInteger) date.mSecond
//											timeZone:timeZone]; }
#endif
