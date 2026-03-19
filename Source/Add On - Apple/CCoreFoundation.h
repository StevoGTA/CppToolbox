//----------------------------------------------------------------------------------------------------------------------
//	CCoreFoundation.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreFoundation

class CCoreFoundation {
	// O (CoreFoundation Object)
	public:
		template <typename T> struct O {
			// Methods
			public:
					// Lifecycle methods
					O(const T& object) : mObject((T) ::CFRetain(object)) {}
					~O() { ::CFRelease(mObject); }

					// Instance methods
				T&	operator*() const
						{ return (T&) mObject; }

			// Properties
			private:
				T	mObject;
		};

	// OO (Optional CoreFoundationObject)
	public:
		template <typename T> struct OO {
			// Methods
			public:
						// Lifecycle methods
						OO() : mObject(nil) {}
						OO(const T& object) : mObject((T) ::CFRetain(object)) {}
						~OO() { if (mObject != nil) ::CFRelease(mObject); }

						// Instance methods
				bool	hasObject() const
							{ return mObject != nil; }
				T&		operator*() const
							{ return (T&) mObject; }

			// Properties
			private:
				T	mObject;
		};

	// Methods
	public:
										// Array methods
		static	TArray<CData>			dataArrayFrom(CFArrayRef arrayRef);
		static	TArray<CDictionary>		dictionaryArrayFrom(CFArrayRef arrayRef);
		static	TNumberArray<OSType>	osTypeArrayFrom(CFArrayRef arrayRef);
		static	TArray<CString>			stringArrayFrom(CFArrayRef arrayRef);
		static	O<CFArrayRef>			arrayRefFrom(const TArray<CData>& array);
		static	O<CFArrayRef>			arrayRefFrom(const TArray<CDictionary>& array);
		static	O<CFArrayRef>			arrayRefFrom(const TNumberArray<OSType>& array);
		static	O<CFArrayRef>			arrayRefFrom(const TArray<CString>& array);

										// Data methods
		static	CData					dataFrom(CFDataRef dataRef);
		static	O<CFDataRef>			dataRefFrom(const CData& data);

										// Dictionary methods
		static	CDictionary				dictionaryFrom(CFDictionaryRef dictionaryRef);
		static	TDictionary<CString>	dictionaryOfStringsFrom(CFDictionaryRef dictionaryRef);
		static	O<CFDictionaryRef>		dictionaryRefFrom(const CDictionary& dictionary);
		static	O<CFDictionaryRef>		dictionaryRefFrom(const TDictionary<CString>& dictionary);

										// Number methods
		static	Float32					float32From(CFNumberRef numberRef);
		static	Float64					float64From(CFNumberRef numberRef);
		static	SInt8					sInt8From(CFNumberRef numberRef);
		static	SInt16					sInt16From(CFNumberRef numberRef);
		static	SInt32					sInt32From(CFNumberRef numberRef);
		static	SInt64					sInt64From(CFNumberRef numberRef);
		static	UInt8					uInt8From(CFNumberRef numberRef);
		static	UInt16					uInt16From(CFNumberRef numberRef);
		static	UInt32					uInt32From(CFNumberRef numberRef);
		static	UInt64					uInt64From(CFNumberRef numberRef);

		static	O<CFNumberRef>			numberRefFrom(Float32 value);
		static	O<CFNumberRef>			numberRefFrom(Float64 value);
		static	O<CFNumberRef>			numberRefFrom(SInt8 value);
		static	O<CFNumberRef>			numberRefFrom(SInt16 value);
		static	O<CFNumberRef>			numberRefFrom(SInt32 value);
		static	O<CFNumberRef>			numberRefFrom(SInt64 value);
		static	O<CFNumberRef>			numberRefFrom(UInt8 value);
		static	O<CFNumberRef>			numberRefFrom(UInt16 value);
		static	O<CFNumberRef>			numberRefFrom(UInt32 value);
		static	O<CFNumberRef>			numberRefFrom(UInt64 value);

										// Set methods
		static	TSet<CString>			setOfStringsFrom(const CFSetRef setRef);
};
