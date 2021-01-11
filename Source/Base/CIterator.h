//----------------------------------------------------------------------------------------------------------------------
//	CIterator.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CIterator

class CIterator {
	// Classes
	public:
		// Will be deleted when the Iterator is deleted
		class Info {
			// Methods
			public:
								// Lifecycle methods
								Info() {}
				virtual			~Info() {}

								// Instance methods
				virtual	Info*	copy() = 0;
		};

	// Procs
	public:
		typedef	void*	(*AdvanceProc)(Info& info);	// Return next value
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorS (for single dereference!)

template <typename T> class TIteratorS : public CIterator {
	// Methods
	public:
				// Lifecycle methods
				TIteratorS(T* firstValue, AdvanceProc advanceProc, Info& info) :
					mCurrentValue(firstValue), mAdvanceProc(advanceProc), mInfo(info)
					{}
				TIteratorS(const TIteratorS* other) :
					mCurrentValue(other->mCurrentValue), mAdvanceProc(other->mAdvanceProc), mInfo(*other->mInfo.copy())
					{}
				~TIteratorS()
					{ Info* info = &mInfo; Delete(info); }

				// Instance methods
		bool	advance()
					{ mCurrentValue = (T*) mAdvanceProc(mInfo); return mCurrentValue != nil; }
		bool	hasValue() const
					{ return mCurrentValue != nil; }

		T&		getValue() const
					{ return *mCurrentValue; }

		T&		operator*() const
					{ return *mCurrentValue; }
		T*		operator->() const
					{ return mCurrentValue; }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		Info&		mInfo;
		T*			mCurrentValue;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorD (for double dereference!!)

template <typename T> class TIteratorD : public CIterator {
	// Methods
	public:
				// Lifecycle methods
				TIteratorD(T** firstValue, AdvanceProc advanceProc, Info& info) :
					mCurrentValue(firstValue), mAdvanceProc(advanceProc), mInfo(info)
					{}
				TIteratorD(const TIteratorD* other) :
					mCurrentValue(other->mCurrentValue), mAdvanceProc(other->mAdvanceProc), mInfo(*other->mInfo.copy())
					{}
				~TIteratorD()
					{ Info* info = &mInfo; Delete(info); }

				// Instance methods
		bool	advance()
					{ mCurrentValue = (T**) mAdvanceProc(mInfo); return mCurrentValue != nil; }
		bool	hasValue() const
					{ return mCurrentValue != nil; }

		T&		getValue() const
					{ return **mCurrentValue; }

		T&		operator*() const
					{ return **mCurrentValue; }
		T*		operator->() const
					{ return *mCurrentValue; }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		Info&		mInfo;
		T**			mCurrentValue;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorM (for mapping dereference!)

template <typename K, typename T> class TIteratorM : public CIterator {
	// Methods
	public:
				// Lifecycle methods
				TIteratorM(K** firstRawValue, T (mapProc)(K** rawValue), AdvanceProc advanceProc, Info& info) :
					mCurrentRawValue(firstRawValue), mMapProc(mapProc), mAdvanceProc(advanceProc), mInfo(info)
					{}
				TIteratorM(const TIteratorM* other, T (mapProc)(K** rawValue)) :
					mCurrentRawValue(other->mCurrentRawValue), mAdvanceProc(other->mAdvanceProc), mMapProc(mapProc),
							mInfo(*other->mInfo.copy())
					{}
				~TIteratorM()
					{ Info*	info = &mInfo; Delete(info); }

				// Instance methods
		bool	advance()
					{ mCurrentRawValue = (K**) mAdvanceProc(mInfo); return mCurrentRawValue != nil; }
		bool	hasValue() const
					{ return mCurrentRawValue != nil; }

		T		getValue() const
					{ return mMapProc(mCurrentRawValue); }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		Info&		mInfo;
		K**			mCurrentRawValue;
		T 			(*mMapProc)(K** rawValue);
};
