//----------------------------------------------------------------------------------------------------------------------
//	SWAVEInfo.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

const	OSType	kWAVEFORMChunkID = MAKE_OSTYPE('R', 'I', 'F', 'F');
const	OSType	kWAVEFORMType = MAKE_OSTYPE('W', 'A', 'V', 'E');

const	OSType	kWAVEFormatChunkID = MAKE_OSTYPE('f', 'm', 't', ' ');

const	OSType	kWAVEDataChunkID = MAKE_OSTYPE('d', 'a', 't', 'a');

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SWAVEFORMChunk32

#pragma pack(push, 1)

struct SWAVEFORMChunk32 {
	// Methods
	public:
				// Lifecycle methods
				SWAVEFORMChunk32(OSType id, UInt32 byteCount, OSType formType) :
					mID(EndianU32_NtoB(id)), mByteCount(EndianU32_NtoL(byteCount)), mFormType(EndianU32_NtoB(formType))
					{}

				// Instance methods
		OSType	getID() const { return EndianU32_BtoN(mID); }
		UInt32	getByteCount() const { return EndianU32_LtoN(mByteCount); }
		OSType	getFormType() const { return EndianU32_BtoN(mFormType); }

	// Properties (in storage endian)
	private:
		OSType	mID;
		UInt32	mByteCount;
		OSType	mFormType;
};

#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SWAVEFORMAT

// From http://msdn.microsoft.com/en-us/library/windows/desktop/dd757712(v=vs.85).aspx
#pragma pack(push, 1)

struct SWAVEFORMAT {
			// Lifecycle methods
			SWAVEFORMAT(UInt16 formatTag, UInt16 channelCount, UInt32 samplesPerSecond, UInt32 averageBytesPerSecond,
					UInt16 blockAlign) :
				mFormatTag(EndianU16_NtoL(formatTag)), mChannelCount(EndianU16_LtoN(channelCount)),
						mSamplesPerSecond(EndianU32_NtoL(samplesPerSecond)),
						mAverageBytesPerSecond(EndianU32_NtoL(averageBytesPerSecond)),
						mBlockAlign(EndianU16_NtoL(blockAlign))
				{}

			// Instance methods
	UInt16	getFormatTag() const { return EndianU16_LtoN(mFormatTag); }
	UInt16	getChannelCount() const { return EndianU16_LtoN(mChannelCount); }
	UInt32	getSamplesPerSecond() const { return EndianU32_LtoN(mSamplesPerSecond); }
	UInt32	getAverageBytesPerSec() const { return EndianU32_LtoN(mAverageBytesPerSecond); }
	UInt16	getBlockAlign() const { return EndianU16_LtoN(mBlockAlign); }

	// Properties (in storage endian)
	private:
		UInt16	mFormatTag;
		UInt16	mChannelCount;
		UInt32	mSamplesPerSecond;
		UInt32	mAverageBytesPerSecond;
		UInt16	mBlockAlign;
};

#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SWAVEFORMATEX

// From http://msdn.microsoft.com/en-us/library/windows/desktop/dd757712(v=vs.85).aspx
#pragma pack(push, 1)

struct SWAVEFORMATEX {
	// Methods
	public:
				// Lifecycle methods
				SWAVEFORMATEX(UInt16 formatTag, UInt8 bits, Float32 sampleRate, UInt8 channelCount,
						UInt16 blockAlign, UInt16 bitsPerSample, UInt16 additionalInfoByteCount) :
					mFormatTag(EndianU16_NtoL(formatTag)), mChannelCount(EndianU16_NtoL(channelCount)),
							mSamplesPerSec(EndianU32_NtoL((UInt32) (sampleRate + 0.5))),
							mAverageBytesPerSecond(EndianU32_NtoL(mSamplesPerSec * channelCount * bitsPerSample / 8)),
							mBlockAlign(EndianU16_NtoL(blockAlign)),
							mBitsPerSample(EndianU16_NtoL(bitsPerSample)),
							mAdditionalInfoByteCount(EndianU16_NtoL(additionalInfoByteCount))
					{}

				// Instance methods
		UInt16	getFormatTag() const { return EndianU16_LtoN(mFormatTag); }
		UInt16	getChannelCount() const { return EndianU16_LtoN(mChannelCount); }
		UInt32	getSamplesPerSec() const { return EndianU32_LtoN(mSamplesPerSec); }
		UInt32	getAverageBytesPerSec() const { return EndianU32_LtoN(mAverageBytesPerSecond); }
		UInt16	getBlockAlign() const { return EndianU16_LtoN(mBlockAlign); }
		UInt16	getBitsPerSample() const { return EndianU16_LtoN(mBitsPerSample); }
		UInt16	getAdditionalInfoByteCount() const { return EndianU16_LtoN(mAdditionalInfoByteCount); }

	// Properties (in storage endian)
	private:
		UInt16	mFormatTag;
		UInt16	mChannelCount;
		UInt32	mSamplesPerSec;
		UInt32	mAverageBytesPerSecond;
		UInt16	mBlockAlign;
		UInt16	mBitsPerSample;
		UInt16	mAdditionalInfoByteCount;
};

#pragma pack(pop)
