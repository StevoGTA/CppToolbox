//----------------------------------------------------------------------------------------------------------------------
//	CTimeInfo.h			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Flags

enum {
	kTimeInfoTimeFlagsUseDays						= 0x00000000,
	kTimeInfoTimeFlagsUseDaysHours					= 0x00000001,
	kTimeInfoTimeFlagsUseDaysHoursMinutes			= 0x00000002,
	kTimeInfoTimeFlagsUseDaysHoursMinutesSeconds	= 0x00000003,
	kTimeInfoTimeFlagsUseHours						= 0x00000004,
	kTimeInfoTimeFlagsUseHoursMinutes				= 0x00000005,
	kTimeInfoTimeFlagsUseHoursMinutesSeconds		= 0x00000006,
	kTimeInfoTimeFlagsUseMinutes					= 0x00000007,
	kTimeInfoTimeFlagsUseMinutesSeconds				= 0x00000008,
	kTimeInfoTimeFlagsUseSeconds					= 0x00000009,
	
	kTimeInfoTimeFlagsUseHoursMinutesSecondsHMS		= 0x0000000A,
	kTimeInfoTimeFlagsUseMinutesSecondsHMS			= 0x0000000B,
	kTimeInfoTimeFlagsUseSecondsHMS					= 0x0000000C,
	
	kTimeInfoTimeFlagsUseSamples					= 0x00000010,
	
	kTimeInfoTimeFlagsUseFrames						= 0x00000020,
	
	kTimeInfoTimeFlagsUse24fpsTimecode				= 0x00000030,
	kTimeInfoTimeFlagsUse25fpsTimecode				= 0x00000031,
	kTimeInfoTimeFlagsUse30fpsDropFrameTimecode		= 0x00000032,
	kTimeInfoTimeFlagsUse30fpsNonDropFrameTimecode	= 0x00000033,
	
	kTimeInfoTimeFlagsUseMask						= 0x000000FF,
	
	kTimeInfoTimeFlagsRound							= 0x00000100,
	kTimeInfoTimeFlagsAddLabel						= 0x00001000,
	
	kTimeInfoTimeFlagsDefault						= kTimeInfoTimeFlagsUseHoursMinutesSeconds,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - String Style

// The exact formatted result for these styles depends on the locale and OS, but generally:
//		Short is completely numeric, such as "1/1/52" or "3:30pm"
//		Medium is longer, such as "Jan 12, 1952"
//		Long is longer, such as "January 12, 1952" or "3:30:32pm"
//		Full is pretty complete; e.g. "Tuesday, April 12, 1952 AD" or "3:30:42pm PST"
enum ETimeInfoStringStyle {
	kTimeInfoStringStyleNone,
	kTimeInfoStringStyleShort,
	kTimeInfoStringStyleMedium,
	kTimeInfoStringStyleLong,
	kTimeInfoStringStyleFull,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTimeInfo

class CDictionaryX;

class CTimeInfoInternals;
class CTimeInfo {
	// Methods
	public:
								// Lifecycle methods
								CTimeInfo(Float32 secondCount = 0.0);
								CTimeInfo(UInt32 flags, Float32 rate, SInt64 count);
								CTimeInfo(const CDictionary& info);
								CTimeInfo(const CTimeInfo& other);
								~CTimeInfo();
		
								// Instance methods
								// Seconds
				void			setSecondCount(Float32 secondCount);
				void			addSecondCount(Float32 secondCount);
				Float32			getSecondCount() const;
		
								// Samples
				void			setSampleRate(Float32 sampleRate);
				Float32			getSampleRate() const;

				void			setSampleCount(SInt64 sampleCount);
				void			addSampleCount(SInt64 sampleCount);
				SInt64			getSampleCount() const;
				
								// Frames
				void			setFrameRate(Float32 frameRate);
				Float32			getFrameRate() const;
				
				void			setFrameCount(SInt64 frameCount);
				void			addFrameCount(SInt64 frameCount);
				SInt64			getFrameCount() const;
				
								// Getting / Setting info
				CString			getTimeAsString(UInt32 flags = kTimeInfoTimeFlagsDefault) const;
				void			setTimeFromString(const CString& string,
										UInt32 flags = kTimeInfoTimeFlagsUseHoursMinutesSeconds);

#if TARGET_OS_MACOS
								// Quicktime
				TimeValue		getTimeValue(TimeScale timeScale) const;
#endif
				
								// Storage
				CDictionary		getInfo() const;

								// Convenience functions
				bool			operator==(const CTimeInfo& other) const;
				bool			operator!=(const CTimeInfo& other) const;
				bool			operator<(const CTimeInfo& other) const;
				bool			operator<=(const CTimeInfo& other) const;
				bool			operator>(const CTimeInfo& other) const;
				bool			operator>=(const CTimeInfo& other) const;

				CTimeInfo&		operator=(const CTimeInfo& other);
				CTimeInfo&		operator+=(const CTimeInfo& other);
				CTimeInfo&		operator-=(const CTimeInfo& other);

				CTimeInfo		operator+(const CTimeInfo& other) const;
				CTimeInfo		operator-(const CTimeInfo& other) const;

								// Class methods
		static	bool			doTimeWindowsOverlap(const CTimeInfo& start1, const CTimeInfo& length1,
										const CTimeInfo& start2, const CTimeInfo& length2);

		static	UniversalTime	getCurrentUniversalTime();
		static	void			setCurrentUniversalTime(UniversalTime time);

		static	CString			getStringForGregorianDate(const SGregorianDate& date,
										ETimeInfoStringStyle dateStyle = kTimeInfoStringStyleMedium,
										ETimeInfoStringStyle timeStyle = kTimeInfoStringStyleShort)
									{
										return getStringForUniversalTime(getUniversalTimeForGregorianDate(date),
												dateStyle, timeStyle);
									}
		static	CString			getStringForUniversalTime(UniversalTime time,
										ETimeInfoStringStyle dateStyle = kTimeInfoStringStyleMedium,
										ETimeInfoStringStyle timeStyle = kTimeInfoStringStyleShort);
		static	UniversalTime	getUniversalTimeForString(const CString& string,
										ETimeInfoStringStyle dateStyle = kTimeInfoStringStyleMedium,
										ETimeInfoStringStyle timeStyle = kTimeInfoStringStyleShort);
		static	SGregorianDate	getGregorianDateForUniversalTime(UniversalTime time);
		static	UniversalTime	getUniversalTimeForGregorianDate(const SGregorianDate& date);
		static	UniversalTime	addGregorianUnitsToUniversalTime(UniversalTime time, const SGregorianUnits& units);
										
#if TARGET_OS_MACOS
		static	UniversalTime	getUniversalTimeForUTCDateTime(const UTCDateTime& utcDateTime);
		
#if defined(__OBJC__)
//		static	NSCalendarDate*	calendarDateForGregorianDate(const SGregorianDate& date,
//										NSTimeZone* timeZone)
//									{ return [NSCalendarDate dateWithYear:date.mYear
//											month:date.mMonth day:date.mDay hour:date.mHour
//											minute:date.mMinute second:(NSUInteger) date.mSecond
//											timeZone:timeZone]; }
#endif
#endif

								// Deprecated
//								CTimeInfo(const CDictionaryX& info);
//				CDictionaryX		getInfoX() const;

	// Properties
	public:
		static	const	CTimeInfo			mOneSecond;
		static	const	CTimeInfo			mOneMinute;
		static	const	CTimeInfo			mOneHour;
		static	const	CTimeInfo			mZero;
	
	private:
						CTimeInfoInternals*	mInternals;
};
