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
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianDate

// MARK: Properties

const	CString		SGregorianDate::mJanString(OSSTR("Jan"));
const	CString		SGregorianDate::mFebString(OSSTR("Feb"));
const	CString		SGregorianDate::mMarString(OSSTR("Mar"));
const	CString		SGregorianDate::mAprString(OSSTR("Apr"));
const	CString		SGregorianDate::mMayString(OSSTR("May"));
const	CString		SGregorianDate::mJunString(OSSTR("Jun"));
const	CString		SGregorianDate::mJulString(OSSTR("Jul"));
const	CString		SGregorianDate::mAugString(OSSTR("Aug"));
const	CString		SGregorianDate::mSepString(OSSTR("Sep"));
const	CString		SGregorianDate::mOctString(OSSTR("Oct"));
const	CString		SGregorianDate::mNovString(OSSTR("Nov"));
const	CString		SGregorianDate::mDecString(OSSTR("Dec"));

const	CString		SGregorianDate::mJanuaryString(OSSTR("January"));
const	CString		SGregorianDate::mFebruaryString(OSSTR("February"));
const	CString		SGregorianDate::mMarchString(OSSTR("March"));
const	CString		SGregorianDate::mAprilString(OSSTR("April"));
const	CString		SGregorianDate::mJuneString(OSSTR("June"));
const	CString		SGregorianDate::mJulyString(OSSTR("July"));
const	CString		SGregorianDate::mAugustString(OSSTR("August"));
const	CString		SGregorianDate::mSeptemberString(OSSTR("September"));
const	CString		SGregorianDate::mOctoberString(OSSTR("October"));
const	CString		SGregorianDate::mNovemberString(OSSTR("November"));
const	CString		SGregorianDate::mDecemberString(OSSTR("December"));

const	CString		SGregorianDate::mSunString(OSSTR("Sunday"));
const	CString		SGregorianDate::mMonString(OSSTR("Monday"));
const	CString		SGregorianDate::mTueString(OSSTR("Tuesday"));
const	CString		SGregorianDate::mWedString(OSSTR("Wednesday"));
const	CString		SGregorianDate::mThuString(OSSTR("Thursday"));
const	CString		SGregorianDate::mFriString(OSSTR("Friday"));
const	CString		SGregorianDate::mSatString(OSSTR("Saturday"));

const	CString		SGregorianDate::mAMString(OSSTR("am"));
const	CString		SGregorianDate::mPMString(OSSTR("pm"));

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getString(ComponentStyle dateComponentStyle, ComponentStyle timeComponentStyle) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose string
	const	CString*	day;
	const	CString*	month;
			CString		string;
			UInt8		hour = (mHour == 0) ? 12 : ((mHour - 1) % 12 + 1);

	switch (dateComponentStyle) {
		case kComponentStyleNone:
			// None
			break;

		case kComponentStyleShort:
			// Short - 1/1/1952
			string = CString(mMonth) + CString(OSSTR("/")) + CString(mDay) + CString(OSSTR("/")) + CString(mYear);
			break;

		case kComponentStyleMedium:
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

		case kComponentStyleLong:
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

		case kComponentStyleFull:
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

	switch (timeComponentStyle) {
		case kComponentStyleNone:
			// None
			break;

		case kComponentStyleShort:
			// Short - 3:30pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kComponentStyleMedium:
			// Medium - 3:30pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kComponentStyleLong:
			// Long - 3:30:32pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) + CString(OSSTR(":")) +
							CString((UInt16) mSecond, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kComponentStyleFull:
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
CString SGregorianDate::getString(StringStyle stringStyle) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check style
	switch (stringStyle) {
		case kStringStyleRFC339Extended: {
			// RFC339 extended
			// "yyyy-MM-dd'T'HH:mm:ss.SSSZ"
			//	2021-03-23T22:54:13.922-0700
			SInt32			offset = (SInt32) (SGregorianDate::getCurrentTimeZoneOffset() / 60.0);
			char			offsetSign = (offset > 0) ? '+' : '-';
			SInt32			offsetAbsolute = abs(offset);

			return CString::make(OSSTR("%4u-%02u-%02uT%02u:%02u:%06.3f%c%02u%02u"), mYear, mMonth, mDay, mHour, mMinute,
					mSecond, offsetSign, offsetAbsolute / 60, offsetAbsolute % 60);
		}

#if defined(TARGET_OS_WINDOWS)
		// Unnessary, but making the compiler happy
		default:	return CString::mEmpty;
#endif
	}
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<SGregorianDate> SGregorianDate::getFrom(const CString& string, StringStyle stringStyle)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check style
	switch (stringStyle) {
		case kStringStyleRFC339Extended: {
			// RFC339 extended
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
			if ((buffer[4] != '-') || (buffer[7] != '-') || (buffer[10] != 'T') || (buffer[13] != ':') ||
					(buffer[16] != ':'))
				// Required characters not in their respective positions
				return OV<SGregorianDate>();

			// Check for timezone offset sign
			OV<SRange32>	timezoneOffsetMinusRange = string.findSubString(CString(OSSTR("-")), 17);
			OV<SRange32>	timezoneOffsetPlusRange = string.findSubString(CString(OSSTR("+")), 17);
			if (!timezoneOffsetMinusRange.hasValue() && !timezoneOffsetPlusRange.hasValue())
				// Did not find timezone offset sign
				return OV<SGregorianDate>();

			CString::CharIndex	timezoneOffsetSignCharIndex =
										timezoneOffsetMinusRange.hasValue() ?
												timezoneOffsetMinusRange->getStart() :
												timezoneOffsetPlusRange->getStart();

			// Compose gregorian date
			SGregorianDate	gregorianDate;
			gregorianDate.mYear = string.getSubString(0, 4).getUInt32();
			gregorianDate.mMonth = string.getSubString(5, 2).getUInt8();
			gregorianDate.mDay = string.getSubString(8, 2).getUInt8();
			gregorianDate.mHour = string.getSubString(11, 2).getUInt8();
			gregorianDate.mMinute = string.getSubString(14, 2).getUInt8();
			gregorianDate.mSecond = string.getSubString(17, timezoneOffsetSignCharIndex - 17).getFloat32();

			// Compose timezone offset
			SInt32					timezoneOffsetRaw =
											string.getSubString(timezoneOffsetSignCharIndex + 1).getSInt32();
			SInt32					timezoneOffsetHours = timezoneOffsetRaw / 100;
			SInt32					timezoneOffsetMinutes = timezoneOffsetRaw % 100;
			UniversalTimeInterval	timezoneOffset =
											timezoneOffsetMinusRange.hasValue() ?
													(UniversalTimeInterval)
															(-timezoneOffsetHours * 60 * 60 -
																	timezoneOffsetMinutes * 60) :
													(UniversalTimeInterval)
															(timezoneOffsetHours * 60 * 60 +
																	timezoneOffsetMinutes * 60);
			return OV<SGregorianDate>(
					SGregorianDate(gregorianDate.getUniversalTime() - timezoneOffset + getCurrentTimeZoneOffset()));
		}

#if defined(TARGET_OS_WINDOWS)
		// Unnessary, but making the compiler happy
		default:	return OV<SGregorianDate>();
#endif
	}
}
