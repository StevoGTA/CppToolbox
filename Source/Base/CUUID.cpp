//----------------------------------------------------------------------------------------------------------------------
//	CUUIDCFImplementation.cpp			©2009 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CUUID.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CUUID::Internals

class CUUID::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals() : TReferenceCountableAutoDelete()
			{
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
				// Create new UUID and get raw bytes
				CFUUIDRef	uuidRef = ::CFUUIDCreate(kCFAllocatorDefault);
				CFUUIDBytes	uuidBytes = ::CFUUIDGetUUIDBytes(uuidRef);
				::CFRelease(uuidRef);

				::memcpy(mBytes, &uuidBytes, sizeof(Bytes));
#elif defined(TARGET_OS_WINDOWS)
#endif
			}
		Internals(const Bytes& uuidBytes) :
			TReferenceCountableAutoDelete()
			{
				::memcpy(mBytes, uuidBytes, sizeof(Bytes));
			}
		Internals(const CData& data) : TReferenceCountableAutoDelete()
			{
				// Check if data is correct size
				AssertFailIf(data.getByteCount() != 16);

				// Check data size
				if (data.getByteCount() == 16)
					// Data is correct size
					data.copyBytes(&mBytes, 0, 16);
				else
					// Data is not correct size
					::memcpy(&mBytes, "-Invalid data-", 14);
			}
		Internals(const CString& string) : TReferenceCountableAutoDelete()
			{
				// Check length
				if (string.getLength() == 36) {
					// Hex string
					//	Note only version 1 strings are supported currently
					*((UInt32*) &mBytes[0]) = EndianU32_NtoB(string.getSubString(0, 8).getUInt32(16));
					*((UInt16*) &mBytes[4]) = EndianU16_NtoB(string.getSubString(9, 4).getUInt16(16));
					*((UInt16*) &mBytes[6]) = EndianU16_NtoB(string.getSubString(14, 4).getUInt16(16));
					*((UInt16*) &mBytes[8]) = EndianU16_NtoB(string.getSubString(19, 4).getUInt16(16));
					*((UInt32*) &mBytes[10]) = EndianU32_NtoB(string.getSubString(24, 8).getUInt32(16));
					*((UInt16*) &mBytes[14]) = EndianU16_NtoB(string.getSubString(32, 4).getUInt16(16));
				} else
					// Unknown
					::memcpy(&mBytes, "-Unknown Format-", 16);
			}

		Bytes	mBytes;
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
			mInternals->mBytes[0], mInternals->mBytes[1], mInternals->mBytes[2],
			mInternals->mBytes[3], mInternals->mBytes[4], mInternals->mBytes[5],
			mInternals->mBytes[6], mInternals->mBytes[7], mInternals->mBytes[8],
			mInternals->mBytes[9], mInternals->mBytes[10], mInternals->mBytes[11],
			mInternals->mBytes[12], mInternals->mBytes[13], mInternals->mBytes[14],
			mInternals->mBytes[15]);
}

//----------------------------------------------------------------------------------------------------------------------
CData CUUID::getData() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CData(&mInternals->mBytes, sizeof(Bytes));
}

//----------------------------------------------------------------------------------------------------------------------
void CUUID::getBytes(Bytes& bytes) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Copy
	::memcpy(bytes, mInternals->mBytes, sizeof(Bytes));
}

//----------------------------------------------------------------------------------------------------------------------
bool CUUID::equals(const CUUID& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::memcmp(&mInternals->mBytes, &other.mInternals->mBytes, sizeof(Bytes)) == 0;
}
