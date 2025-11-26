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
		virtual	bool		hasValue() const = 0;
		virtual	UInt32		getIndex() const = 0;
				bool		isFirstValue() const
								{ return getIndex() == 0; }
		virtual	bool		advance() = 0;

		virtual	AdvanceProc	getAdvanceProc() const = 0;
		virtual	Info&		getInfo() const = 0;

	protected:
							// Lifecycle methods
		virtual				~CIterator() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorS (for single dereference!)

template <typename T> class TIteratorS : public CIterator {
	// Methods
	public:
					// Lifecycle methods
					TIteratorS(T* firstValue, AdvanceProc advanceProc, Info& info) :
						mAdvanceProc(advanceProc), mInfo(info), mCurrentValue(firstValue), mIndex(0)
						{}
					TIteratorS(const TIteratorS& other) :
						mAdvanceProc(other.mAdvanceProc), mInfo(*other.mInfo.copy()),
								mCurrentValue(other.mCurrentValue), mIndex(other.mIndex)
						{}
					~TIteratorS()
						{ Info* info = &mInfo; Delete(info); }

					// Instance methods
		bool		hasValue() const
						{ return mCurrentValue != nil; }
		UInt32		getIndex() const
						{ return mIndex; }
		bool		advance()
						{
							// Advance
							mCurrentValue = (T*) mAdvanceProc(mInfo);
							mIndex++;

							return mCurrentValue != nil;
						}

		T&			getValue() const
						{ return *mCurrentValue; }
		T*			getRawValue() const
						{ return mCurrentValue; }

		T&			operator*() const
						{ return *mCurrentValue; }
		T*			operator->() const
						{ return mCurrentValue; }

	protected:
					// CIterator methods
		AdvanceProc	getAdvanceProc() const
						{ return mAdvanceProc; }
		Info&		getInfo() const
						{ return mInfo; }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		Info&		mInfo;
		T*			mCurrentValue;
		UInt32		mIndex;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorD (for double dereference!!)

template <typename T> class TIteratorD : public CIterator {
	// Methods
	public:
					// Lifecycle methods
					TIteratorD(const CIterator& other, T** currentValue) :
						mAdvanceProc(other.getAdvanceProc()), mInfo(*other.getInfo().copy()),
								mCurrentValue(currentValue), mIndex(other.getIndex())
						{}
					~TIteratorD()
						{ Info* info = &mInfo; Delete(info); }

					// Instance methods
		bool		hasValue() const
						{ return mCurrentValue != nil; }
		UInt32		getIndex() const
						{ return mIndex; }
		bool		advance()
						{
							// Advance
							mCurrentValue = (T**) mAdvanceProc(mInfo);
							mIndex++;

							return mCurrentValue != nil;
						}

		T&			getValue() const
						{ return **mCurrentValue; }

		T&			operator*() const
						{ return **mCurrentValue; }
		T*			operator->() const
						{ return *mCurrentValue; }

	protected:
					// CIterator methods
		AdvanceProc	getAdvanceProc() const
						{ return mAdvanceProc; }
		Info&		getInfo() const
						{ return mInfo; }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		Info&		mInfo;
		T**			mCurrentValue;
		UInt32		mIndex;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIteratorM (for mapping dereference!)

template <typename K, typename T> class TIteratorM : public CIterator {
	// Methods
	public:
					// Lifecycle methods
					TIteratorM(const CIterator& other, K** currentRawValue, T (mapProc)(K** rawValue)) :
						mAdvanceProc(other.getAdvanceProc()), mInfo(*other.getInfo().copy()),
								mCurrentRawValue(currentRawValue), mMapProc(mapProc), mIndex(other.getIndex())
						{}
					~TIteratorM()
						{ Info*	info = &mInfo; Delete(info); }

					// Instance methods
		bool		hasValue() const
						{ return mCurrentRawValue != nil; }
		UInt32		getIndex() const
						{ return mIndex; }
		bool		advance()
						{
							// Advance
							mCurrentRawValue = (K**) mAdvanceProc(mInfo);
							mIndex++;

							return mCurrentRawValue != nil;
						}

		T			getValue() const
						{ return mMapProc(mCurrentRawValue); }

		T			operator*() const
						{ return mMapProc(mCurrentRawValue); }

	protected:
					// CIterator methods
		AdvanceProc	getAdvanceProc() const
						{ return mAdvanceProc; }
		Info&		getInfo() const
						{ return mInfo; }

	// Properties
	private:
		AdvanceProc	mAdvanceProc;
		Info&		mInfo;
		K**			mCurrentRawValue;
		T 			(*mMapProc)(K** rawValue);
		UInt32		mIndex;
};
