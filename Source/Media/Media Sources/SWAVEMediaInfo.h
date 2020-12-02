//----------------------------------------------------------------------------------------------------------------------
//	SWAVEMediaInfo.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// Types

struct SWAVEChunkHeader32 {
	// Methods
	OSType	getNativeChunkID() { return EndianU32_BtoN(mChunkID); }
	UInt32	getNativeChunkSize() { return EndianU32_LtoN(mChunkSize); }

	// Properties
	OSType	mChunkID;
	UInt32	mChunkSize;
};

struct SWAVEFORMChunk32 {
	// Methods
	OSType	getNativeChunkID() { return EndianU32_BtoN(mChunkID); }
	UInt32	getNativeChunkSize() { return EndianU32_LtoN(mChunkSize); }
	OSType	getNativeFormType() { return EndianU32_BtoN(mFormType); }

	// Properties
	OSType	mChunkID;
	UInt32	mChunkSize;
	OSType	mFormType;
};

// From http://msdn.microsoft.com/en-us/library/windows/desktop/dd757712(v=vs.85).aspx
struct SWAVEFORMAT {
	// Methods
	OSType	getNativeChunkID() { return EndianU32_BtoN(mChunkID); }
	UInt32	getNativeChunkSize() { return EndianU32_LtoN(mChunkSize); }
	UInt16	getNativeFormatTag() { return EndianU16_LtoN(mFormatTag); }
	UInt16	getNativeChannels() { return EndianU16_LtoN(mChannels); }
	UInt32	getNativeSamplesPerSec() { return EndianU32_LtoN(mSamplesPerSec); }
	UInt32	getNativeAvgBytesPerSec() { return EndianU32_LtoN(mAvgBytesPerSec); }
	UInt16	getNativeBlockAlign() { return EndianU16_LtoN(mBlockAlign); }

	// Properties
	OSType	mChunkID;
	UInt32	mChunkSize;
	UInt16	mFormatTag;
	UInt16	mChannels;
	UInt32	mSamplesPerSec;
	UInt32	mAvgBytesPerSec;
	UInt16	mBlockAlign;
};

const	OSType	kWAVEFORMChunkID = MAKE_OSTYPE('R', 'I', 'F', 'F');
const	OSType	kWAVEFORMType = MAKE_OSTYPE('W', 'A', 'V', 'E');
const	OSType	kWAVEFormatChunkID = MAKE_OSTYPE('f', 'm', 't', ' ');
const	OSType	kWAVEDataChunkID = MAKE_OSTYPE('d', 'a', 't', 'a');
