//----------------------------------------------------------------------------------------------------------------------
//	CBits.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBits.h"

#include "TReferenceTracking.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBits::Internals

class CBits::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
				Internals(UInt32 count) :
					TCopyOnWriteReferenceCountable(), mUsed(count)
					{
						// Setup
						UInt32	bytes = (count + 7) / 8;
						mAvailable = bytes * 8;
						mStorage = (UInt8*) ::calloc(bytes, 1);
					}
				Internals(const Internals& other) :
					TCopyOnWriteReferenceCountable(), mAvailable(other.mAvailable), mUsed(other.mUsed)
					{
						// Copy storage
						mStorage = (UInt8*) ::malloc(mAvailable / 8);
						::memcpy(mStorage, other.mStorage, mAvailable / 8);
					}
				~Internals()
					{
						free(mStorage);
					}

		void	set(UInt32 index, bool value)
					{
						// Update storage if necessary
						if (index >= mAvailable) {
							// Resize
							UInt32	bytes = (index + 8) / 8;
							mAvailable = bytes * 8;
							mStorage = (UInt8*) ::realloc(mStorage, bytes);
						}

						// Setup
						UInt32	byteIndex = index / 8;
						UInt32	bitIndex = index % 8;

						if (value)
							// Set
							mStorage[byteIndex] |= 1 << bitIndex;
						else
							// Clear
							mStorage[byteIndex] &= ~(1 << bitIndex);
					}

		UInt8*	mStorage;
		UInt32	mAvailable;
		UInt32	mUsed;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBits

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CBits::CBits(UInt32 count)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(count);
}

//----------------------------------------------------------------------------------------------------------------------
CBits::CBits(const CBits& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CBits::~CBits()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CBits::getCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUsed;
}

//----------------------------------------------------------------------------------------------------------------------
bool CBits::get(UInt32 index) const
//----------------------------------------------------------------------------------------------------------------------
{
	UInt32	byteIndex = index / 8;
	UInt32	bitIndex = index % 8;

	return mInternals->mStorage[byteIndex] & (1 << bitIndex);
}

//----------------------------------------------------------------------------------------------------------------------
void CBits::set(UInt32 index, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(index, value);
}
