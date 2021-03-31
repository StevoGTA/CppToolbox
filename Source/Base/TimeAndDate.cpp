//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDate.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "TimeAndDate.h"

#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SUniversalTime

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SUniversalTime::getDistantFuture()
//----------------------------------------------------------------------------------------------------------------------
{
	return 64092211200.0;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime SUniversalTime::getDistantPast()
//----------------------------------------------------------------------------------------------------------------------
{
	return -62135769600.0;
}

//----------------------------------------------------------------------------------------------------------------------
CString SUniversalTime::getRFC339Extended(UniversalTime universalTime)
//----------------------------------------------------------------------------------------------------------------------
{
	// "yyyy-MM-dd'T'HH:mm:ss.SSSZ"
	//	2021-03-23T22:54:13.922-0700
	SGregorianDate	gregorianDate(universalTime);
	SInt32			offset = (SInt32) (SUniversalTime::getCurrentTimeZoneOffset() / 60.0);
	char			offsetSign = (offset > 0) ? '+' : '-';
	SInt32			offsetAbsolute = abs(offset);

	return CString::make("%4u-%02u-%02uT%02u:%02u:%06.3f%c%02u%02u", gregorianDate.mYear, gregorianDate.mMonth,
			gregorianDate.mDay, gregorianDate.mHour, gregorianDate.mMinute, gregorianDate.mSecond,
			offsetSign, offsetAbsolute / 60, offsetAbsolute % 60);
}

//----------------------------------------------------------------------------------------------------------------------
OV<UniversalTime> SUniversalTime::getFromRFC3339Extended(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
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

	// Setup
	CString::Length	length = string.getLength();
	TBuffer<char>	buffer(length + 1);
	string.get(*buffer, length + 1);

	// Check for required characters
	if ((buffer[4] != '-') || (buffer[7] != '-') || (buffer[10] != 'T') || (buffer[13] != ':') || (buffer[16] != ':'))
		// Required characters not in their respective positions
		return OV<UniversalTime>();

	// Check for timezone offset sign
	CString::Range	timezoneOffsetMinusRange = string.findSubString(CString(OSSTR("-")), 17);
	CString::Range	timezoneOffsetPlusRange = string.findSubString(CString(OSSTR("+")), 17);
	if (!timezoneOffsetMinusRange.isValid() && !timezoneOffsetPlusRange.isValid())
		// Did not find timezone offset sign
		return OV<UniversalTime>();

	CString::CharIndex	timezoneOffsetSignCharIndex =
								timezoneOffsetMinusRange.isValid() ?
										timezoneOffsetMinusRange.mStart : timezoneOffsetPlusRange.mStart;

	// Compose gregorian date
	SGregorianDate	gregorianDate;
	gregorianDate.mYear = string.getSubString(0, 4).getUInt32();
	gregorianDate.mMonth = string.getSubString(5, 2).getUInt8();
	gregorianDate.mDay = string.getSubString(8, 2).getUInt8();
	gregorianDate.mHour = string.getSubString(11, 2).getUInt8();
	gregorianDate.mMinute = string.getSubString(14, 2).getUInt8();
	gregorianDate.mSecond = string.getSubString(17, timezoneOffsetSignCharIndex - 17).getFloat32();

	// Compose timezone offset
	SInt32					timezoneOffsetRaw = string.getSubString(timezoneOffsetSignCharIndex + 1).getSInt32();
	SInt32					timezoneOffsetHours = timezoneOffsetRaw / 100;
	SInt32					timezoneOffsetMinutes = timezoneOffsetRaw % 100;
	UniversalTimeInterval	timezoneOffset =
									timezoneOffsetMinusRange.isValid() ?
											(UniversalTimeInterval)
													(-timezoneOffsetHours * 60 * 60 - timezoneOffsetMinutes * 60) :
											(UniversalTimeInterval)
													(timezoneOffsetHours * 60 * 60 + timezoneOffsetMinutes * 60);

	return gregorianDate.getUniversalTime() - timezoneOffset + SUniversalTime::getCurrentTimeZoneOffset();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianDate

// MARK: Properties

CString		SGregorianDate::mJanString(OSSTR("Jan"));
CString		SGregorianDate::mFebString(OSSTR("Feb"));
CString		SGregorianDate::mMarString(OSSTR("Mar"));
CString		SGregorianDate::mAprString(OSSTR("Apr"));
CString		SGregorianDate::mMayString(OSSTR("May"));
CString		SGregorianDate::mJunString(OSSTR("Jun"));
CString		SGregorianDate::mJulString(OSSTR("Jul"));
CString		SGregorianDate::mAugString(OSSTR("Aug"));
CString		SGregorianDate::mSepString(OSSTR("Sep"));
CString		SGregorianDate::mOctString(OSSTR("Oct"));
CString		SGregorianDate::mNovString(OSSTR("Nov"));
CString		SGregorianDate::mDecString(OSSTR("Dec"));

CString		SGregorianDate::mJanuaryString(OSSTR("January"));
CString		SGregorianDate::mFebruaryString(OSSTR("February"));
CString		SGregorianDate::mMarchString(OSSTR("March"));
CString		SGregorianDate::mAprilString(OSSTR("April"));
CString		SGregorianDate::mJuneString(OSSTR("June"));
CString		SGregorianDate::mJulyString(OSSTR("July"));
CString		SGregorianDate::mAugustString(OSSTR("August"));
CString		SGregorianDate::mSeptemberString(OSSTR("September"));
CString		SGregorianDate::mOctoberString(OSSTR("October"));
CString		SGregorianDate::mNovemberString(OSSTR("November"));
CString		SGregorianDate::mDecemberString(OSSTR("December"));

CString		SGregorianDate::mSunString(OSSTR("Sunday"));
CString		SGregorianDate::mMonString(OSSTR("Monday"));
CString		SGregorianDate::mTueString(OSSTR("Tuesday"));
CString		SGregorianDate::mWedString(OSSTR("Wednesday"));
CString		SGregorianDate::mThuString(OSSTR("Thursday"));
CString		SGregorianDate::mFriString(OSSTR("Friday"));
CString		SGregorianDate::mSatString(OSSTR("Saturday"));

CString		SGregorianDate::mAMString(OSSTR("am"));
CString		SGregorianDate::mPMString(OSSTR("pm"));

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getString(StringStyle dateStringStyle, StringStyle timeStringStyle) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose string
	CString*	day;
	CString*	month;
	CString		string;
	UInt8		hour = (mHour == 0) ? 12 : ((mHour - 1) % 12 + 1);

	switch (dateStringStyle) {
		case kStringStyleNone:
			// None
			break;

		case kStringStyleShort:
			// Short - 1/1/1952
			string = CString(mMonth) + CString(OSSTR("/")) + CString(mDay) + CString(OSSTR("/")) + CString(mYear);
			break;

		case kStringStyleMedium:
			// Medium - Jan 12, 1952
			switch (mMonth) {
				case 1:		month = &SGregorianDate::mJanString;	break;
				case 2:		month = &SGregorianDate::mFebString;	break;
				case 3:		month = &SGregorianDate::mMarString;	break;
				case 4:		month = &SGregorianDate::mAprString;	break;
				case 5:		month = &SGregorianDate::mMayString;	break;
				case 6:		month = &SGregorianDate::mJunString;	break;
				case 7:		month = &SGregorianDate::mJulString;	break;
				case 8:		month = &SGregorianDate::mAugString;	break;
				case 9:		month = &SGregorianDate::mSepString;	break;
				case 10:	month = &SGregorianDate::mOctString;	break;
				case 11:	month = &SGregorianDate::mNovString;	break;
				case 12:	month = &SGregorianDate::mDecString;	break;
				default:	month = &CString::mEmpty;		break;
			}

			string = *month + CString(OSSTR(" ")) + CString(mDay) + CString(OSSTR(", ")) + CString(mYear);
			break;

		case kStringStyleLong:
			// Long - January 12, 1952
			switch (mMonth) {
				case 1:		month = &SGregorianDate::mJanuaryString;		break;
				case 2:		month = &SGregorianDate::mFebruaryString;	break;
				case 3:		month = &SGregorianDate::mMarchString;		break;
				case 4:		month = &SGregorianDate::mAprilString;		break;
				case 5:		month = &SGregorianDate::mMayString;			break;
				case 6:		month = &SGregorianDate::mJuneString;		break;
				case 7:		month = &SGregorianDate::mJulyString;		break;
				case 8:		month = &SGregorianDate::mAugustString;		break;
				case 9:		month = &SGregorianDate::mSeptemberString;	break;
				case 10:	month = &SGregorianDate::mOctoberString;		break;
				case 11:	month = &SGregorianDate::mNovemberString;	break;
				case 12:	month = &SGregorianDate::mDecemberString;	break;
				default:	month = &CString::mEmpty;				break;
			}

			string = *month + CString(OSSTR(" ")) + CString(mDay) + CString(OSSTR(", ")) + CString(mYear);
			break;

		case kStringStyleFull:
			// Full - Tuesday, April 12, 1952 AD
			switch (mDayOfWeek) {
				case 0:		day = &SGregorianDate::mSunString;	break;
				case 1:		day = &SGregorianDate::mMonString;	break;
				case 2:		day = &SGregorianDate::mTueString;	break;
				case 3:		day = &SGregorianDate::mWedString;	break;
				case 4:		day = &SGregorianDate::mThuString;	break;
				case 5:		day = &SGregorianDate::mFriString;	break;
				case 6:		day = &SGregorianDate::mSatString;	break;
				default:	day = &CString::mEmpty;			break;
			}

			switch (mMonth) {
				case 1:		month = &SGregorianDate::mJanuaryString;		break;
				case 2:		month = &SGregorianDate::mFebruaryString;	break;
				case 3:		month = &SGregorianDate::mMarchString;		break;
				case 4:		month = &SGregorianDate::mAprilString;		break;
				case 5:		month = &SGregorianDate::mMayString;			break;
				case 6:		month = &SGregorianDate::mJuneString;		break;
				case 7:		month = &SGregorianDate::mJulyString;		break;
				case 8:		month = &SGregorianDate::mAugustString;		break;
				case 9:		month = &SGregorianDate::mSeptemberString;	break;
				case 10:	month = &SGregorianDate::mOctoberString;		break;
				case 11:	month = &SGregorianDate::mNovemberString;	break;
				case 12:	month = &SGregorianDate::mDecemberString;	break;
				default:	month = &CString::mEmpty;				break;
			}

			string =
					*day + CString(OSSTR(", ")) + *month + CString(OSSTR(" ")) + CString(mDay) +
							CString(OSSTR(", ")) + CString(mYear);
			break;
	}

	switch (timeStringStyle) {
		case kStringStyleNone:
			// None
			break;

		case kStringStyleShort:
			// Short - 3:30pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kStringStyleMedium:
			// Medium - 3:30pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kStringStyleLong:
			// Long - 3:30:32pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) + CString(OSSTR(":")) +
							CString((UInt16) mSecond, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kStringStyleFull:
			// Full - 3:30:32pm PST
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) + CString(OSSTR(":")) +
							CString((UInt16) mSecond, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString) +
							CString(OSSTR(" GMT"));
			break;
	}

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
SGregorianDate SGregorianDate::operator+(const Units& units) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Adding or subtracting too much can cause the raw values to go out of range.  We transform to UniverstalTime and
	//	back to normalize back to valid values.
	SGregorianDate	newDate(mYear + units.mYears, mMonth + units.mMonths, mDay + units.mDays, mHour + units.mHours,
							mMinute + units.mMinutes, mSecond + units.mSeconds);

	return SGregorianDate(newDate.getUniversalTime());
}

//----------------------------------------------------------------------------------------------------------------------
SGregorianDate& SGregorianDate::operator+=(const Units& units)
//----------------------------------------------------------------------------------------------------------------------
{
	// Adding or subtracting too much can cause the raw values to go out of range.  We transform to UniverstalTime and
	//	back to normalize back to valid values.
	SGregorianDate	newDate(mYear + units.mYears, mMonth + units.mMonths, mDay + units.mDays, mHour + units.mHours,
							mMinute + units.mMinutes, mSecond + units.mSeconds);
	*this = SGregorianDate(newDate.getUniversalTime());

	return *this;
}
