//----------------------------------------------------------------------------------------------------------------------
//	CPreferences.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

CPreferences::Pref			CPreferences::mNoPref(nil);
CPreferences::StringPref	CPreferences::mNoStringPref(nil, OSSTR(""));
CPreferences::Float32Pref	CPreferences::mNoFloat32Pref(nil, 0.0);
CPreferences::Float64Pref	CPreferences::mNoFloat64Pref(nil, 0.0);
CPreferences::UInt32Pref	CPreferences::mNoUInt32Pref(nil, 0);
