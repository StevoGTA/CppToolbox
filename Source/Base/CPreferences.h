//----------------------------------------------------------------------------------------------------------------------
//	CPreferences.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPreferences

class CPreferences {
	// Structs
	public:
		struct Pref {
			// Lifecycle methods
			Pref() : mKeyString(nil) {}
			Pref(OSStringType keyString) : mKeyString(keyString) {}
			Pref(const Pref& other) : mKeyString(other.mKeyString) {}

			// Properties
			OSStringType	mKeyString;	// "key"
		};

		struct StringPref : public Pref {
			// Lifecycle methods
			StringPref() : Pref(nil), mDefaultValue(OSSTR("")) {}
			StringPref(OSStringType keyString, OSStringVar(defaultValue) = OSSTR("")) :
				Pref(keyString), mDefaultValue(defaultValue)
				{}

			// Properties
			CString	mDefaultValue;
		};

		struct Float32Pref : public Pref {
			// Lifecycle methods
			Float32Pref() : Pref(nil), mDefaultValue(0.0) {}
			Float32Pref(OSStringType keyString, Float32 defaultValue = 0.0) :
				Pref(keyString), mDefaultValue(defaultValue)
				{}

			// Properties
			Float32	mDefaultValue;		// 32.0
		};

		struct Float64Pref : public Pref {
			// Lifecycle methods
			Float64Pref() : Pref(nil), mDefaultValue(0.0) {}
			Float64Pref(OSStringType keyString, Float64 defaultValue = 0.0) :
				Pref(keyString), mDefaultValue(defaultValue)
				{}

			// Properties
			Float64	mDefaultValue;		// 64.0
		};

		struct SInt32Pref : public Pref {
			// Lifecycle methods
			SInt32Pref() : Pref(nil), mDefaultValue(0) {}
			SInt32Pref(OSStringType keyString, SInt32 defaultValue = 0) :
				Pref(keyString), mDefaultValue(defaultValue)
				{}

			// Properties
			SInt32	mDefaultValue;		// 32
		};

		struct UInt32Pref : public Pref {
			// Lifecycle methods
			UInt32Pref() : Pref(nil), mDefaultValue(0) {}
			UInt32Pref(OSStringType keyString, UInt32 defaultValue = 0) :
				Pref(keyString), mDefaultValue(defaultValue)
				{}

			// Properties
			UInt32	mDefaultValue;		// 32
		};

		struct UInt64Pref : public Pref {
			// Lifecycle methods
			UInt64Pref() : Pref(nil), mDefaultValue(0) {}
			UInt64Pref(OSStringType keyString, UInt32 defaultValue = 0) :
				Pref(keyString), mDefaultValue(defaultValue)
				{}

			// Properties
			UInt64	mDefaultValue;		// 32
		};

		struct UniversalTimeIntervalPref : public Pref {
			// Lifecycle methods
			UniversalTimeIntervalPref() : Pref(nil), mDefaultValue(0.0) {}
			UniversalTimeIntervalPref(OSStringType keyString, Float64 defaultValue = 0.0) :
				Pref(keyString), mDefaultValue(defaultValue)
				{}

			// Properties
			UniversalTimeInterval	mDefaultValue;		// 64.0
		};

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		struct Reference {
			// Lifecycle methods
			Reference(const CString& applicationID) : mApplicationID(applicationID) {}
			Reference(const Reference& other) : mApplicationID(other.mApplicationID) {}

			// Properties
			CString	mApplicationID;
		};
#else
		struct Reference {
			// Lifecycle methods
			Reference(UInt32 dummy) {}
			Reference(const Reference& other) {}
		};
#endif

	// Classes
	private:
		class Internals;

	// Methods
	public:
									// Lifecycle methods
									CPreferences();
									CPreferences(const Reference& reference);
									~CPreferences();

									// Instance methods
		bool						hasValue(const Pref& pref);

		OV<TArray<CData> >			getDataArray(const Pref& pref);
		OV<TArray<CDictionary> >	getDictionaryArray(const Pref& pref);
		OV<TNumberArray<OSType> >	getOSTypeArray(const Pref& pref);
		OV<CData>					getData(const Pref& pref);
		OV<CDictionary>				getDictionary(const Pref& pref);
		CString						getString(const StringPref& pref);
		Float32						getFloat32(const Float32Pref& pref);
		Float64						getFloat64(const Float64Pref& pref);
		SInt32						getSInt32(const SInt32Pref& pref);
		UInt32						getUInt32(const UInt32Pref& pref);
		UInt64						getUInt64(const UInt64Pref& pref);
		UniversalTimeInterval		getUniversalTimeInterval(const UniversalTimeIntervalPref& pref);

		void						set(const Pref& pref, const TArray<CData>& array);
		void						set(const Pref& pref, const TArray<CDictionary>& array);
		void						set(const Pref& pref, const TNumberArray<OSType>& array);
		void						set(const Pref& pref, const CData& data);
		void						set(const Pref& pref, const CDictionary& dictionary);
		void						set(const StringPref& pref, const CString& string);
		void						set(const Float32Pref& pref, Float32 value);
		void						set(const Float64Pref& pref, Float64 value);
		void						set(const SInt32Pref& pref, SInt32 value);
		void						set(const UInt32Pref& pref, UInt32 value);
		void						set(const UInt64Pref& pref, UInt64 value);
		void						set(const UniversalTimeIntervalPref& pref, UniversalTimeInterval value);

		void						remove(const Pref& pref);
		
		void						beginGroupSet();
		void						endGroupSet();

		void						setAlternate(const Reference& reference);

	// Properties
	public:
		static			CPreferences	mDefault;

		static	const	Pref			mNoPref;
		static	const	StringPref		mNoStringPref;
		static	const	Float32Pref		mNoFloat32Pref;
		static	const	Float64Pref		mNoFloat64Pref;
		static	const	UInt32Pref		mNoUInt32Pref;

	private:
						Internals*		mInternals;
};
