//----------------------------------------------------------------------------------------------------------------------
//	CPreferences.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
//#include "CFileX.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Structures

struct SPref {
	// Lifecycle methods
	SPref() : mKeyString(nil) {}
	SPref(const char* keyString) : mKeyString(keyString) {}
	SPref(const SPref& other) : mKeyString(other.mKeyString) {}

	// Properties
	const	char*	mKeyString;	// "key"
};

struct SStringPref : public SPref {
	// Lifecycle methods
	SStringPref() : SPref(nil), mDefaultValue(OSSTR("")) {}
	SStringPref(const char* keyString, OSString defaultValue) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	OSString	mDefaultValue;	// OSSTR("value")
};

struct SFloat32Pref : public SPref {
	// Lifecycle methods
	SFloat32Pref() : SPref(nil), mDefaultValue(0.0) {}
	SFloat32Pref(const char* keyString, Float32 defaultValue) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	Float32	mDefaultValue;		// 32.0
};

struct SFloat64Pref : public SPref {
	// Lifecycle methods
	SFloat64Pref() : SPref(nil), mDefaultValue(0.0) {}
	SFloat64Pref(const char* keyString, Float64 defaultValue) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	Float64	mDefaultValue;		// 64.0
};

struct SSInt32Pref : public SPref {
	// Lifecycle methods
	SSInt32Pref() : SPref(nil), mDefaultValue(0) {}
	SSInt32Pref(const char* keyString, SInt32 defaultValue) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	SInt32	mDefaultValue;		// 32
};

struct SUInt32Pref : public SPref {
	// Lifecycle methods
	SUInt32Pref() : SPref(nil), mDefaultValue(0) {}
	SUInt32Pref(const char* keyString, UInt32 defaultValue) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	UInt32	mDefaultValue;		// 32
};

struct SUInt64Pref : public SPref {
	// Lifecycle methods
	SUInt64Pref() : SPref(nil), mDefaultValue(0) {}
	SUInt64Pref(const char* keyString, UInt32 defaultValue) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	UInt64	mDefaultValue;		// 32
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPreferences

class CPreferences {
	// Methods
	public:
										// Class methods
		static	bool					hasValue(const SPref& pref,
												const CString& applicationID = CString::mEmpty);

		static	TArray<CData>			getDataArray(const SPref& pref,
												const CString& applicationID = CString::mEmpty);
		static	TNumericArray<OSType>	getOSTypeArray(const SPref& pref,
												const CString& applicationID = CString::mEmpty);
		static	CData					getData(const SPref& pref,
												const CString& applicationID = CString::mEmpty);
		static	CDictionary				getDictionary(const SPref& pref,
												const CString& applicationID = CString::mEmpty);
		static	CString					getString(const SStringPref& pref,
												const CString& applicationID = CString::mEmpty);
		static	Float32					getFloat32(const SFloat32Pref& pref,
												const CString& applicationID = CString::mEmpty);
		static	Float64					getFloat64(const SFloat64Pref& pref,
												const CString& applicationID = CString::mEmpty);
		static	SInt32					getSInt32(const SSInt32Pref& pref,
												const CString& applicationID = CString::mEmpty);
		static	UInt32					getUInt32(const SUInt32Pref& pref,
												const CString& applicationID = CString::mEmpty);
		static	UInt64					getUInt64(const SUInt64Pref& pref,
												const CString& applicationID = CString::mEmpty);

		static	void					set(const SPref& pref, const TPtrArray<CData*>& array);
		static	void					set(const SPref& pref, const TNumericArray<OSType>& array);
		static	void					set(const SPref& pref, const CData& data);
		static	void					set(const SPref& pref, const CDictionary& dictionary);
		static	void					set(const SStringPref& pref, const CString& string);
		static	void					set(const SFloat32Pref& pref, Float32 value);
		static	void					set(const SFloat64Pref& pref, Float64 value);
		static	void					set(const SSInt32Pref& pref, SInt32 value);
		static	void					set(const SUInt32Pref& pref, UInt32 value);
		static	void					set(const SUInt64Pref& pref, UInt64 value);

		static	void					remove(const SPref& pref);
		
		static	void					beginGroupSet();
		static	void					endGroupSet();
		
		static	void					setAlternateApplicationID(const CString& applicationID = CString::mEmpty);
#if TARGET_OS_MACOS
//		static	CFileX					getPrimaryFile();
#endif

										// Deprecated
//		static	CArrayX					getArray(const SPref& pref,
//												const CString& applicationID = CString::mEmpty);
//		static	CDictionaryX			getDictionaryX(const SPref& pref,
//												const CString& applicationID = CString::mEmpty);
//		static	void					set(const SPref& pref, const CArrayX& array);
//		static	void					set(const SPref& pref, const CDictionaryX& dict);

	// Properties
	public:
		static	SPref			mNoPref;
		static	SStringPref		mNoStringPref;
		static	SFloat32Pref	mNoFloat32Pref;
		static	SFloat64Pref	mNoFloat64Pref;
		static	SUInt32Pref		mNoUInt32Pref;
};
