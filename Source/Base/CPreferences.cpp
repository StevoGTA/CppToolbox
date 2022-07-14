//----------------------------------------------------------------------------------------------------------------------
//	CPreferences.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

const	CPreferences::Pref			CPreferences::mNoPref(nil);
const	CPreferences::StringPref	CPreferences::mNoStringPref(nil, OSSTR(""));
const	CPreferences::Float32Pref	CPreferences::mNoFloat32Pref(nil, 0.0);
const	CPreferences::Float64Pref	CPreferences::mNoFloat64Pref(nil, 0.0);
const	CPreferences::UInt32Pref	CPreferences::mNoUInt32Pref(nil, 0);
