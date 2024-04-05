//----------------------------------------------------------------------------------------------------------------------
//	CBits.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBits.h"

#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBits::Internals

class CBits::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
				Internals(UInt32 count, bool initialValue) :
					TCopyOnWriteReferenceCountable(), mUsed(count)
					{
						// Setup
						UInt32	bytes = (count + 7) / 8;
						mAvailable = bytes * 8;
						mStorage = (UInt8*) ::calloc(bytes, 1);

						// Check initial value
						if (initialValue)
							// Set all bits
							::memset(mStorage, 0xFF, mUsed / 8);
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
CBits::CBits(UInt32 count, bool initialValue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(count, initialValue);
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

//----------------------------------------------------------------------------------------------------------------------
CBits& CBits::operator=(const CBits& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->removeReference();
	mInternals = other.mInternals->addReference();

	return *this;
}
