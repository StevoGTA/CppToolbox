//----------------------------------------------------------------------------------------------------------------------
//	SValue.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SValue

class CDictionary;

struct SValue {
	// Type
	public:
		enum Type {
			kTypeEmpty,
			kTypeArrayOfDictionaries,
			kTypeArrayOfStrings,
			kTypeBool,
			kTypeData,
			kTypeDictionary,
			kTypeString,
			kTypeFloat32,
			kTypeFloat64,
			kTypeSInt8,
			kTypeSInt16,
			kTypeSInt32,
			kTypeSInt64,
			kTypeUInt8,
			kTypeUInt16,
			kTypeUInt32,
			kTypeUInt64,
			kTypeOpaque,
		};

	// Opaque
	typedef	const	void*	Opaque;

	// ValueTracker
	public:
		class ValueTracker {
			// Procs
			public:
				typedef	void	(*NoteValueChangedProc)(const SValue& value, void* userData);

			public:
								// Lifecycle methods
								ValueTracker(NoteValueChangedProc noteValueChangedProc = nil, void* userData = nil) :
									mNoteValueChangedProc(noteValueChangedProc), mUserData(userData)
									{}
				virtual			~ValueTracker() {}

								// Instance methods
				virtual	SValue	getValue() const = 0;

						void	setNoteValueChangedProc(NoteValueChangedProc noteValueChangedProc, void* userData)
									{ mNoteValueChangedProc = noteValueChangedProc; mUserData = userData; }

			protected:
								// Subclass methods
						void	noteValueChanged() const
									{ mNoteValueChangedProc(getValue(), mUserData); }

			// Properties
			protected:
				NoteValueChangedProc	mNoteValueChangedProc;
				void*					mUserData;
		};

	// Procs
	public:
		typedef			Opaque		(*OpaqueCopyProc)(Opaque opaque);
		typedef			bool		(*OpaqueEqualsProc)(Opaque opaque1, Opaque opaque2);
		typedef			void		(*OpaqueDisposeProc)(Opaque opaque);
		typedef	const	OR<SValue>	(*Proc)(const CString& key, void* userData);

	// Methods
	public:
												// Lifecycle methods
												SValue(const TArray<CDictionary>& value);
												SValue(const TArray<CString>& value);
												SValue(bool value);
												SValue(const CData& value);
												SValue(const CDictionary& value);
												SValue(const CString& value);
												SValue(Float32 value);
												SValue(Float64 value);
												SValue(SInt8 value);
												SValue(SInt16 value);
												SValue(SInt32 value);
												SValue(SInt64 value);
												SValue(UInt8 value);
												SValue(UInt16 value);
												SValue(UInt32 value);
												SValue(UInt64 value);
												SValue(Opaque value);
												SValue(const SValue& other, OpaqueCopyProc opaqueCopyProc = nil);

												// Instance methods
						Type					getType() const { return mType; }
						bool					canCoerceToType(Type type) const;

				const	TArray<CDictionary>&	getArrayOfDictionaries(
														const TArray<CDictionary>& defaultValue =
																TNArray<CDictionary>()) const;
				const	TArray<CString>&		getArrayOfStrings(
														const TArray<CString>& defaultValue = TNArray<CString>()) const;
						bool					getBool(bool defaultValue = false) const;
				const	CData&					getData(const CData& defaultValue = CData::mEmpty) const;
				const	CDictionary&			getDictionary(const CDictionary& defaultValue = getEmptyDictionary())
														const;
				const	CString&				getString(const CString& defaultValue = CString::mEmpty) const;
						Float32					getFloat32(Float32 defaultValue = 0.0) const;
						Float64					getFloat64(Float64 defaultValue = 0.0) const;
						SInt8					getSInt8(SInt8 defaultValue = 0) const;
						SInt16					getSInt16(SInt16 defaultValue = 0) const;
						SInt32					getSInt32(SInt32 defaultValue = 0) const;
						SInt64					getSInt64(SInt64 defaultValue = 0) const;
						UInt8					getUInt8(UInt8 defaultValue = 0) const;
						UInt16					getUInt16(UInt16 defaultValue = 0) const;
						UInt32					getUInt32(UInt32 defaultValue = 0) const;
						UInt64					getUInt64(UInt64 defaultValue = 0) const;
						Opaque					getOpaque() const;

						bool					equals(const SValue& other, OpaqueEqualsProc opaqueEqualsProc) const;

						void					dispose(OpaqueDisposeProc opaqueDisposeProc);

						SValue&					operator=(const SValue& other);

	private:
												// Lifecycle methods
												SValue();

												// Class methods
		static	const	CDictionary&			getEmptyDictionary();

	// Properties
	public:
		static	const	SValue	mEmpty;
		
	private:
						Type	mType;
						union ValueValue {
							// Lifecycle methods
							ValueValue(TArray<CDictionary>* value) : mArrayOfDictionaries(value) {}
							ValueValue(TArray<CString>* value) : mArrayOfStrings(value) {}
							ValueValue(bool value) : mBool(value) {}
							ValueValue(CData* value) : mData(value) {}
							ValueValue(CDictionary* value) : mDictionary(value) {}
							ValueValue(CString* value) : mString(value) {}
							ValueValue(Float32 value) : mFloat32(value) {}
							ValueValue(Float64 value) : mFloat64(value) {}
							ValueValue(SInt8 value) : mSInt8(value) {}
							ValueValue(SInt16 value) : mSInt16(value) {}
							ValueValue(SInt32 value) : mSInt32(value) {}
							ValueValue(SInt64 value) : mSInt64(value) {}
							ValueValue(UInt8 value) : mUInt8(value) {}
							ValueValue(UInt16 value) : mUInt16(value) {}
							ValueValue(UInt32 value) : mUInt32(value) {}
							ValueValue(UInt64 value) : mUInt64(value) {}
							ValueValue(Opaque value) : mOpaque(value) {}

							// Properties
							TArray<CDictionary>*	mArrayOfDictionaries;
							TArray<CString>*		mArrayOfStrings;
							bool					mBool;
							CData*					mData;
							CDictionary*			mDictionary;
							CString*				mString;
							Float32					mFloat32;
							Float64					mFloat64;
							SInt8					mSInt8;
							SInt16					mSInt16;
							SInt32					mSInt32;
							SInt64					mSInt64;
							UInt8					mUInt8;
							UInt16					mUInt16;
							UInt32					mUInt32;
							UInt64					mUInt64;
							Opaque					mOpaque;
						} mValue;
};
