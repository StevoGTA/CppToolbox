//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDate.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "TimeAndDate.h"

#include "CDictionary.h"
#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGregorianDate::Components

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::Components::getString(DateStyle dateStyle, TimeStyle timeStyle) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose string
	CString	string;
	switch (dateStyle) {
		case kDateStyleNone:
			// None
			break;

		case kDateStyleMDYShort:
			// 1/1/1952
			if (mYear.hasValue() && mMonth.hasValue() && mDay.hasValue())
				// 1/1/1952
				string = CString(*mMonth) + CString::mSlash + CString(*mDay) + CString::mSlash + CString(*mYear);
			else if (mYear.hasValue() && mMonth.hasValue())
				// 1/1952
				string = CString(*mMonth) + CString::mSlash + CString(*mYear);
			else if (mYear.hasValue())
				// 1952
				string = CString(*mYear);
			else if (mMonth.hasValue() && mDay.hasValue())
				// 1/1
				string = CString(*mMonth) + CString::mSlash + CString(*mDay);
			break;

		case kDateStyleMDYMedium:
			// Jan 12, 1952
			if (mYear.hasValue() && mMonth.hasValue() && mDay.hasValue())
				// Jan 12, 1952
				string = SGregorianDate::getMonthString(*mMonth, true) + CString::mSpace + CString(*mDay) +
						CString(OSSTR(", ")) + CString(*mYear);
			else if (mYear.hasValue() && mMonth.hasValue())
				// Jan 1952
				string = SGregorianDate::getMonthString(*mMonth, true) + CString::mSpace + CString(*mYear);
			else if (mYear.hasValue())
				// 1952
				string = CString(*mYear);
			else if (mMonth.hasValue() && mDay.hasValue())
				// Jan 12
				string = SGregorianDate::getMonthString(*mMonth, true) + CString::mSlash + CString(*mDay);
			else if (mMonth.hasValue())
				// Jan
				string = SGregorianDate::getMonthString(*mMonth, true);
			break;

		case kDateStyleMDYLong:
			// January 12, 1952
			if (mYear.hasValue() && mMonth.hasValue() && mDay.hasValue())
				// Jan 12, 1952
				string = SGregorianDate::getMonthString(*mMonth) + CString::mSpace + CString(*mDay) +
						CString(OSSTR(", ")) + CString(*mYear);
			else if (mYear.hasValue() && mMonth.hasValue())
				// Jan 1952
				string = SGregorianDate::getMonthString(*mMonth) + CString::mSpace + CString(*mYear);
			else if (mYear.hasValue())
				// 1952
				string = CString(*mYear);
			else if (mMonth.hasValue() && mDay.hasValue())
				// Jan 12
				string = SGregorianDate::getMonthString(*mMonth) + CString::mSlash + CString(*mDay);
			else if (mMonth.hasValue())
				// Jan
				string = SGregorianDate::getMonthString(*mMonth);
			break;

		case kDateStyleY:
			// 1952
			if (mYear.hasValue())
				string = CString(*mYear);
			break;

		case kDateStyleMDShort:
			// 1/1
			if (mMonth.hasValue() && mDay.hasValue())
				string = CString(*mMonth) + CString::mSlash + CString(*mDay);;
			break;

		case kDateStyleMDMedium:
			// Jan 12
			if (mMonth.hasValue() && mDay.hasValue())
				string = SGregorianDate::getMonthString(*mMonth, true) + CString::mSpace + CString(*mDay);
			else if (mMonth.hasValue())
				string = SGregorianDate::getMonthString(*mMonth, true);
			break;

		case kDateStyleMDLong:
			// January 12
			if (mMonth.hasValue() && mDay.hasValue())
				string =SGregorianDate:: getMonthString(*mMonth) + CString::mSpace + CString(*mDay);
			else if (mMonth.hasValue())
				string = SGregorianDate::getMonthString(*mMonth);
			break;
	}

	switch (timeStyle) {
		case kTimeStyleNone:
			// None
			break;

		case kTimeStyleHMAMPM:
			// 3:30pm
			if (mHour.hasValue() && mMinute.hasValue()) {
				// 3:30pm
				if (!string.isEmpty())
					// Have date
					string += CString::mSpace;

				UInt8	hour = (*mHour == 0) ? 12 : ((*mHour - 1) % 12 + 1);
				string +=
						CString(hour) + CString::mColon + CString(*mMinute, 2, true) +
								((*mHour >= 12) ? getPMString() : getAMString());
			}
			break;

		case kTimeStyleHM24:
			// 15:30
			if (mHour.hasValue() && mMinute.hasValue()) {
				// 15:30
				if (!string.isEmpty())
					// Have date
					string += CString::mSpace;

				string += CString(*mHour, 2, true) + CString::mColon + CString(*mMinute, 2, true);
			}
			break;

		case kTimeStyleHMSAMPM:
			// 3:30:32pm
			if (mHour.hasValue() && mMinute.hasValue() && mSecond.hasValue()) {
				// 3:30:32pm
				if (!string.isEmpty())
					// Have date
					string += CString::mSpace;

				UInt8	hour = (*mHour == 0) ? 12 : ((*mHour - 1) % 12 + 1);
				string +=
						CString(hour) + CString::mColon + CString(*mMinute, 2, true) + CString::mColon +
								CString((UInt16) *mSecond, 2, true) + ((*mHour >= 12) ? getPMString() : getAMString());
			} else if (mHour.hasValue() && mMinute.hasValue()) {
				// 3:30pm
				if (!string.isEmpty())
					// Have date
					string += CString::mSpace;

				UInt8	hour = (*mHour == 0) ? 12 : ((*mHour - 1) % 12 + 1);
				string +=
						CString(hour) + CString::mColon + CString(*mMinute, 2, true) +
								((*mHour >= 12) ? getPMString() : getAMString());
			}
			break;

		case kTimeStyleHMS24:
			// 15:30:32
			if (mHour.hasValue() && mMinute.hasValue() && mSecond.hasValue()) {
				// 15:30:32
				if (!string.isEmpty())
					// Have date
					string += CString::mSpace;

				string +=
						CString(*mHour, 2, true) + CString::mColon + CString(*mMinute, 2, true) + CString::mColon +
								CString((UInt16) *mSecond, 2, true);
			} else if (mHour.hasValue() && mMinute.hasValue()) {
				// 15:30
				if (!string.isEmpty())
					// Have date
					string += CString::mSpace;

				string += CString(*mHour, 2, true) + CString::mColon + CString(*mMinute, 2, true);
			}
			break;
	}

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary SGregorianDate::Components::getInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	info;

	if (mYear.hasValue())
		info.set(CString(OSSTR("year")), *mYear);
	if (mMonth.hasValue())
		info.set(CString(OSSTR("month")), *mMonth);
	if (mDay.hasValue())
		info.set(CString(OSSTR("day")), *mDay);
	if (mHour.hasValue())
		info.set(CString(OSSTR("hour")), *mHour);
	if (mMinute.hasValue())
		info.set(CString(OSSTR("minute")), *mMinute);
	if (mSecond.hasValue())
		info.set(CString(OSSTR("second")), *mSecond);

	return info;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SGregorianDate::Components> SGregorianDate::Components::getFrom(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OV<UInt32>	year = info.getOVUInt32(CString(OSSTR("year")));
	OV<UInt8>	month = info.getOVUInt8(CString(OSSTR("month")));
	OV<UInt8>	day = info.getOVUInt8(CString(OSSTR("day")));
	OV<UInt8>	hour = info.getOVUInt8(CString(OSSTR("hour")));
	OV<UInt8>	minute = info.getOVUInt8(CString(OSSTR("minute")));
	OV<Float32>	second = info.getOVFloat32(CString(OSSTR("second")));

	return (year.hasValue() || month.hasValue() || day.hasValue() || hour.hasValue() || minute.hasValue() ||
					second.hasValue()) ?
			OV<Components>(Components(year, month, day, hour, minute, second)) : OV<Components>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SGregorianDate::Components> SGregorianDate::Components::getFrom(const CString& string, StringStyle stringStyle)
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

			// Check for required characters
			TBuffer<char>	buffer = string.getUTF8Chars();
			if ((buffer[4] != '-') || (buffer[7] != '-') || (buffer[10] != 'T') || (buffer[13] != ':') ||
					(buffer[16] != ':'))
				// Required characters not in their respective positions
				return OV<Components>();

			// Check for timezone offset sign
			OV<SRange32>	timezoneOffsetMinusRange = string.findSubString(CString(OSSTR("-")), 17);
			OV<SRange32>	timezoneOffsetPlusRange = string.findSubString(CString(OSSTR("+")), 17);
			if (!timezoneOffsetMinusRange.hasValue() && !timezoneOffsetPlusRange.hasValue())
				// Did not find timezone offset sign
				return OV<Components>();

			CString::CharIndex	timezoneOffsetSignCharIndex =
										timezoneOffsetMinusRange.hasValue() ?
												timezoneOffsetMinusRange->getStart() :
												timezoneOffsetPlusRange->getStart();

			return OV<Components>(
					Components(string.getSubString(0, 4).getUInt32(), string.getSubString(5, 2).getUInt8(),
							string.getSubString(8, 2).getUInt8(), string.getSubString(11, 2).getUInt8(),
							string.getSubString(14, 2).getUInt8(),
							string.getSubString(17, timezoneOffsetSignCharIndex - 17).getFloat32()));
		}

		case kStringStyleYYYY_MM_DDTHH_MM_SS:
			// yyyy-MM-ddTHH:mm:ss
			if (string.getLength() != 19)
				// Must be 19 characters
				return OV<Components>();

			return OV<Components>(
					Components(string.getSubString(0, 4).getUInt32(), string.getSubString(5, 2).getUInt8(),
							string.getSubString(8, 2).getUInt8(), string.getSubString(11, 2).getUInt8(),
							string.getSubString(14, 2).getUInt8(), string.getSubString(17, 2).getFloat32()));

		case kStringStyleYYYY_MM_DDTHH_MM:
			// yyyy-MM-ddTHH:mm
			if (string.getLength() != 16)
				// Must be 16 characters
				return OV<Components>();

			return OV<Components>(
					Components(string.getSubString(0, 4).getUInt32(), string.getSubString(5, 2).getUInt8(),
							string.getSubString(8, 2).getUInt8(), string.getSubString(11, 2).getUInt8(),
							string.getSubString(14, 2).getUInt8()));

		case kStringStyleYYYY_MM_DDTHH:
			// yyyy-MM-ddTHH
			if (string.getLength() != 13)
				// Must be 13 characters
				return OV<Components>();

			return OV<Components>(
					Components(string.getSubString(0, 4).getUInt32(), string.getSubString(5, 2).getUInt8(),
							string.getSubString(8, 2).getUInt8(), string.getSubString(11, 2).getUInt8()));

		case kStringStyleYYYY_MM_DD:
			// yyyy-MM-dd
			if (string.getLength() != 10)
				// Must be 10 characters
				return OV<Components>();

			return OV<Components>(
					Components(string.getSubString(0, 4).getUInt32(), string.getSubString(5, 2).getUInt8(),
							string.getSubString(8, 2).getUInt8()));

		case kStringStyleYYYYMMDD:
			// yyyyMMdd
			if (string.getLength() != 8)
				// Must be 8 characters
				return OV<Components>();

			return OV<Components>(
					Components(string.getSubString(0, 4).getUInt32(), string.getSubString(4, 2).getUInt8(),
							string.getSubString(6, 2).getUInt8()));

		case kStringStyleYYYY_MM:
			// yyyy-MM
			if (string.getLength() != 7)
				// Must be 7 characters
				return OV<Components>();

			return OV<Components>(
					Components(string.getSubString(0, 4).getUInt32(), string.getSubString(5, 2).getUInt8(),
							string.getSubString(8, 2).getUInt8()));

		case kStringStyleYYYY:
			// yyyy
			if (string.getLength() != 4)
				// Must be 4 characters
				return OV<Components>();

			return OV<Components>(Components(string.getUInt32()));

		case kStringStyleMMDD:
			// MM/dd
			if (string.getLength() != 5)
				// Must be 5 characters
				return OV<Components>();

			return OV<Components>(
					Components(OV<UInt32>(), OV<UInt8>(string.getSubString(0, 2).getUInt8()),
							OV<UInt8>(string.getSubString(3).getUInt8())));

		case kStringStyleDDMM:
			// ddMM
			if (string.getLength() != 4)
				// Must be 4 characters
				return OV<Components>();

			return OV<Components>(
					Components(OV<UInt32>(), OV<UInt8>(string.getSubString(2).getUInt8()),
							OV<UInt8>(string.getSubString(0, 2).getUInt8())));

		case kStringStyleHH_MM_SS:
			// HH:mm:ss
			if (string.getLength() != 8)
				// Must be 8 characters
				return OV<Components>();

			return OV<Components>(
					Components(OV<UInt32>(), OV<UInt8>(), OV<UInt8>(), OV<UInt8>(string.getSubString(0, 2).getUInt8()),
							OV<UInt8>(string.getSubString(3, 2).getUInt8()),
							OV<Float32>(string.getSubString(6, 2).getFloat32())));

		case kStringStyleHH_MM:
			// HH:mm
			if (string.getLength() != 5)
				// Must be 5 characters
				return OV<Components>();

			return OV<Components>(
					Components(OV<UInt32>(), OV<UInt8>(), OV<UInt8>(), OV<UInt8>(string.getSubString(0, 2).getUInt8()),
							OV<UInt8>(string.getSubString(3).getUInt8())));

		case kStringStyleHHMM:
			// HHmm
			if (string.getLength() != 4)
				// Must be 4 characters
				return OV<Components>();

			return OV<Components>(
					Components(OV<UInt32>(), OV<UInt8>(), OV<UInt8>(), OV<UInt8>(string.getSubString(0, 2).getUInt8()),
							OV<UInt8>(string.getSubString(2).getUInt8())));

