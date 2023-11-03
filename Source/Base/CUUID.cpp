//----------------------------------------------------------------------------------------------------------------------
//	CUUIDCFImplementation.cpp			©2009 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CUUID.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: External functions

extern	CUUID::Bytes	eCreateUUIDBytes();

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CUUID::Internals

class CUUID::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals() : TReferenceCountableAutoDelete(), mUUIDBytes(eCreateUUIDBytes()) {}
		Internals(const CUUID::Bytes& uuidBytes) : TReferenceCountableAutoDelete(), mUUIDBytes(uuidBytes) {}
		Internals(const CData& data) : TReferenceCountableAutoDelete()
			{
				// Check if data is correct size
				AssertFailIf(data.getByteCount() != 16);

				// Check data size
				if (data.getByteCount() == 16)
					// Data is correct size
					data.copyBytes(&mUUIDBytes);
				else
					// Data is not correct size
					mUUIDBytes = eCreateUUIDBytes();
			}
		Internals(const CString& string) : TReferenceCountableAutoDelete()
			{
				// Check length
				if (string.getLength() == 36) {
					// Hex string
					//	Note only version 1 strings are supported currently
					*((UInt32*) &mUUIDBytes.mBytes[0]) = EndianU32_NtoB(string.getSubString(0, 8).getUInt32(16));
					*((UInt16*) &mUUIDBytes.mBytes[4]) = EndianU16_NtoB(string.getSubString(9, 4).getUInt16(16));
					*((UInt16*) &mUUIDBytes.mBytes[6]) = EndianU16_NtoB(string.getSubString(14, 4).getUInt16(16));
					*((UInt16*) &mUUIDBytes.mBytes[8]) = EndianU16_NtoB(string.getSubString(19, 4).getUInt16(16));
					*((UInt32*) &mUUIDBytes.mBytes[10]) = EndianU32_NtoB(string.getSubString(24, 8).getUInt32(16));
					*((UInt16*) &mUUIDBytes.mBytes[14]) = EndianU16_NtoB(string.getSubString(32, 4).getUInt16(16));
				} else
					// Unknown
					::memcpy(&mUUIDBytes, "-Unknown Format-", 16);
			}

		CUUID::Bytes	mUUIDBytes;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CUUID

const	CUUID	CUUID::mZero(CString(OSSTR("00000000-0000-0000-0000-000000000000")));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID(const Bytes& bytes)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(bytes);
}

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(data);
}

//----------------------------------------------------------------------------------------------------------------------
CUUID::CUUID(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(string);
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
	return CString::make(OSSTR("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
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
	return CData(&mInternals->mUUIDBytes, sizeof(Bytes));
}

//----------------------------------------------------------------------------------------------------------------------
const CUUID::Bytes& CUUID::getBytes() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUUIDBytes;
}

//----------------------------------------------------------------------------------------------------------------------
bool CUUID::equals(const CUUID& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::memcmp(&mInternals->mUUIDBytes, &other.mInternals->mUUIDBytes, sizeof(Bytes)) == 0;
}
