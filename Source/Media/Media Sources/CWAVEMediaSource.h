//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioTrack.h"
#include "CChunkReader.h"
#include "SWAVEInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaSourceImportTracker

class CWAVEMediaSourceImportTracker {
	// Methods
	public:
													// Lifecycle methods
													CWAVEMediaSourceImportTracker() {}
		virtual										~CWAVEMediaSourceImportTracker() {}

													// Instance methods
		virtual	bool								note(const SWAVEFORMAT& waveFormat, const OV<UInt16>& sampleSize)
															= 0;
		virtual	void								note(const CChunkReader::ChunkInfo& chunkInfo) {}

		virtual	bool								canFinalize() const = 0;
		virtual	CAudioTrack							composeAudioTrack(UInt16 sampleSize, UInt64 dataChunkByteCount) = 0;
		virtual	I<CCodec::DecodeInfo>				composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
															UInt64 dataChunkStartByteOffset,
															UInt64 dataChunkByteCount) = 0;

													// Class methods
		static	I<CWAVEMediaSourceImportTracker>	instantiate();
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDefaultWAVEMediaSourceImportTracker

class CDefaultWAVEMediaSourceImportTrackerInternals;
class CDefaultWAVEMediaSourceImportTracker : public CWAVEMediaSourceImportTracker {
	// Methods
	public:
								// Lifecycle methods
								CDefaultWAVEMediaSourceImportTracker();
								~CDefaultWAVEMediaSourceImportTracker();

								// Instance methods
		bool					note(const SWAVEFORMAT& waveFormat, const OV<UInt16>& sampleSize);

		bool					canFinalize() const;
		CAudioTrack				composeAudioTrack(UInt16 sampleSize, UInt64 dataChunkByteCount);
		I<CCodec::DecodeInfo>	composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
										UInt64 dataChunkStartByteOffset, UInt64 dataChunkByteCount);

	// Properties
	private:
		CDefaultWAVEMediaSourceImportTrackerInternals*	mInternals;
};
