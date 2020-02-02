//----------------------------------------------------------------------------------------------------------------------
//	CUUIDCFImplementation.cpp			©2009 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CUUID.h"

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: External functions

extern	SUUIDBytes	eCreateUUIDBytes();

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CUUIDInternals

class CUUIDInternals : public TReferenceCountable<CUUIDInternals>{
	public:
		CUUIDInternals() : TReferenceCountable(), mUUIDBytes(eCreateUUIDBytes()) {}
		CUUIDInternals(const SUUIDBytes& uuidBytes) : TReferenceCountable(), mUUIDBytes(uuidBytes) {}
		CUUIDInternals(const CData& data) : TReferenceCountable()
			{
				// Check if data is correct size
				AssertFailIf(data.getSize() != 16);

				// Check data size
				if (data.getSize() == 16)
					// Data is correct size
					data.copyBytes(&mUUIDBytes);
				else
					// Data is not correct size
					mUUIDBytes = eCreateUUIDBytes();
			}
		CUUIDInternals(const CString& string) : TReferenceCountable()
			{
				// Check length
				if (string.getLength() == 36) {
					// Hex string
					mUUIDBytes.mBytes[0] = string.getSubString(0, 2).getUInt8(16);
					mUUIDBytes.mBytes[1] = string.getSubString(2, 2).getUInt8(16);
					mUUIDBytes.mBytes[2] = string.getSubString(4, 2).getUInt8(16);
					mUUIDBytes.mBytes[3] = string.getSubString(6, 2).getUInt8(16);
					mUUIDBytes.mBytes[4] = string.getSubString(9, 2).getUInt8(16);
					mUUIDBytes.mBytes[5] = string.getSubString(11, 2).getUInt8(16);
					mUUIDBytes.mBytes[6] = string.getSubString(14, 2).getUInt8(16);
					mUUIDBytes.mBytes[7] = string.getSubString(16, 2).getUInt8(16);
					mUUIDBytes.mBytes[8] = string.getSubString(19, 2).getUInt8(16);
					mUUIDBytes.mBytes[9] = string.getSubString(21, 2).getUInt8(16);
					mUUIDBytes.mBytes[10] = string.getSubString(24, 2).getUInt8(16);
					mUUIDBytes.mBytes[11] = string.getSubString(26, 2).getUInt8(16);
					mUUIDBytes.mBytes[12] = string.getSubString(28, 2).getUInt8(16);
					mUUIDBytes.mBytes[13] = string.getSubString(30, 2).getUInt8(16);
					mUUIDBytes.mBytes[14] = string.getSubString(32, 2).getUInt8(16);
					mUUIDBytes.mBytes[15] = string.getSubString(34, 2).getUInt8(16);
				} else
					// Unknown
					mUUIDBytes = eCreateUUIDBytes();
			}

		SUUIDBytes	mUUIDBytes;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CUUID

CUUID	CUUID::mZero(CString("00000000-0000-0000-0000-000000000000"));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CUUIDInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID(const SUUIDBytes& uuidBytes)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CUUIDInternals(uuidBytes);
}

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CUUIDInternals(data);
}

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CUUIDInternals(string);
}

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID(const CUUID& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CUUID::~CUUID()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString CUUID::getHexString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CString::make("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			mInternals->mUUIDBytes.mBytes[0], mInternals->mUUIDBytes.mBytes[1], mInternals->mUUIDBytes.mBytes[2],
			mInternals->mUUIDBytes.mBytes[3], mInternals->mUUIDBytes.mBytes[4], mInternals->mUUIDBytes.mBytes[5],
			mInternals->mUUIDBytes.mBytes[6], mInternals->mUUIDBytes.mBytes[7], mInternals->mUUIDBytes.mBytes[8],
			mInternals->mUUIDBytes.mBytes[9], mInternals->mUUIDBytes.mBytes[10], mInternals->mUUIDBytes.mBytes[11],
			mInternals->mUUIDBytes.mBytes[12], mInternals->mUUIDBytes.mBytes[13], mInternals->mUUIDBytes.mBytes[14],
			mInternals->mUUIDBytes.mBytes[15]);
}

//----------------------------------------------------------------------------------------------------------------------
CData CUUID::getData() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CData(&mInternals->mUUIDBytes, sizeof(SUUIDBytes));
}

//----------------------------------------------------------------------------------------------------------------------
SUUIDBytes CUUID::getBytes()
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUUIDBytes;
}

//----------------------------------------------------------------------------------------------------------------------
bool CUUID::equals(const CUUID& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::memcmp(&mInternals->mUUIDBytes, &other.mInternals->mUUIDBytes, sizeof(SUUIDBytes)) == 0;
}
