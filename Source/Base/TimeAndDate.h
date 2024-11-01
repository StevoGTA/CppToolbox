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

// UniversalTime is the number of seconds relative to 00:00:00 on January 1, 2001
typedef Float64 UniversalTime;

const	UniversalTime			kUniversalTimeDistantFuture = 64092211200.0;
const	UniversalTime			kUniversalTimeDistantPast = -62135769600.0;

const	UniversalTimeInterval	kUniversalTimeInterval1904To2001	= 3061152000.0;
const	UniversalTimeInterval	kUniversalTimeInterval1970To2001	= 978307200.0;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SUniversalTime

struct SUniversalTime {
	// Methods
	public:
								// Class methods
		static	UniversalTime	getCurrent();
		static	void			setCurrent(UniversalTime time);

#if defined(TARGET_OS_MACOS)
		static	UniversalTime	get(const UTCDateTime& utcDateTime);
#endif
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianDate

struct SGregorianDate {
	// Enums
	public:
		// The exact formatted result for these styles depends on the locale and OS, but generally:
		enum ComponentStyle {
			// Do not process this component
			kComponentStyleNone,

			// Completely numeric, such as "1/1/52" or "3:30pm"
			kComponentStyleShort,

			// Longer, such as "Jan 12, 1952"
			kComponentStyleMedium,

			// Even longer, such as "January 12, 1952" or "3:30:32pm"
			kComponentStyleLong,

			// Pretty complete; e.g. "Tuesday, April 12, 1952 AD" or "3:30:42pm PST"
			kComponentStyleFull,
		};

		enum StringStyle {
			// HH:mm:ss
			kStringStyleHH_MM_SS,

			// HH:mm
			kStringStyleHH_MM,

			// HHmm
			kStringStyleHHMM,

			// ddMM
			kStringStyleDDMM,

			// MM/dd
			kStringStyleMMDD,

			// "yyyy-MM-dd'T'HH:mm:ss.SX"
			// "yyyy-MM-dd'T'HH:mm:ss.SSX"
			// "yyyy-MM-dd'T'HH:mm:ss.SSSX"
			// "yyyy-MM-dd'T'HH:mm:ss.SSSZ"
			// "yyyy-MM-dd'T'HH:mm:ss.SSSSX"
			// "yyyy-MM-dd'T'HH:mm:ss.SSSSSX"
			// "yyyy-MM-dd'T'HH:mm:ss.SSSSSSX"
			// "yyyy-MM-dd'T'HH:mm:ss.SSSSSSSX"
			// "yyyy-MM-dd'T'HH:mm:ss.SSSSSSSzzz"
			//	2021-03-23T22:54:13.922-0700
			kStringStyleRFC339Extended,

			// yyyy-MM-dd
			kStringStyleYYYY_MM_DD,

			// yyyy
			kStringStyleYYYY,
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
									SGregorianDate(UInt32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute,
											Float32 second, UInt8 dayOfWeek = 0) :
										mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute),
												mSecond(second), mDayOfWeek(dayOfWeek)
										{}
									SGregorianDate(UniversalTime time = SUniversalTime::getCurrent());

									// Instance methods
			UInt32					getYear() const
										{ return mYear; }
			UInt8					getMonth() const
										{ return mMonth; }
			UInt8					getDay() const
										{ return mDay; }
			UInt8					getHour() const
										{ return mHour; }
			UInt8					getMinute() const
										{ return mMinute; }
			Float32					getSecond() const
										{ return mSecond; }
			UInt8					getDayOfWeek() const
										{ return mDayOfWeek; }

			UniversalTime			getUniversalTime() const;

			CString					getMonthString(bool abbrieviated = false) const
										{ return getMonthString(mMonth, abbrieviated); }
			CString					getDayOfWeekString(bool abbrieviated = false) const;
			CString					getAMString() const;
			CString					getPMString() const;
			CString					getString(ComponentStyle dateComponentStyle,
											ComponentStyle timeComponentStyle) const;
			CString					getString(StringStyle stringStyle = kStringStyleRFC339Extended) const;

			bool					operator==(const SGregorianDate& other) const
										{ return (mYear == other.mYear) && (mMonth == other.mMonth) &&
												(mDay == other.mDay) && (mHour == other.mHour) &&
												(mMinute == other.mMinute) && (mSecond == other.mSecond); }
			bool					operator!=(const SGregorianDate& other) const
										{ return (mYear != other.mYear) || (mMonth != other.mMonth) ||
												(mDay != other.mDay) || (mHour != other.mHour) ||
												(mMinute != other.mMinute) || (mSecond != other.mSecond); }
			SGregorianDate			operator+(const Units& units) const;
			SGregorianDate&			operator+=(const Units& units)
										{ *this = *this + units; return *this; }

									// Class methods
	static	CString					getMonthString(UInt8 month, bool abbrieviated = false);
	static	UInt8					getMaxDays(UInt8 month);

	static	OV<SGregorianDate>		getFrom(const CString& string, ComponentStyle dateComponentStyle,
											ComponentStyle timeComponentStyle);
	static	OV<SGregorianDate>		getFrom(const CString& string,
											StringStyle stringStyle = kStringStyleRFC339Extended);
	static	OV<SGregorianDate>		getFrom(const OR<CString>& string,
											StringStyle stringStyle = kStringStyleRFC339Extended)
										{ return (string.hasReference()) ?
												getFrom(*string, stringStyle) : OV<SGregorianDate>(); }

	static	SGregorianDate			forHourMinuteSecond(UInt8 hour, UInt8 minute, Float32 second)
										{ return SGregorianDate(0, 0, 0, hour, minute, second); }
	static	SGregorianDate			forHourMinute(UInt8 hour, UInt8 minute)
										{ return SGregorianDate(0, 0, 0, hour, minute, 0.0); }
	static	SGregorianDate			forMonthDay(UInt8 month, UInt8 day)
										{ return SGregorianDate(0, month, day, 0, 0, 0.0); }
	static	SGregorianDate			forYear(UInt32 year)
										{ return SGregorianDate(year, 0, 0, 0, 0, 0.0); }
	static	SGregorianDate			forYearMonthDay(UInt32 year, UInt8 month, UInt8 day)
										{ return SGregorianDate(year, month, day, 0, 0, 0.0); }

	static	CString					getCurrentTimeZoneName();
	static	UniversalTimeInterval	getCurrentTimeZoneOffset();

	// Properties
	private:
		UInt32	mYear;		// i.e. 2010
		UInt8	mMonth;		// 1 - 12
		UInt8	mDay;		// 1 - 28/29/30/31
		UInt8	mHour;		// 0 - 23
		UInt8	mMinute;	// 0 - 59
		Float32	mSecond;	// 0 - 59.9
		UInt8	mDayOfWeek;	// 0 (Sun) - 6 (Sat)
};
