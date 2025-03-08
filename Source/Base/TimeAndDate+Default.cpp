//----------------------------------------------------------------------------------------------------------------------
//	TimeAndDate+Default.cpp			©2024 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGregorianDate

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getDayOfWeekString(bool abbrieviated) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check day of week
	switch (mDayOfWeek) {
		case 0:		return CString(abbrieviated ? OSSTR("Sun") : OSSTR("Sunday"));
		case 1:		return CString(abbrieviated ? OSSTR("Mon") : OSSTR("Monday"));
		case 2:		return CString(abbrieviated ? OSSTR("Tue") : OSSTR("Tuesday"));
		case 3:		return CString(abbrieviated ? OSSTR("Wed") : OSSTR("Wednesday"));
		case 4:		return CString(abbrieviated ? OSSTR("Thu") : OSSTR("Thursday"));
		case 5:		return CString(abbrieviated ? OSSTR("Fri") : OSSTR("Friday"));
		case 6:		return CString(abbrieviated ? OSSTR("Sat") : OSSTR("Saturday"));
		default:	return CString::mEmpty;
	}
}

// Class methods

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getMonthString(UInt8 month, bool abbrieviated)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check month
	switch (month) {
		case 1:		return CString(abbrieviated ? OSSTR("Jan") : OSSTR("January"));
		case 2:		return CString(abbrieviated ? OSSTR("Feb") : OSSTR("February"));
		case 3:		return CString(abbrieviated ? OSSTR("Mar") : OSSTR("March"));
		case 4:		return CString(abbrieviated ? OSSTR("Apr") : OSSTR("April"));
		case 5:		return CString(abbrieviated ? OSSTR("May") : OSSTR("May"));
		case 6:		return CString(abbrieviated ? OSSTR("Jun") : OSSTR("June"));
		case 7:		return CString(abbrieviated ? OSSTR("Jul") : OSSTR("July"));
		case 8:		return CString(abbrieviated ? OSSTR("Aug") : OSSTR("August"));
		case 9:		return CString(abbrieviated ? OSSTR("Sep") : OSSTR("September"));
		case 10:	return CString(abbrieviated ? OSSTR("Oct") : OSSTR("October"));
		case 11:	return CString(abbrieviated ? OSSTR("Nov") : OSSTR("November"));
		case 12:	return CString(abbrieviated ? OSSTR("Dec") : OSSTR("December"));
		default:	return CString::mEmpty;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getAMString()
//----------------------------------------------------------------------------------------------------------------------
{
	return CString(OSSTR("am"));
}

//----------------------------------------------------------------------------------------------------------------------
CString SGregorianDate::getPMString()
//----------------------------------------------------------------------------------------------------------------------
{
	return CString(OSSTR("pm"));
}
