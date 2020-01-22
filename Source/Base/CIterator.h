//----------------------------------------------------------------------------------------------------------------------
//	CIterator.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CIteratorInfo
//	Will be deleted when the Iterator is deleted

class CIteratorInfo {
	// Methods
	public:
								// Lifecycle methods
								CIteratorInfo() {}
		virtual					~CIteratorInfo() {}

								// Instance methods
		virtual	CIteratorInfo*	copy() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Procs

typedef	void*	(*CIteratorAdvanceProc)(CIteratorInfo& iteratorInfo);	// Return next value

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorS (for single dereference!)

template <typename T> class TIteratorS {
	// Methods
	public:
				// Lifecycle methods
				TIteratorS(T* firstValue, CIteratorAdvanceProc advanceProc, CIteratorInfo& iteratorInfo) :
					mCurrentValue(firstValue), mAdvanceProc(advanceProc), mIteratorInfo(iteratorInfo)
					{}
				TIteratorS(const TIteratorS* other) :
					mCurrentValue(other->mCurrentValue), mAdvanceProc(other->mAdvanceProc),
							mIteratorInfo(*other->mIteratorInfo.copy())
					{}
				~TIteratorS()
					{ CIteratorInfo* iteratorInfo = &mIteratorInfo; DisposeOf(iteratorInfo); }

				// Instance methods
		bool	advance()
					{ mCurrentValue = (T*) mAdvanceProc(mIteratorInfo); return mCurrentValue != nil; }
		bool	hasValue() const
					{ return mCurrentValue != nil; }

		T&		getValue() const
					{ return *mCurrentValue; }

	// Properties
	private:
		CIteratorAdvanceProc	mAdvanceProc;
		CIteratorInfo&			mIteratorInfo;
		T*						mCurrentValue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorD (for double dereference!)

template <typename T> class TIteratorD {
	// Methods
	public:
				// Lifecycle methods
				TIteratorD(T** firstValue, CIteratorAdvanceProc advanceProc, CIteratorInfo& iteratorInfo) :
					mCurrentValue(firstValue), mAdvanceProc(advanceProc), mIteratorInfo(iteratorInfo)
					{}
				TIteratorD(const TIteratorD* other) :
					mCurrentValue(other->mCurrentValue), mAdvanceProc(other->mAdvanceProc),
							mIteratorInfo(*other->mIteratorInfo.copy())
					{}
				~TIteratorD()
					{ CIteratorInfo*	iteratorInfo = &mIteratorInfo; DisposeOf(iteratorInfo); }

				// Instance methods
		bool	advance()
					{ mCurrentValue = (T**) mAdvanceProc(mIteratorInfo); return mCurrentValue != nil; }
		bool	hasValue() const
					{ return mCurrentValue != nil; }

		T&		getValue() const
					{ return **mCurrentValue; }

	// Properties
	private:
		CIteratorAdvanceProc	mAdvanceProc;
		CIteratorInfo&			mIteratorInfo;
		T**						mCurrentValue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorM (for mapping dereference!)

template <typename K, typename T> class TIteratorM {
	// Methods
	public:
				// Lifecycle methods
				TIteratorM(K** firstRawValue, T (mapProc)(K** rawValue), CIteratorAdvanceProc advanceProc,
						CIteratorInfo& iteratorInfo) :
					mCurrentRawValue(firstRawValue), mMapProc(mapProc), mAdvanceProc(advanceProc),
							mIteratorInfo(iteratorInfo)
					{}
				TIteratorM(const TIteratorM* other, T (mapProc)(K** rawValue)) :
					mCurrentRawValue(other->mCurrentRawValue), mAdvanceProc(other->mAdvanceProc),
							mMapProc(mapProc), mIteratorInfo(*other->mIteratorInfo.copy())
					{}
				~TIteratorM()
					{ CIteratorInfo*	iteratorInfo = &mIteratorInfo; DisposeOf(iteratorInfo); }

				// Instance methods
		bool	advance()
					{ mCurrentRawValue = (K**) mAdvanceProc(mIteratorInfo); return mCurrentRawValue != nil; }
		bool	hasValue() const
					{ return mCurrentRawValue != nil; }

		T		getValue() const
					{ return mMapProc(mCurrentRawValue); }

	// Properties
	private:
		CIteratorAdvanceProc	mAdvanceProc;
		CIteratorInfo&			mIteratorInfo;
		K**						mCurrentRawValue;
		T 						(*mMapProc)(K** rawValue);
};
