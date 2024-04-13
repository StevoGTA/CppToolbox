//----------------------------------------------------------------------------------------------------------------------
//	CBits.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBits.h"

#include "CDictionary.h"
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
				Internals(UInt32 count, const CData& data) :
					TCopyOnWriteReferenceCountable(), mUsed(count)
					{
						// Setup
						mAvailable = (UInt32) data.getByteCount() * 8;
						mStorage = (UInt8*) ::malloc(mAvailable);
						::memcpy(mStorage, data.getBytePtr(), data.getByteCount());
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
CBits::CBits(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(info.getUInt32(CString(OSSTR("used"))),
					CData::fromBase64String(info.getString(CString(OSSTR("data")))));
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
CBits& CBits::set(UInt32 index, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	mInternals->set(index, value);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CBits::getInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	info;

	// Store
	info.set(CString(OSSTR("data")), CData(mInternals->mStorage, mInternals->mAvailable / 8, false).getBase64String());
	info.set(CString(OSSTR("used")), mInternals->mUsed);

	return info;
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
