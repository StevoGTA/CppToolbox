//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDate.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGregorianDate

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
CString SGregorianDate::getString(EGregorianDateStringStyle dateStyle, EGregorianDateStringStyle timeStyle) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose string
	CString*	day;
	CString*	month;
	CString		string;
	UInt8		hour = (mHour == 0) ? 12 : ((mHour - 1) % 12 + 1);

	switch (dateStyle) {
		case kGregorianDateStringStyleNone:
			// None
			break;

		case kGregorianDateStringStyleShort:
			// Short - 1/1/1952
			string = CString(mMonth) + CString(OSSTR("/")) + CString(mDay) + CString(OSSTR("/")) + CString(mYear);
			break;

		case kGregorianDateStringStyleMedium:
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

		case kGregorianDateStringStyleLong:
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

		case kGregorianDateStringStyleFull:
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

	switch (timeStyle) {
		case kGregorianDateStringStyleNone:
			// None
			break;

		case kGregorianDateStringStyleShort:
			// Short - 3:30pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kGregorianDateStringStyleMedium:
			// Medium - 3:30pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kGregorianDateStringStyleLong:
			// Long - 3:30:32pm
			if (!string.isEmpty())
				string += CString(OSSTR(" "));
			string +=
					CString(hour) + CString(OSSTR(":")) + CString(mMinute, 2, true) + CString(OSSTR(":")) +
							CString((UInt16) mSecond, 2, true) +
							((mHour >= 12) ? SGregorianDate::mPMString : SGregorianDate::mAMString);
			break;

		case kGregorianDateStringStyleFull:
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
SGregorianDate SGregorianDate::operator+(const SGregorianUnits& units) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Adding or subtracting too much can cause the raw values to go out of range.  We transform to UniverstalTime and
	//	back to normalize back to valid values.
	SGregorianDate	newDate(mYear + units.mYears, mMonth + units.mMonths, mDay + units.mDays, mHour + units.mHours,
							mMinute + units.mMinutes, mSecond + units.mSeconds);

	return SGregorianDate(newDate.getUniversalTime());
}

//----------------------------------------------------------------------------------------------------------------------
SGregorianDate& SGregorianDate::operator+=(const SGregorianUnits& units)
//----------------------------------------------------------------------------------------------------------------------
{
	// Adding or subtracting too much can cause the raw values to go out of range.  We transform to UniverstalTime and
	//	back to normalize back to valid values.
	SGregorianDate	newDate(mYear + units.mYears, mMonth + units.mMonths, mDay + units.mDays, mHour + units.mHours,
							mMinute + units.mMinutes, mSecond + units.mSeconds);
	*this = SGregorianDate(newDate.getUniversalTime());

	return *this;
}
