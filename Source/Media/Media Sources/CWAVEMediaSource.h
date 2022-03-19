//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioTrack.h"
#include "CChunkReader.h"
#include "CMediaSourceRegistry.h"
#include "SWAVEInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaSource
class CWAVEMediaSource {
	// Methods
	public:
												// Class methods
		static	SMediaSource::QueryTracksResult	queryTracks(const I<CSeekableDataSource>& seekableDataSource,
														SMediaSource::Options options);

	// Properties
	public:
		static	CString	mErrorDomain;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaSourceImportTracker

class CWAVEMediaSourceImportTrackerInternals;
class CWAVEMediaSourceImportTracker {
	// Methods
	public:
													// Lifecycle methods
													CWAVEMediaSourceImportTracker();
		virtual										~CWAVEMediaSourceImportTracker();

													// Instance methods
		virtual	bool								note(const SWAVEFORMAT& waveFormat, const OV<UInt16>& sampleSize);
		virtual	void								note(const CChunkReader::ChunkInfo& chunkInfo) {}

		virtual	bool								canFinalize() const;
		virtual	CAudioTrack							composeAudioTrack(UInt16 sampleSize, UInt64 dataChunkByteCount);
		virtual	I<CCodec::DecodeInfo>				composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
															UInt64 dataChunkStartByteOffset, UInt64 dataChunkByteCount);

													// Class methods
		static	I<CWAVEMediaSourceImportTracker>	instantiate();

	// Properties
	private:
		CWAVEMediaSourceImportTrackerInternals*	mInternals;
};
