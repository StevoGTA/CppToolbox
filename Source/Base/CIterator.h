//----------------------------------------------------------------------------------------------------------------------
//	CIterator.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CIteratorInfo
//	Will be deleted when the TIterator is deleted

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
// MARK: - Types
typedef	void*	(*CIteratorAdvanceProc)(CIteratorInfo& iteratorInfo);		// Return next value

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIterator

template <typename T> class TIterator /* : public CIterator */ {
	// Methods
	public:
				// Lifecycle methods
				TIterator(T* firstValue, CIteratorAdvanceProc advanceProc, CIteratorInfo& iteratorInfo) :
					mCurrentValue(firstValue), mAdvanceProc(advanceProc), mIteratorInfo(iteratorInfo)
					{}
				TIterator(const TIterator* other) :
					mCurrentValue(other->mCurrentValue), mAdvanceProc(other->mAdvanceProc),
							mIteratorInfo(*other->mIteratorInfo.copy())
					{}
				~TIterator()
					{ CIteratorInfo*	iteratorInfo = &mIteratorInfo; DisposeOf(iteratorInfo); }

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
