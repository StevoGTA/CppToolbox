//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDate.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Basic types

// UniversalTimeInterval is in units of seconds
typedef	Float64 UniversalTimeInterval;

const	UniversalTimeInterval	kUniversalTimeIntervalNone			= 0.0;
const	UniversalTimeInterval	kUniversalTimeIntervalSecond		= 1.0;
const	UniversalTimeInterval	kUniversalTimeIntervalNanosecond	= kUniversalTimeIntervalSecond / 1000000000.0;
const	UniversalTimeInterval	kUniversalTimeIntervalMicrosecond	= kUniversalTimeIntervalSecond / 1000000.0;
const	UniversalTimeInterval	kUniversalTimeIntervalMillisecond	= kUniversalTimeIntervalSecond / 1000.0;
const	UniversalTimeInterval	kUniversalTimeIntervalMinute		= kUniversalTimeIntervalSecond * 60.0;
const	UniversalTimeInterval	kUniversalTimeIntervalHour			= kUniversalTimeIntervalMinute * 60.0;
const	UniversalTimeInterval	kUniversalTimeIntervalDay			= kUniversalTimeIntervalHour * 24.0;

// UniversalTime is the number of seconds relative to 00:00:00 January 1, 2001
typedef Float64 UniversalTime;

const	UniversalTimeInterval	kUniversalTimeInterval1904To2001	= 3061152000.0;
const	UniversalTimeInterval	kUniversalTimeInterval1970To2001	= 978307200.0;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SUniversalTime

struct SUniversalTime {
							// Class methods
	static	UniversalTime	getCurrentUniversalTime();
	static	void			setCurrentUniversalTime(UniversalTime time);

#if TARGET_OS_MACOS
	static	UniversalTime	getUniversalTimeForUTCDateTime(const UTCDateTime& utcDateTime);
#endif
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianDate

struct SGregorianDate {
	// Enums
	public:
		// The exact formatted result for these styles depends on the locale and OS, but generally:
		//		Short is completely numeric, such as "1/1/52" or "3:30pm"
		//		Medium is longer, such as "Jan 12, 1952"
		//		Long is longer, such as "January 12, 1952" or "3:30:32pm"
		//		Full is pretty complete; e.g. "Tuesday, April 12, 1952 AD" or "3:30:42pm PST"
		enum StringStyle {
			kStringStyleNone,
			kStringStyleShort,
			kStringStyleMedium,
			kStringStyleLong,
			kStringStyleFull,
		};

	// Structs
	public:
		struct Units {
			// Lifecycle methods
			Units(SInt32 years, SInt32 months, SInt32 days, SInt32 hours, SInt32 minutes, Float32 seconds) :
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

					// Lifecycle methods
					SGregorianDate(UInt32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, Float32 second,
							UInt8 dayOfWeek = 0) :
						mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute), mSecond(second),
								mDayOfWeek(dayOfWeek)
						{}
					SGregorianDate(UniversalTime time = SUniversalTime::getCurrentUniversalTime());
					SGregorianDate(const CString& string, StringStyle dateStringStyle = kStringStyleMedium,
							StringStyle timeStringStyle = kStringStyleShort);

					// Instance methods
	UniversalTime	getUniversalTime() const;
	CString			getString(StringStyle dateStringStyle = kStringStyleMedium,
							StringStyle timeStringStyle = kStringStyleShort) const;

	SGregorianDate	operator+(const Units& units) const;
	SGregorianDate&	operator+=(const Units& units);

	// Properties
	static	CString	mJanString;
	static	CString	mFebString;
	static	CString	mMarString;
	static	CString	mAprString;
	static	CString	mMayString;
	static	CString	mJunString;
	static	CString	mJulString;
	static	CString	mAugString;
	static	CString	mSepString;
	static	CString	mOctString;
	static	CString	mNovString;
	static	CString	mDecString;

	static	CString	mJanuaryString;
	static	CString	mFebruaryString;
	static	CString	mMarchString;
	static	CString	mAprilString;
	static	CString	mJuneString;
	static	CString	mJulyString;
	static	CString	mAugustString;
	static	CString	mSeptemberString;
	static	CString	mOctoberString;
	static	CString	mNovemberString;
	static	CString	mDecemberString;

	static	CString	mSunString;
	static	CString	mMonString;
	static	CString	mTueString;
	static	CString	mWedString;
	static	CString	mThuString;
	static	CString	mFriString;
	static	CString	mSatString;

	static	CString	mAMString;
	static	CString	mPMString;

			UInt32	mYear;		// i.e. 2010
			UInt8	mMonth;		// 1 - 12
			UInt8	mDay;		// 1 - 28/29/30/31
			UInt8	mHour;		// 0 - 23
			UInt8	mMinute;	// 0 - 59
			Float32	mSecond;	// 0 - 59.9
			UInt8	mDayOfWeek;	// 0 (Sun) - 6 (Sat)
};
