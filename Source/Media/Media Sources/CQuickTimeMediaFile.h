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
				I<SMediaSource::ImportResult>				import(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	const OI<CAppleResourceManager>&
																			appleResourceManager,
																	TNArray<CString>& messages, UInt32 options);
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
																	UInt32 options, OSType type,
																	UniversalTimeInterval duration,
																	const OV<CData>& metaAtomPayloadData,
																	const Internals& internals);
		virtual	TVResult<CMediaTrackInfos::VideoTrackInfo>	composeVideoTrackInfo(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	UInt32 options, OSType type,
																	UInt32 timeScale, UniversalTimeInterval duration,
																	const OV<CData>& metaAtomPayloadData,
																	const Internals& internals);
		virtual	void										processFileMetadata(const CData& metaAtomPayloadData) {}

															// Subclass methods
				Float32										getSampleRate(const Internals& internals) const;
				UInt8										getChannels(const Internals& internals) const;
				TVResult<CData>								getAudioDecompressionData(const Internals& internals) const;
};