#if defined(TARGET_OS_WINDOWS)
		// Unnessary, but making the compiler happy
		default:	return OV<Components>();
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGregorianDate

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getHourString(bool use24HourFormat) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if using 24 hour format
	if (use24HourFormat)
		// 24 hour format
		return CString(mHour, 2, true) + CString(OSSTR(":00"));
	else {
		// Setup
		UInt8	hour = (mHour == 0) ? 12 : ((mHour - 1) % 12 + 1);

		return CString(hour) + CString(OSSTR(":00")) + ((mHour >= 12) ? getPMString() : getAMString());
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getString(DateStyle dateStyle, TimeStyle timeStyle) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose string
	CString	string;
	UInt8	hour = (mHour == 0) ? 12 : ((mHour - 1) % 12 + 1);

	switch (dateStyle) {
		case kDateStyleNone:
			// None
			break;

		case kDateStyleMDYShort:
			// 1/1/1952
			string = CString(mMonth) + CString::mSlash + CString(mDay) + CString::mSlash + CString(mYear);
			break;

		case kDateStyleMDYMedium:
			// Jan 12, 1952
			string = getMonthString(true) + CString::mSpace + CString(mDay) + CString(OSSTR(", ")) + CString(mYear);
			break;

		case kDateStyleMDYLong:
			// January 12, 1952
			string = getMonthString() + CString::mSpace + CString(mDay) + CString(OSSTR(", ")) + CString(mYear);
			break;

		case kDateStyleY:
			// 1952
			string = CString(mYear);
			break;

		case kDateStyleMDShort:
			// 1/1
			string = CString(mMonth) + CString::mSlash + CString(mDay);;
			break;

		case kDateStyleMDMedium:
			// Jan 12
			string = getMonthString(true) + CString::mSpace + CString(mDay);
			break;

		case kDateStyleMDLong:
			// January 12
			string = getMonthString() + CString::mSpace + CString(mDay);
			break;
	}

	switch (timeStyle) {
		case kTimeStyleNone:
			// None
			break;

		case kTimeStyleHMAMPM:
			// 3:30pm
			if (!string.isEmpty())
				string += CString::mSpace;
			string +=
					CString(hour) + CString::mColon + CString(mMinute, 2, true) +
							((mHour >= 12) ? getPMString() : getAMString());
			break;

		case kTimeStyleHM24:
			// 15:30pm
			if (!string.isEmpty())
				string += CString::mSpace;
			string += CString(mHour, 2, true) + CString::mColon + CString(mMinute, 2, true);
			break;

		case kTimeStyleHMSAMPM:
			// 3:30:32pm
			if (!string.isEmpty())
				string += CString::mSpace;
			string +=
					CString(hour) + CString::mColon + CString(mMinute, 2, true) + CString::mColon +
							CString((UInt16) mSecond, 2, true) + ((mHour >= 12) ? getPMString() : getAMString());
			break;

		case kTimeStyleHMS24:
			// 15:30:32
			if (!string.isEmpty())
				string += CString::mSpace;
			string +=
					CString(mHour, 2, true) + CString::mColon + CString(mMinute, 2, true) + CString::mColon +
							CString((UInt16) mSecond, 2, true);
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

			return CString::make(OSSTR("%04u-%02u-%02uT%02u:%02u:%06.3f%c%02u%02u"), mYear, mMonth, mDay, mHour,
					mMinute, mSecond, offsetSign, offsetAbsolute / 60, offsetAbsolute % 60);
		}

		case kStringStyleYYYY_MM_DDTHH_MM_SS:
			// yyyy-MM-ddTHH:mm:ss
			return CString::make(OSSTR("%04u-%02u-%02T$02u:%02u:%02.0f"), mYear, mMonth, mDay, mHour, mMinute, mSecond);

		case kStringStyleYYYY_MM_DDTHH_MM:
			// yyyy-MM-ddTHH:mm
			return CString::make(OSSTR("%04u-%02u-%02T$02u:%02u"), mYear, mMonth, mDay, mHour, mMinute);

		case kStringStyleYYYY_MM_DDTHH:
			// yyyy-MM-ddTHH
			return CString::make(OSSTR("%04u-%02u-%02T$02u"), mYear, mMonth, mDay, mHour);

		case kStringStyleYYYY_MM_DD:
			// yyyy-MM-dd
			return CString::make(OSSTR("%04u-%02u-%02"), mYear, mMonth, mDay);

		case kStringStyleYYYYMMDD:
			// yyyyMMdd
			return CString::make(OSSTR("%04u%02u%02"), mYear, mMonth, mDay);

		case kStringStyleYYYY_MM:
			// yyyy-MM
			return CString::make(OSSTR("%04u-%02u"), mYear, mMonth);

		case kStringStyleYYYY:
			// yyyy
			return CString::make(OSSTR("%04u"), mYear);

		case kStringStyleMMDD:
			// MMdd
			return CString::make(OSSTR("%02u%02u"), mMonth, mDay);

		case kStringStyleDDMM:
			// ddMM
			return CString::make(OSSTR("%02u%02u"), mDay, mMonth);

		case kStringStyleHH_MM_SS:
			// HH:mm:ss
			return CString::make(OSSTR("%02u:%02u:%02f"), mHour, mMinute, mSecond);

		case kStringStyleHH_MM:
			// HH:mm
			return CString::make(OSSTR("%02u:%02u"), mHour, mMinute);

		case kStringStyleHHMM:
			// HHmm
			return CString::make(OSSTR("%02u%02u"), mHour, mMinute);

#if defined(TARGET_OS_WINDOWS)
		// Unnessary, but making the compiler happy
		default:	return CString::mEmpty;
#endif
	}
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UInt8 SGregorianDate::getMaxDays(UInt8 month)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check month
	switch (month) {
		case 1:		return 31;
		case 2:		return 29;
		case 3:		return 31;
		case 4:		return 30;
		case 5:		return 31;
		case 6:		return 30;
		case 7:		return 31;
		case 8:		return 31;
		case 9:		return 30;
		case 10:	return 31;
		case 11:	return 30;
		case 12:	return 31;
		default:	return 0;
	}
}

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

			// Check for required characters
			TBuffer<char>	buffer = string.getUTF8Chars();
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

		default: {
			// Get components
			OV<Components>	components = Components::getFrom(string, stringStyle);

			return components.hasValue() ? OV<SGregorianDate>(SGregorianDate(*components)) : OV<SGregorianDate>();
		}
	}
}
