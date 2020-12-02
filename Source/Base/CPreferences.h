//----------------------------------------------------------------------------------------------------------------------
//	CPreferences.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Structures

struct SPref {
	// Lifecycle methods
	SPref() : mKeyString(nil) {}
	SPref(OSStringType keyString) : mKeyString(keyString) {}
	SPref(const SPref& other) : mKeyString(other.mKeyString) {}

	// Properties
	OSStringType	mKeyString;	// "key"
};

struct SStringPref : public SPref {
	// Lifecycle methods
	SStringPref() : SPref(nil), mDefaultValue(OSSTR("")) {}
	SStringPref(OSStringType keyString, OSStringVar(defaultValue) = OSSTR("")) :
		SPref(keyString), mDefaultValue(defaultValue)
		{}

	// Properties
	CString	mDefaultValue;
};

struct SFloat32Pref : public SPref {
	// Lifecycle methods
	SFloat32Pref() : SPref(nil), mDefaultValue(0.0) {}
	SFloat32Pref(OSStringType keyString, Float32 defaultValue = 0.0) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	Float32	mDefaultValue;		// 32.0
};

struct SFloat64Pref : public SPref {
	// Lifecycle methods
	SFloat64Pref() : SPref(nil), mDefaultValue(0.0) {}
	SFloat64Pref(OSStringType keyString, Float64 defaultValue = 0.0) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	Float64	mDefaultValue;		// 64.0
};

struct SSInt32Pref : public SPref {
	// Lifecycle methods
	SSInt32Pref() : SPref(nil), mDefaultValue(0) {}
	SSInt32Pref(OSStringType keyString, SInt32 defaultValue = 0) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	SInt32	mDefaultValue;		// 32
};

struct SUInt32Pref : public SPref {
	// Lifecycle methods
	SUInt32Pref() : SPref(nil), mDefaultValue(0) {}
	SUInt32Pref(OSStringType keyString, UInt32 defaultValue = 0) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	UInt32	mDefaultValue;		// 32
};

struct SUInt64Pref : public SPref {
	// Lifecycle methods
	SUInt64Pref() : SPref(nil), mDefaultValue(0) {}
	SUInt64Pref(OSStringType keyString, UInt32 defaultValue = 0) : SPref(keyString), mDefaultValue(defaultValue) {}

	// Properties
	UInt64	mDefaultValue;		// 32
};

struct SUniversalTimeIntervalPref : public SPref {
	// Lifecycle methods
	SUniversalTimeIntervalPref() : SPref(nil), mDefaultValue(0.0) {}
	SUniversalTimeIntervalPref(OSStringType keyString, Float64 defaultValue = 0.0) :
		SPref(keyString), mDefaultValue(defaultValue)
		{}

	// Properties
	UniversalTimeInterval	mDefaultValue;		// 64.0
};

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
struct SPreferencesReference {
	// Lifecycle methods
	SPreferencesReference(const CString& applicationID) : mApplicationID(applicationID) {}
	SPreferencesReference(const SPreferencesReference& other) : mApplicationID(other.mApplicationID) {}

	// Properties
	CString	mApplicationID;
};
#else
struct SPreferencesReference {
	// Lifecycle methods
	SPreferencesReference(UInt32 dummy) {}
	SPreferencesReference(const SPreferencesReference& other) {}
};
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPreferences

class CPreferencesInternals;
class CPreferences {
	// Methods
	public:
								// Lifecycle methods
								CPreferences();
								CPreferences(const SPreferencesReference& preferencesReference);
								~CPreferences();

								// Instance methods
		bool					hasValue(const SPref& pref);

		TNArray<CData>			getDataArray(const SPref& pref);
		TNArray<CDictionary>	getDictionaryArray(const SPref& pref);
		TNumericArray<OSType>	getOSTypeArray(const SPref& pref);
		CData					getData(const SPref& pref);
		CDictionary				getDictionary(const SPref& pref);
		CString					getString(const SStringPref& pref);
		Float32					getFloat32(const SFloat32Pref& pref);
		Float64					getFloat64(const SFloat64Pref& pref);
		SInt32					getSInt32(const SSInt32Pref& pref);
		UInt32					getUInt32(const SUInt32Pref& pref);
		UInt64					getUInt64(const SUInt64Pref& pref);
		UniversalTimeInterval	getUniversalTimeInterval(const SUniversalTimeIntervalPref& pref);

		void					set(const SPref& pref, const TArray<CData>& array);
		void					set(const SPref& pref, const TArray<CDictionary>& array);
		void					set(const SPref& pref, const TNumericArray<OSType>& array);
		void					set(const SPref& pref, const CData& data);
		void					set(const SPref& pref, const CDictionary& dictionary);
		void					set(const SStringPref& pref, const CString& string);
		void					set(const SFloat32Pref& pref, Float32 value);
		void					set(const SFloat64Pref& pref, Float64 value);
		void					set(const SSInt32Pref& pref, SInt32 value);
		void					set(const SUInt32Pref& pref, UInt32 value);
		void					set(const SUInt64Pref& pref, UInt64 value);
		void					set(const SUniversalTimeIntervalPref& pref, UniversalTimeInterval value);

		void					remove(const SPref& pref);
		
		void					beginGroupSet();
		void					endGroupSet();

		void					setAlternate(const SPreferencesReference& preferencesReference);

	// Properties
	public:
		static	CPreferences			mDefault;

		static	SPref					mNoPref;
		static	SStringPref				mNoStringPref;
		static	SFloat32Pref			mNoFloat32Pref;
		static	SFloat64Pref			mNoFloat64Pref;
		static	SUInt32Pref				mNoUInt32Pref;

	private:
				CPreferencesInternals*	mInternals;
};
