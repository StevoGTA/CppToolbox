//----------------------------------------------------------------------------------------------------------------------
//	CPreferences.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPreferences

class CPreferences {
	// Pref
	public:
		struct Pref {
			// Methods
			public:
								// Lifecycle methods
								Pref(OSStringType keyString) : mKeyString(keyString) {}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }

			// Properties
			private:
				OSStringType	mKeyString;
		};

	// BoolPref
	public:
		struct BoolPref {
			// Methods
			public:
								// Lifecycle methods
								BoolPref(OSStringType keyString, bool defaultValue = false) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				bool			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				bool			mDefaultValue;
		};

	// StringPref
	public:
		struct StringPref {
			// Methods
			public:
										// Lifecycle methods
										StringPref(OSStringType keyString, OSStringVar(defaultValue) = OSSTR("")) :
											mKeyString(keyString), mDefaultValue(defaultValue)
											{}

										// Instance methods
						OSStringType	getKeyString() const
											{ return mKeyString; }
				const	CString&		getDefaultValue() const
											{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				CString			mDefaultValue;
		};

	// Float32Pref
	public:
		struct Float32Pref {
			// Methods
			public:
								// Lifecycle methods
								Float32Pref(OSStringType keyString, Float32 defaultValue = 0.0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				Float32			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				Float32			mDefaultValue;
		};

	// Float64Pref
	public:
		struct Float64Pref {
								// Lifecycle methods
								Float64Pref(OSStringType keyString, Float64 defaultValue = 0.0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				Float64			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				Float64			mDefaultValue;
		};

	// SInt8Pref
	public:
		struct SInt8Pref {
			// Methods
			public:
								// Lifecycle methods
								SInt8Pref(OSStringType keyString, SInt8 defaultValue = 0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				SInt8			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				SInt8			mDefaultValue;
		};

	// SInt16Pref
	public:
		struct SInt16Pref {
			// Methods
			public:
								// Lifecycle methods
								SInt16Pref(OSStringType keyString, SInt16 defaultValue = 0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				SInt16			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				SInt16			mDefaultValue;
		};

	// SInt32Pref
	public:
		struct SInt32Pref {
			// Methods
			public:
								// Lifecycle methods
								SInt32Pref(OSStringType keyString, SInt32 defaultValue = 0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				SInt32			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				SInt32			mDefaultValue;
		};

	// SInt64Pref
	public:
		struct SInt64Pref {
			// Methods
			public:
								// Lifecycle methods
								SInt64Pref(OSStringType keyString, SInt64 defaultValue = 0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				SInt64			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				SInt64			mDefaultValue;
		};

	// UInt8Pref
	public:
		struct UInt8Pref {
			// Methods
			public:
								// Lifecycle methods
								UInt8Pref(OSStringType keyString, UInt8 defaultValue = 0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				UInt8			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				UInt8			mDefaultValue;
		};

	// UInt16Pref
	public:
		struct UInt16Pref {
			// Methods
			public:
								// Lifecycle methods
								UInt16Pref(OSStringType keyString, UInt16 defaultValue = 0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				UInt16			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				UInt16			mDefaultValue;
		};

	// UInt32Pref
	public:
		struct UInt32Pref {
			// Methods
			public:
								// Lifecycle methods
								UInt32Pref(OSStringType keyString, UInt32 defaultValue = 0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				UInt32			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				UInt32			mDefaultValue;
		};

	// UInt64Pref
	public:
		struct UInt64Pref {
			// Methods
			public:
								// Lifecycle methods
								UInt64Pref(OSStringType keyString, UInt32 defaultValue = 0) :
									mKeyString(keyString), mDefaultValue(defaultValue)
									{}

								// Instance methods
				OSStringType	getKeyString() const
									{ return mKeyString; }
				UInt64			getDefaultValue() const
									{ return mDefaultValue; }

			// Properties
			private:
				OSStringType	mKeyString;
				UInt64			mDefaultValue;
		};

	// UniversalTimeIntervalPref
	public:
		struct UniversalTimeIntervalPref {
			// Methods
			public:
										// Lifecycle methods
										UniversalTimeIntervalPref(OSStringType keyString,
												UniversalTimeInterval defaultValue = 0.0) :
											mKeyString(keyString), mDefaultValue(defaultValue)
											{}

										// Instance methods
				OSStringType			getKeyString() const
											{ return mKeyString; }
				UniversalTimeInterval	getDefaultValue() const
											{ return mDefaultValue; }

			// Properties
			private:
				OSStringType			mKeyString;
				UniversalTimeInterval	mDefaultValue;
		};

	// Reference
	public:
		struct Reference {
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
			// Methods
			public:
									// Lifecycle methods
									Reference(CFStringRef applicationIDStringRef) :
										mApplicationIDStringRef(applicationIDStringRef)
										{}
									Reference(const Reference& other) :
										mApplicationIDStringRef(other.mApplicationIDStringRef)
										{}

									// Instance methods
				const	CFStringRef	getApplicationIDStringRef() const
										{ return mApplicationIDStringRef; }

			// Properties
			private:
				CFStringRef	mApplicationIDStringRef;
#else
			// Methods
			public:
				// Lifecycle methods
				Reference(UInt32 dummy) {}
				Reference(const Reference& other) {}
#endif
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											~CPreferences();

											// Instance methods
				bool						hasValue(const Pref& pref);
				bool						hasValue(const BoolPref& boolPref)
												{ return hasValue(Pref(boolPref.getKeyString())); }
				bool						hasValue(const StringPref& stringPref)
												{ return hasValue(Pref(stringPref.getKeyString())); }
				bool						hasValue(const Float32Pref& float32Pref)
												{ return hasValue(Pref(float32Pref.getKeyString())); }
				bool						hasValue(const Float64Pref& float64Pref)
												{ return hasValue(Pref(float64Pref.getKeyString())); }
				bool						hasValue(const SInt8Pref& sInt8Pref)
												{ return hasValue(Pref(sInt8Pref.getKeyString())); }
				bool						hasValue(const SInt16Pref& sInt16Pref)
												{ return hasValue(Pref(sInt16Pref.getKeyString())); }
				bool						hasValue(const SInt32Pref& sInt32Pref)
												{ return hasValue(Pref(sInt32Pref.getKeyString())); }
				bool						hasValue(const SInt64Pref& sInt64Pref)
												{ return hasValue(Pref(sInt64Pref.getKeyString())); }
				bool						hasValue(const UInt8Pref& uInt8Pref)
												{ return hasValue(Pref(uInt8Pref.getKeyString())); }
				bool						hasValue(const UInt16Pref& uInt16Pref)
												{ return hasValue(Pref(uInt16Pref.getKeyString())); }
				bool						hasValue(const UInt32Pref& uInt32Pref)
												{ return hasValue(Pref(uInt32Pref.getKeyString())); }
				bool						hasValue(const UInt64Pref& uInt64Pref)
												{ return hasValue(Pref(uInt64Pref.getKeyString())); }
				bool						hasValue(const UniversalTimeIntervalPref& universalTimeIntervalPref)
												{ return hasValue(Pref(universalTimeIntervalPref.getKeyString())); }

				bool						getBool(const BoolPref& boolPref);
				OV<TArray<CData> >			getDataArray(const Pref& pref);
				OV<TArray<CDictionary> >	getDictionaryArray(const Pref& pref);
				OV<TNumberArray<OSType> >	getOSTypeArray(const Pref& pref);
				OV<CData>					getData(const Pref& pref);
				OV<CDictionary>				getDictionary(const Pref& pref);
				CString						getString(const StringPref& stringPref);
				Float32						getFloat32(const Float32Pref& float32Pref);
				Float64						getFloat64(const Float64Pref& float64Pref);
				SInt8						getSInt8(const SInt8Pref& sInt8Pref);
				SInt16						getSInt16(const SInt16Pref& sInt16Pref);
				SInt32						getSInt32(const SInt32Pref& sInt32Pref);
				SInt64						getSInt64(const SInt64Pref& sInt64Pref);
				UInt8						getUInt8(const UInt8Pref& uInt8Pref);
				UInt16						getUInt16(const UInt16Pref& uInt16Pref);
				UInt32						getUInt32(const UInt32Pref& uInt32Pref);
				UInt64						getUInt64(const UInt64Pref& uInt64Pref);
				UniversalTimeInterval		getUniversalTimeInterval(
													const UniversalTimeIntervalPref& universalTimeIntervalPref);

				void						set(const BoolPref& boolPref, bool value);
				void						set(const Pref& pref, const TArray<CData>& array);
				void						set(const Pref& pref, const TArray<CDictionary>& array);
				void						set(const Pref& pref, const TNumberArray<OSType>& array);
				void						set(const Pref& pref, const CData& data);
				void						set(const Pref& pref, const CDictionary& dictionary);
				void						set(const StringPref& stringPref, const CString& string);
				void						set(const Float32Pref& float32Pref, Float32 value);
				void						set(const Float64Pref& float64Pref, Float64 value);
				void						set(const SInt8Pref& sInt8Pref, SInt8 value);
				void						set(const SInt16Pref& sInt16Pref, SInt16 value);
				void						set(const SInt32Pref& sInt32Pref, SInt32 value);
				void						set(const SInt64Pref& sInt64Pref, SInt64 value);
				void						set(const UInt8Pref& uInt8Pref, UInt8 value);
				void						set(const UInt16Pref& uInt16Pref, UInt16 value);
				void						set(const UInt32Pref& uInt32Pref, UInt32 value);
				void						set(const UInt64Pref& uInt64Pref, UInt64 value);
				void						set(const UniversalTimeIntervalPref& universalTimeIntervalPref,
													UniversalTimeInterval value);

				void						remove(const Pref& pref);
				void						remove(const BoolPref& boolPref)
												{ remove(Pref(boolPref.getKeyString())); }
				void						remove(const StringPref& stringPref)
												{ remove(Pref(stringPref.getKeyString())); }
				void						remove(const Float32Pref& float32Pref)
												{ remove(Pref(float32Pref.getKeyString())); }
				void						remove(const Float64Pref& float64Pref)
												{ remove(Pref(float64Pref.getKeyString())); }
				void						remove(const SInt8Pref& sInt8Pref)
												{ remove(Pref(sInt8Pref.getKeyString())); }
				void						remove(const SInt16Pref& sInt16Pref)
												{ remove(Pref(sInt16Pref.getKeyString())); }
				void						remove(const SInt32Pref& sInt32Pref)
												{ remove(Pref(sInt32Pref.getKeyString())); }
				void						remove(const SInt64Pref& sInt64Pref)
												{ remove(Pref(sInt64Pref.getKeyString())); }
				void						remove(const UInt8Pref& uInt8Pref)
												{ remove(Pref(uInt8Pref.getKeyString())); }
				void						remove(const UInt16Pref& uInt16Pref)
												{ remove(Pref(uInt16Pref.getKeyString())); }
				void						remove(const UInt32Pref& uInt32Pref)
												{ remove(Pref(uInt32Pref.getKeyString())); }
				void						remove(const UInt64Pref& uInt64Pref)
												{ remove(Pref(uInt64Pref.getKeyString())); }
				void						remove(const UniversalTimeIntervalPref& universalTimeIntervalPref)
												{ remove(Pref(universalTimeIntervalPref.getKeyString())); }

				void						beginGroupSet();
				void						endGroupSet();

				void						setAlternate(const Reference& reference);

											// Class methods
		static	CPreferences&				shared();

	private:
											// Lifecycle methods
											CPreferences();
											CPreferences(const Reference& reference);

	// Properties
	private:
		Internals*	mInternals;
};
