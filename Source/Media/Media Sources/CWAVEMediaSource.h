//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioTrack.h"
#include "CChunkReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

const	OSType	kWAVEFORMChunkID = MAKE_OSTYPE('R', 'I', 'F', 'F');
const	OSType	kWAVEFORMType = MAKE_OSTYPE('W', 'A', 'V', 'E');
const	OSType	kWAVEFormatChunkID = MAKE_OSTYPE('f', 'm', 't', ' ');
const	OSType	kWAVEDataChunkID = MAKE_OSTYPE('d', 'a', 't', 'a');

//----------------------------------------------------------------------------------------------------------------------
// MARK: SWAVEFORMChunk32

#pragma pack(push,1)
struct SWAVEFORMChunk32 {
			// Methods
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
// MARK: SWAVEFORMAT

// From http://msdn.microsoft.com/en-us/library/windows/desktop/dd757712(v=vs.85).aspx
#pragma pack(push,1)
struct SWAVEFORMAT {
			// Methods
	OSType	getID() const { return EndianU32_BtoN(mID); }
	UInt32	getByteCount() const { return EndianU32_LtoN(mByteCount); }
	UInt16	getFormatTag() const { return EndianU16_LtoN(mFormatTag); }
	UInt16	getChannels() const { return EndianU16_LtoN(mChannels); }
	UInt32	getSamplesPerSec() const { return EndianU32_LtoN(mSamplesPerSec); }
	UInt32	getAvgBytesPerSec() const { return EndianU32_LtoN(mAvgBytesPerSec); }
	UInt16	getBlockAlign() const { return EndianU16_LtoN(mBlockAlign); }

	// Properties (in storage endian)
	private:
		OSType	mID;
		UInt32	mByteCount;
		UInt16	mFormatTag;
		UInt16	mChannels;
		UInt32	mSamplesPerSec;
		UInt32	mAvgBytesPerSec;
		UInt16	mBlockAlign;
};
#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaSourceImportTracker

class CWAVEMediaSourceImportTracker {
	// Methods
	public:
													// Lifecycle methods
													CWAVEMediaSourceImportTracker() {}
		virtual										~CWAVEMediaSourceImportTracker() {}

													// Instance methods
		virtual	bool								note(const SWAVEFORMAT& waveFormat) = 0;
		virtual	void								note(const CChunkReader::ChunkInfo& chunkInfo) {}

		virtual	bool								canFinalize() const = 0;
		virtual	CAudioTrack							composeAudioTrack(UInt64 dataChunkByteCount) = 0;
		virtual	I<CCodec::DecodeInfo>				composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
															UInt64 dataChunkStartByteOffset,
															UInt64 dataChunkByteCount) = 0;

													// Class methods
		static	I<CWAVEMediaSourceImportTracker>	instantiate();
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDefaultWAVEMediaSourceImportTracker

class CDefaultWAVEMediaSourceImportTrackerInternals;
class CDefaultWAVEMediaSourceImportTracker : public CWAVEMediaSourceImportTracker {
	// Methods
	public:
								// Lifecycle methods
								CDefaultWAVEMediaSourceImportTracker();
								~CDefaultWAVEMediaSourceImportTracker();

								// Instance methods
		bool					note(const SWAVEFORMAT& waveFormat);

		bool					canFinalize() const;
		CAudioTrack				composeAudioTrack(UInt64 dataChunkByteCount);
		I<CCodec::DecodeInfo>	composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
										UInt64 dataChunkStartByteOffset, UInt64 dataChunkByteCount);

	// Properties
	private:
		CDefaultWAVEMediaSourceImportTrackerInternals*	mInternals;
};
