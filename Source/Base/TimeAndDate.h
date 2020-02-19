//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDate.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Basic types

// UniversalTimeInterval is in units of seconds
typedef	Float64 UniversalTimeInterval;

const	UniversalTimeInterval	kUniversalTimeIntervalSecond		= 1.0;
const	UniversalTimeInterval	kUniversalTimeIntervalNanosecond	= kUniversalTimeIntervalSecond / 1000000000.0;
const	UniversalTimeInterval	kUniversalTimeIntervalMicrosecond	= kUniversalTimeIntervalSecond / 1000000.0;
const	UniversalTimeInterval	kUniversalTimeIntervalMillisecond	= kUniversalTimeIntervalSecond / 1000.0;
const	UniversalTimeInterval	kUniversalTimeIntervalMinute		= kUniversalTimeIntervalSecond * 60.0;
const	UniversalTimeInterval	kUniversalTimeIntervalHour			= kUniversalTimeIntervalMinute * 60.0;
const	UniversalTimeInterval	kUniversalTimeIntervalDay			= kUniversalTimeIntervalHour * 24.0;
const	UniversalTimeInterval	kUniversalTimeIntervalNone			= 0.0;
const	UniversalTimeInterval	kUniversalTimeIntervalNever			= HUGE_VAL;

// UniversalTime is the number of seconds relative to a reference time.
//	On macOS,			the reference time is 00:00:00 January 1, 2001
//	On Angstrom 2.6.29,	the reference time is 09:13:00 January 27, 2009
typedef Float64 UniversalTime;

const	UniversalTime			kUniversalTimeNone					= 0.0;
const	UniversalTime			kUniversalTimeNever					= HUGE_VAL;
const	UniversalTimeInterval	kUniversalTimeInterval1904To2001	= 3061152000.0;
const	UniversalTimeInterval	kUniversalTimeInterval1970To2001	= 978307200.0;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianDate

struct SGregorianDate {
	// Lifecycle methods
	SGregorianDate(UInt32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, Float32 second, UInt8 dayOfWeek = 0) :
		mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute), mSecond(second), mDayOfWeek(dayOfWeek)
		{}

	// Properties
    UInt32	mYear;		// i.e. 2010
    UInt8	mMonth;		// 1 - 12
    UInt8	mDay;		// 1 - 28/29/30/31
    UInt8	mHour;		// 0 - 23
    UInt8	mMinute;	// 0 - 59
    Float32	mSecond;	// 0 - 59.9
	UInt8	mDayOfWeek;	// 0 (Sun) - 6 (Sat)
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianUnits

struct SGregorianUnits {
	// Lifecycle methods
	SGregorianUnits(SInt32 years, SInt32 months, SInt32 days, SInt32 hours, SInt32 minutes, Float32 seconds) :
		mYears(years), mMonths(months), mDays(days), mHours(hours), mMinutes(minutes), mSeconds(seconds)
		{}

	// Properties
    SInt32	mYears;
    SInt32	mMonths;
    SInt32	mDays;
    SInt32	mHours;
    SInt32	mMinutes;
    Float32	mSeconds;
};
