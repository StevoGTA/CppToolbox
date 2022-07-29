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
		static	SMediaSource::QueryTracksResult	queryTracks(const I<CRandomAccessDataSource>& randomAccessDataSource,
														const OI<CAppleResourceManager>& appleResourceManager,
														SMediaSource::Options options);

	// Properties
	public:
		static	const	CString	mErrorDomain;
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
		virtual	OI<CChunkReader>					setup(const I<CRandomAccessDataSource>& randomAccessDataSource);

		virtual	OI<SError>							note(const CChunkReader::ChunkInfo& chunkInfo,
															CChunkReader& chunkReader);
		virtual	void								note(const SWAVEFORMAT& waveFormat, const OV<UInt16>& sampleSize,
															const CData& chunkPayload);

		virtual	bool								canFinalize(
															const I<CRandomAccessDataSource>& randomAccessDataSource);
		virtual	CAudioTrack							composeAudioTrack();
		virtual	OI<I<CDecodeAudioCodec> >			createAudioCodec(
															const I<CRandomAccessDataSource>& randomAccessDataSource);

													// Class methods
		static	I<CWAVEMediaSourceImportTracker>	instantiate();

	protected:
													// Subclass methods
		virtual	CAudioTrack							composeAudioTrack(UInt16 sampleSize);

	// Properties
	protected:
		OV<UInt64>								mDataChunkStartByteOffset;
		OV<UInt64>								mDataChunkByteCount;
		OV<UInt16>								mFormatTag;
		OI<SAudioStorageFormat>					mAudioStorageFormat;

	private:
		CWAVEMediaSourceImportTrackerInternals*	mInternals;
};
