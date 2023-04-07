//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAtomReader.h"
#include "SMediaSource.h"

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
		virtual			I<SMediaSource::ImportResult>				import(
																			const SMediaSource::ImportSetup&
																					importSetup);
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
																			UInt32 options, OSType type,
																			UniversalTimeInterval duration,
																			const Internals& internals);
		virtual			TVResult<CMediaTrackInfos::VideoTrackInfo>	composeVideoTrackInfo(
																			const I<CRandomAccessDataSource>&
																					randomAccessDataSource,
																			UInt32 options, OSType type,
																			UInt32 timeScale,
																			UniversalTimeInterval duration,
																			const Internals& internals);
		virtual			void										process(const CAtomReader& atomReader,
																			const CAtomReader::Atom& atom) {}

																	// Subclass methods
				const	void*										getSampleDescription(const Internals& internals)
																			const;
						TVResult<CData>								getDecompressionData(const Internals& internals,
																			SInt64 offset) const;
};
