//----------------------------------------------------------------------------------------------------------------------
//	CPreferences.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

SPref			CPreferences::mNoPref(nil);
SStringPref		CPreferences::mNoStringPref(nil, OSSTR(""));
SFloat32Pref	CPreferences::mNoFloat32Pref(nil, 0.0);
SFloat64Pref	CPreferences::mNoFloat64Pref(nil, 0.0);
SUInt32Pref		CPreferences::mNoUInt32Pref(nil, 0);
