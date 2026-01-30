//----------------------------------------------------------------------------------------------------------------------
//	SMPEG4.cpp			©2025 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SMPEG4.h"

#include "CAtom.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

struct SMPEG4AudioFormatAtom {
	// Methods
	public:
				// Instance methods
		UInt32	getLength() const
					{ return EndianU32_BtoN(mLength); }
		OSType	getType() const
					{ return EndianU32_BtoN(mType); }
		OSType	getFormat() const
					{ return EndianU32_BtoN(mFormat); }

	// Properties (in storage endian)
	private:
		UInt32	mLength;
		OSType	mType;
		OSType	mFormat;
};

struct SMPEG4MP4AESDSAtomPayloadHeader {
	// Methods
	public:
				// Lifecycle methods
				SMPEG4MP4AESDSAtomPayloadHeader() :
					mVersion(0)
					{
						// Setup
						mFlags[0] = 0;
						mFlags[1] = 0;
						mFlags[2] = 0;
					}

				// Instance methods
		CData	getData() const
					{ return CData(this, sizeof(SMPEG4MP4AESDSAtomPayloadHeader), false); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;	// 0
		UInt8	mFlags[3];
};

static	CString	sErrorDomain(OSSTR("SMPEG4"));
static	SError	sInvalidMagicCookieError(sErrorDomain, 1, CString(OSSTR("Invalid Magic Cookie")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SMPEG4::STSDAtomPayload

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CData SMPEG4::STSDAtomPayload::getData(const TArray<CData>& descriptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
#if defined(TARGET_OS_WINDOWS)
	#pragma warning(disable:4815)
#endif

	STSDAtomPayload	stsdAtomPayload(descriptions.getCount());

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(default:4815)
#endif

	CData			data(&stsdAtomPayload, sizeof(STSDAtomPayload), false);

	// Add descriptions
	for (TIteratorD<CData> iterator = descriptions.getIterator(); iterator.hasValue(); iterator.advance())
		// Add description
		data += *iterator;

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SMPEG4::STSDMP4ADescription

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> SMPEG4::STSDMP4ADescription::getData(Float32 sampleRate,
		const SAudio::ChannelMap& audioChannelMap, UInt16 dataRefIndex, const CData& magicCookie)
//----------------------------------------------------------------------------------------------------------------------
{
	// Decode Magic Cookie
	const	UInt8*					constBytePtr = (const UInt8*) magicCookie.getBytePtr();
			CData::ByteCount		byteCount = magicCookie.getByteCount();
	const	SMPEG4AudioFormatAtom&	audioFormatAtom = *((const SMPEG4AudioFormatAtom*) constBytePtr);
			CData					magicCookieData;
	if ((byteCount >= sizeof(SMPEG4AudioFormatAtom)) &&
			(audioFormatAtom.getLength() == 12) &&
			(audioFormatAtom.getType() == MAKE_OSTYPE('f', 'r', 'm', 'a')) &&
			(audioFormatAtom.getFormat() == MAKE_OSTYPE('m', 'p', '4', 'a'))) {
		// We have an old-style magic cookie in full atom form
		// Skip the atom header
		constBytePtr += audioFormatAtom.getLength();
		byteCount -= audioFormatAtom.getLength();

		// Extract just the part we need
		const	CAtom::Header&	mp4aAtomHeader = *((const CAtom::Header*) constBytePtr);
		if ((byteCount < sizeof(CAtom::Header)) || (mp4aAtomHeader.getType() != MAKE_OSTYPE('m', 'p', '4', 'a')))
			return TVResult<CData>(sInvalidMagicCookieError);

		// Almost there...
		constBytePtr += mp4aAtomHeader.getLength();

		// Sanity check
		const	CAtom::Header&	esdsAtomHeader = *((const CAtom::Header*) constBytePtr);
		if (esdsAtomHeader.getType() != MAKE_OSTYPE('e', 's', 'd', 's'))
			return TVResult<CData>(sInvalidMagicCookieError);

		// Copy magic cookie
		magicCookieData = CData(constBytePtr, esdsAtomHeader.getLength());
	} else
		// New style magic cookie
//		magicCookieData +=
//				CAtom::Header(sizeof(CAtom::Header) + sizeof(SMPEG4MP4AESDSAtomPayloadHeader) + (UInt32) byteCount,
//								MAKE_OSTYPE('e', 's', 'd', 's'))
//						.getData() +
//				SMPEG4MP4AESDSAtomPayloadHeader().getData() +
//				magicCookie;

		magicCookieData +=
				CAtom(MAKE_OSTYPE('e', 's', 'd', 's'), SMPEG4MP4AESDSAtomPayloadHeader().getData() + magicCookie);

	return STSDMP4ADescription(sizeof(STSDMP4ADescription) + (UInt32) magicCookieData.getByteCount(), sampleRate,
					audioChannelMap, dataRefIndex).getData() +
			magicCookieData;
}
