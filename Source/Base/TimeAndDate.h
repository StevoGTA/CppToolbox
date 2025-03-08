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
	// ComponentStyle
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

	// StringStyle
	public:
		enum StringStyle {
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

			// yyyy-MM-ddTHH:mm:ss
			kStringStyleYYYY_MM_DDTHH_MM_SS,

			// yyyy-MM-ddTHH:mm
			kStringStyleYYYY_MM_DDTHH_MM,

			// yyyy-MM-ddTHH
			kStringStyleYYYY_MM_DDTHH,

			// yyyy-MM-dd
			kStringStyleYYYY_MM_DD,

			// yyyy-MM
			kStringStyleYYYY_MM,

			// yyyy
			kStringStyleYYYY,

			// MM/dd
			kStringStyleMMDD,

			// ddMM
			kStringStyleDDMM,

			// HH:mm:ss
			kStringStyleHH_MM_SS,

			// HH:mm
			kStringStyleHH_MM,

			// HHmm
			kStringStyleHHMM,
		};

		// Components
		public:
			struct Components {
				// Methods
				public:
											// Lifecycle methods
											Components(const OV<UInt32>& year = OV<UInt32>(),
													const OV<UInt8>& month = OV<UInt8>(),
													const OV<UInt8>& day = OV<UInt8>(),
													const OV<UInt8>& hour = OV<UInt8>(),
													const OV<UInt8>& minute = OV<UInt8>(),
													const OV<Float32>& second = OV<Float32>()) :
												mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute),
														mSecond(second)
												{}
											Components(UInt32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute,
													Float32 second) :
												mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute),
														mSecond(second)
												{}
											Components(UInt32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute) :
												mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute)
												{}
											Components(UInt32 year, UInt8 month, UInt8 day, UInt8 hour) :
												mYear(year), mMonth(month), mDay(day), mHour(hour)
												{}
											Components(UInt32 year, UInt8 month, UInt8 day) :
												mYear(year), mMonth(month), mDay(day)
												{}
											Components(UInt32 year, UInt8 month) : mYear(year), mMonth(month) {}
											Components(UInt32 year) : mYear(year) {}
											Components(const CDictionary& info);
											Components(const Components& other) :
												mYear(other.mYear), mMonth(other.mMonth), mDay(other.mDay),
														mHour(other.mHour), mMinute(other.mMinute),
														mSecond(other.mSecond)
												{}

											// Instance methods
					const	OV<UInt32>&		getYear() const
												{ return mYear; }
					const	OV<UInt8>&		getMonth() const
												{ return mMonth; }
					const	OV<UInt8>&		getDay() const
												{ return mDay; }
					const	OV<UInt8>&		getHour() const
												{ return mHour; }
					const	OV<UInt8>&		getMinute() const
												{ return mMinute; }
					const	OV<Float32>&	getSecond() const
												{ return mSecond; }

							CDictionary		getInfo() const;

							bool			operator==(const Components& other) const
												{ return (mYear == other.mYear) && (mMonth == other.mMonth) &&
														(mDay == other.mDay) && (mHour == other.mHour) &&
														(mMinute == other.mMinute) && (mSecond == other.mSecond); }
							bool			operator!=(const Components& other) const
												{ return !(*this == other); }

				// Properties
				private:
					OV<UInt32>	mYear;		// i.e. 2010
					OV<UInt8>	mMonth;		// 1 - 12
					OV<UInt8>	mDay;		// 1 - 28/29/30/31
					OV<UInt8>	mHour;		// 0 - 23
					OV<UInt8>	mMinute;	// 0 - 59
					OV<Float32>	mSecond;	// 0 - 59
			};

	// Units
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

	// Methods
	public:

										// Lifecycle methods
										SGregorianDate(UInt32 year, UInt8 month = 1, UInt8 day = 1, UInt8 hour = 0,
												UInt8 minute = 0, Float32 second = 0.0, UInt8 dayOfWeek = 0) :
											mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute),
													mSecond(second), mDayOfWeek(dayOfWeek)
											{}
										SGregorianDate(const Components& components) :
											mYear(components.getYear().getValue(1970)),
													mMonth(components.getMonth().getValue(1)),
													mDay(components.getDay().getValue(1)),
													mHour(components.getHour().getValue(0)),
													mMinute(components.getMinute().getValue(0)),
													mSecond(components.getSecond().getValue(0.0)), mDayOfWeek(0)
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
				CString					getHourString(bool use24HourFormat = false) const;
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
		static	UInt8					getMaxDays(UInt8 month);

		static	CString					getMonthString(UInt8 month, bool abbrieviated = false);
		static	CString					getAMString();
		static	CString					getPMString();

		static	OV<Components>			getComponentsFrom(const CString& string,
												StringStyle stringStyle = kStringStyleRFC339Extended);

		static	OV<SGregorianDate>		getFrom(const CString& string, ComponentStyle dateComponentStyle,
												ComponentStyle timeComponentStyle);
		static	OV<SGregorianDate>		getFrom(const CString& string,
												StringStyle stringStyle = kStringStyleRFC339Extended);
		static	OV<SGregorianDate>		getFrom(const OR<CString>& string,
												StringStyle stringStyle = kStringStyleRFC339Extended)
											{ return (string.hasReference()) ?
													getFrom(*string, stringStyle) : OV<SGregorianDate>(); }

		static	SGregorianDate			forMonthDay(UInt8 month, UInt8 day)
											{ return SGregorianDate(0, month, day, 0, 0, 0.0); }
		static	SGregorianDate			forHourMinuteSecond(UInt8 hour, UInt8 minute, Float32 second)
											{ return SGregorianDate(0, 0, 0, hour, minute, second); }
		static	SGregorianDate			forHourMinute(UInt8 hour, UInt8 minute)
											{ return SGregorianDate(0, 0, 0, hour, minute, 0.0); }

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
