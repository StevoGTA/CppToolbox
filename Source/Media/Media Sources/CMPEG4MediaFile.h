//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAtomReader.h"
#include "CMediaSourceRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMPEG4MediaFile

class CMPEG4MediaFile {
	// Internals
	public:
		struct Internals;

	// Methods
	public:
																	// Lifecycle methods
		virtual														~CMPEG4MediaFile() {}

																	// Instance methods
						SMediaSource::QueryTracksResult				queryTracks(
																			const I<CRandomAccessDataSource>&
																					randomAccessDataSource,
																			const OI<CAppleResourceManager>&
																					appleResourceManager,
																			SMediaSource::Options options);
						TArray<SMediaPacketAndLocation>				composePacketAndLocations(
																			const Internals& internals) const;

																	// Class methods
		static			I<CMPEG4MediaFile>							create();

	protected:
																	// Lifecycle methods
																	CMPEG4MediaFile() {}

																	// Instance methods
		virtual			TVResult<CMediaTrackInfos::AudioTrackInfo>	composeAudioTrackInfo(
																			const I<CRandomAccessDataSource>&
																					randomAccessDataSource,
																			SMediaSource::Options options, OSType type,
																			UniversalTimeInterval duration,
																			const Internals& internals);
		virtual			TVResult<CMediaTrackInfos::VideoTrackInfo>	composeVideoTrackInfo(
																			const I<CRandomAccessDataSource>&
																					randomAccessDataSource,
																			SMediaSource::Options options, OSType type,
																			UInt32 timeScale,
																			UniversalTimeInterval duration,
																			const Internals& internals);

																	// Subclass methods
				const	void*										getSampleDescription(const Internals& internals)
																			const;
						TIResult<CData>								getDecompressionData(const Internals& internals,
																			SInt64 sampleDescriptionByteCount) const;
};
