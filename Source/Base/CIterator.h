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

	// Methods
	public:
						// Instance methods
		virtual	bool	hasValue() const = 0;
		virtual	bool	isFirstValue() const = 0;
		virtual	bool	advance() = 0;

	protected:
						// Lifecycle methods
		virtual			~CIterator() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorS (for single dereference!)

template <typename T> class TIteratorS : public CIterator {
	// Methods
	public:
				// Lifecycle methods
				TIteratorS(T* firstValue, AdvanceProc advanceProc, Info& info) :
					mAdvanceProc(advanceProc), mIsFirstValue(true), mCurrentValue(firstValue), mInfo(info)
					{}
				TIteratorS(const TIteratorS* other) :
					mAdvanceProc(other->mAdvanceProc), mIsFirstValue(other->mIsFirstValue),
							mCurrentValue(other->mCurrentValue), mInfo(*other->mInfo.copy())
					{}
				~TIteratorS()
					{ Info* info = &mInfo; Delete(info); }

				// Instance methods
		bool	hasValue() const
					{ return mCurrentValue != nil; }
		bool	isFirstValue() const
					{ return mIsFirstValue; }
		bool	advance()
					{
						// Advance
						mCurrentValue = (T*) mAdvanceProc(mInfo);
						mIsFirstValue = false;

						return mCurrentValue != nil;
					}

		T&		getValue() const
					{ return *mCurrentValue; }

		T&		operator*() const
					{ return *mCurrentValue; }
		T*		operator->() const
					{ return mCurrentValue; }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		bool		mIsFirstValue;
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
					mAdvanceProc(advanceProc), mIsFirstValue(true), mCurrentValue(firstValue), mInfo(info)
					{}
				TIteratorD(const TIteratorD* other) :
					mAdvanceProc(other->mAdvanceProc), mIsFirstValue(other->mIsFirstValue),
							mCurrentValue(other->mCurrentValue), mInfo(*other->mInfo.copy())
					{}
				~TIteratorD()
					{ Info* info = &mInfo; Delete(info); }

				// Instance methods
		bool	hasValue() const
					{ return mCurrentValue != nil; }
		bool	isFirstValue() const
					{ return mIsFirstValue; }
		bool	advance()
					{
						// Advance
						mCurrentValue = (T**) mAdvanceProc(mInfo);
						mIsFirstValue = false;

						return mCurrentValue != nil;
					}

		T&		getValue() const
					{ return **mCurrentValue; }

		T&		operator*() const
					{ return **mCurrentValue; }
		T*		operator->() const
					{ return *mCurrentValue; }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		bool		mIsFirstValue;
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
					mAdvanceProc(advanceProc), mIsFirstValue(true), mCurrentRawValue(firstRawValue), mMapProc(mapProc),
							mInfo(info)
					{}
				TIteratorM(const TIteratorM* other, T (mapProc)(K** rawValue)) :
					mAdvanceProc(other->mAdvanceProc), mIsFirstValue(other->mIsFirstValue),
							mCurrentRawValue(other->mCurrentRawValue), mMapProc(mapProc), mInfo(*other->mInfo.copy())
					{}
				~TIteratorM()
					{ Info*	info = &mInfo; Delete(info); }

				// Instance methods
		bool	hasValue() const
					{ return mCurrentRawValue != nil; }
		bool	isFirstValue() const
					{ return mIsFirstValue; }
		bool	advance()
					{
						// Advance
						mCurrentRawValue = (K**) mAdvanceProc(mInfo);
						mIsFirstValue = false;

						return mCurrentRawValue != nil;
					}

		T		getValue() const
					{ return mMapProc(mCurrentRawValue); }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		bool		mIsFirstValue;
		Info&		mInfo;
		K**			mCurrentRawValue;
		T 			(*mMapProc)(K** rawValue);
};
