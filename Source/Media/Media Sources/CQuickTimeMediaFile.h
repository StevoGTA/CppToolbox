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

#pragma pack(push,1)

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(disable:4200)
	#pragma warning(disable:4815)
#endif

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

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(default:4200)
	#pragma warning(default:4815)
#endif

#pragma pack(pop)

	// Methods
	public:
															// Lifecycle methods
															CQuickTimeMediaFile() {}
		virtual												~CQuickTimeMediaFile() {}

															// Instance methods
		virtual	I<SMediaSource::ImportResult>				import(const SMediaSource::ImportSetup& importSetup);
				TArray<SMedia::PacketAndLocation>			composePacketAndLocations(const Internals& internals,
																	const OV<UInt32>& framesPerPacket = OV<UInt32>(),
																	const OV<UInt32>& bytesPerPacket = OV<UInt32>())
																	const;

	protected:
															// Instance methods
		virtual	TVResult<SMediaSource::Tracks::AudioTrack>	composeAudioTrack(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	UInt32 options, OSType type,
																	UniversalTimeInterval duration,
																	const OV<CData>& metaAtomPayloadData,
																	const Internals& internals);
		virtual	TVResult<SMediaSource::Tracks::VideoTrack>	composeVideoTrack(
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
		virtual	void										processFileMetadata(const CData& metaAtomPayloadData) {}

															// Subclass methods
				Float32										getSampleRate(const Internals& internals) const;
				UInt8										getChannelCount(const Internals& internals) const;
				TVResult<CData>								getAudioDecompressionData(const Internals& internals) const;

	// Properties
	public:
		static	OSType	mID;
};
