//----------------------------------------------------------------------------------------------------------------------
//	CQuickTimeMediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAtomReader.h"
#include "SMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CQuickTimeMediaFile

class CQuickTimeMediaFile {
	// Internals
	public:
		struct Internals;

	// SstsdDescription
	public:
		struct SstsdDescription {
				// Methods
						OSType		getType() const
										{ return EndianU32_BtoN(mType); }
				const	CData		getSampleDescriptionPayload() const
										{ return CData(&mPayload, EndianU32_BtoN(mLength) - sizeof(SstsdDescription),
												false); }

			// Properties (in storage endian)
			private:
				UInt32	mLength;
				OSType	mType;
				UInt8	mReserved[6];
				UInt16	mDataRefIndex;
				UInt8	mPayload[];
		};

	// Methods
	public:
															// Lifecycle methods
		virtual												~CQuickTimeMediaFile() {}

															// Instance methods
		virtual	I<SMediaSource::ImportResult>				import(const SMediaSource::ImportSetup& importSetup);
				TArray<SMedia::PacketAndLocation>			composePacketAndLocations(const Internals& internals,
																	const OV<UInt32>& framesPerPacket = OV<UInt32>(),
																	const OV<UInt32>& bytesPerPacket = OV<UInt32>())
																	const;

	protected:
															// Lifecycle methods
															CQuickTimeMediaFile() {}

															// Instance methods
		virtual	void										processFileMetadata(const CData& metaAtomPayloadData) {}
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
		virtual	OV<SError>									importTrack(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	OSType type,
																	const SstsdDescription& stsdDescription,
																	const Internals& internals)
																{ return OV<SError>(); }

															// Subclass methods
				Float32										getSampleRate(const Internals& internals) const;
				UInt8										getChannels(const Internals& internals) const;
				TVResult<CData>								getAudioDecompressionData(const Internals& internals) const;

	// Properties
	public:
		static	OSType	mID;
};
