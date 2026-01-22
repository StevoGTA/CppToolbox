//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAtomReader.h"
#include "SMediaSource.h"
#include "SMPEG4.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMPEG4MediaFile

class CMPEG4MediaFile {
	// Internals
	public:
		struct Internals;

	// Methods
	public:
															// Lifecycle methods
															CMPEG4MediaFile() {}
		virtual												~CMPEG4MediaFile() {}

															// Instance methods
		virtual	I<SMediaSource::ImportResult>				import(const SMediaSource::ImportSetup& importSetup);
				TArray<SMedia::PacketAndLocation>			composePacketAndLocations(const Internals& internals) const;

	protected:
															// Instance methods
		virtual	TVResult<SMediaSource::Tracks::AudioTrack>	composeAudioTrack(
																	const SMediaSource::ImportSetup& importSetup,
																	OSType type, UniversalTimeInterval duration,
																	const Internals& internals);
		virtual	TVResult<SMediaSource::Tracks::VideoTrack>	composeVideoTrack(
																	const SMediaSource::ImportSetup& importSetup,
																	OSType type, UInt32 timeScale,
																	UniversalTimeInterval duration,
																	const Internals& internals);
		virtual	OV<SError>									importTrack(const SMediaSource::ImportSetup& importSetup,
																	OSType type,
																	const SMPEG4::STSDDescriptionHeader&
																			stsdDescriptionHeader,
																	const Internals& internals)
																{ return OV<SError>(); }
		virtual	void										processFileMetadata(const CData& metaAtomPayloadData) {}

															// Subclass methods
				TVResult<CData>								getDecompressionData(const Internals& internals,
																	SInt64 offset) const;

	// Properties
	public:
		static	OSType	mID;
};
