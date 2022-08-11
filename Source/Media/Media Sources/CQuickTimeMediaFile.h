//----------------------------------------------------------------------------------------------------------------------
//	CQuickTimeMediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAtomReader.h"
#include "CMediaSourceRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CQuickTimeMediaFile

class CQuickTimeMediaFile {
	// Internals
	public:
		struct Internals;

	// Methods
	public:
															// Lifecycle methods
		virtual												~CQuickTimeMediaFile() {}

															// Instance methods
				SMediaSource::ImportResult					import(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	const OI<CAppleResourceManager>&
																			appleResourceManager,
																	SMediaSource::Options options);
				TArray<SMediaPacketAndLocation>				composePacketAndLocations(const Internals& internals,
																	const OV<UInt32>& framesPerPacket = OV<UInt32>(),
																	const OV<UInt32>& bytesPerPacket = OV<UInt32>())
																	const;

															// Class methods
		static	I<CQuickTimeMediaFile>						create();

	protected:
															// Lifecycle methods
															CQuickTimeMediaFile() {}

															// Instance methods
		virtual	TVResult<CMediaTrackInfos::AudioTrackInfo>	composeAudioTrackInfo(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	SMediaSource::Options options, OSType type,
																	UniversalTimeInterval duration,
																	const Internals& internals);
		virtual	TVResult<CMediaTrackInfos::VideoTrackInfo>	composeVideoTrackInfo(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	SMediaSource::Options options, OSType type,
																	UInt32 timeScale, UniversalTimeInterval duration,
																	const Internals& internals);

															// Subclass methods
				Float32										getSampleRate(const Internals& internals) const;
				UInt8										getChannels(const Internals& internals) const;
				TIResult<CData>								getAudioDecompressionData(const Internals& internals) const;
};
