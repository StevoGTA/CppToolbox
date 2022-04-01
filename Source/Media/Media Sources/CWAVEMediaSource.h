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
		virtual	OI<CChunkReader>					setup(const I<CSeekableDataSource>& seekableDataSource);

		virtual	OI<SError>							note(const CChunkReader::ChunkInfo& chunkInfo,
															CChunkReader& chunkReader);
		virtual	bool								note(const SWAVEFORMAT& waveFormat, const OV<UInt16>& sampleSize,
															const CData& chunkPayload);

		virtual	bool								canFinalize() const;
		virtual	CAudioTrack							composeAudioTrack();
		virtual	I<CDecodeAudioCodec>				createAudioCodec(const I<CSeekableDataSource>& seekableDataSource);

													// Class methods
		static	I<CWAVEMediaSourceImportTracker>	instantiate();

	protected:
													// Subclass methods
		virtual	CAudioTrack							composeAudioTrack(UInt16 sampleSize);

	// Properties
	protected:
		OV<UInt64>								mDataChunkStartByteOffset;
		OV<UInt64>								mDataChunkByteCount;

	private:
		CWAVEMediaSourceImportTrackerInternals*	mInternals;
};
