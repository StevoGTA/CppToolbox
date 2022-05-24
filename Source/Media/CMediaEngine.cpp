//----------------------------------------------------------------------------------------------------------------------
//	CMediaEngine.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaEngine.h"

#include "CAudioChannelMapper.h"
#include "CAudioDecoder.h"
#include "CCodecRegistry.h"
#include "CVideoDecoder.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local types
struct SAudioProcessingFormats {
			// Lifecycle Methods
			SAudioProcessingFormats(const SAudioProcessingFormat& sourceAudioProcessingFormat,
					const SAudioProcessingFormat& destinationAudioProcessingFormat) :
				mSourceAudioProcessingFormat(sourceAudioProcessingFormat),
						mDestinationAudioProcessingFormat(destinationAudioProcessingFormat)
				{}

			// Instance methods
	bool	doBitsMatch() const
				{ return mSourceAudioProcessingFormat.getBits() == mDestinationAudioProcessingFormat.getBits(); }
	bool	doSampleRatesMatch() const
				{ return mSourceAudioProcessingFormat.getSampleRate() ==
						mDestinationAudioProcessingFormat.getSampleRate(); }
	bool	doChannelMapsMatch() const
				{ return mSourceAudioProcessingFormat.getChannelMap() ==
						mDestinationAudioProcessingFormat.getChannelMap(); }
	bool	doSampleTypesMatch() const
				{ return mSourceAudioProcessingFormat.getSampleType() ==
						mDestinationAudioProcessingFormat.getSampleType(); }
	bool	doEndiansMatch() const
				{ return mSourceAudioProcessingFormat.getEndian() == mDestinationAudioProcessingFormat.getEndian(); }
	bool	doInterleavedsMatch() const
				{ return mSourceAudioProcessingFormat.getInterleaved() ==
						mDestinationAudioProcessingFormat.getInterleaved(); }

	// Properties
	SAudioProcessingFormat	mSourceAudioProcessingFormat;
	SAudioProcessingFormat	mDestinationAudioProcessingFormat;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	OI<SAudioProcessingFormat>	sDetermineCommonAudioProcessingFormat(
											const TArray<SAudioProcessingSetup>& sourceAudioProcessingSetups,
											const TArray<SAudioProcessingSetup>& destinationAudioProcessingSetups,
											const SAudioProcessingFormat& audioProcessingFormat);
static	SAudioProcessingFormats		sComposeAudioProcessingFormats(
											const SAudioProcessingSetup& sourceAudioProcessingSetup,
											const SAudioProcessingSetup& destinationAudioProcessingSetup,
											const SAudioProcessingFormat& audioProcessingFormat);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaEngine

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
I<CAudioSource> CMediaEngine::getAudioSource(const CMediaTrackInfos::AudioTrackInfo& audioTrackInfo,
		const CString& identifier) const
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CAudioSource>(
			new CAudioDecoder(audioTrackInfo.mMediaTrack.getAudioStorageFormat(), *audioTrackInfo.mCodec, identifier));
}

//----------------------------------------------------------------------------------------------------------------------
I<CVideoSource> CMediaEngine::getVideoSource(const CMediaTrackInfos::VideoTrackInfo& videoTrackInfo,
		CVideoFrame::Compatibility compatibility, const CString& identifier) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SVideoStorageFormat&	videoStorageFormat = videoTrackInfo.mMediaTrack.getVideoStorageFormat();

	return I<CVideoSource>(
			new CVideoDecoder(videoStorageFormat, *videoTrackInfo.mCodec,
					SVideoProcessingFormat(videoStorageFormat.getFrameSize(), videoStorageFormat.getFramerate(),
							compatibility),
					identifier));
}

//----------------------------------------------------------------------------------------------------------------------
SAudioProcessingFormat CMediaEngine::composeAudioProcessingFormat(const CAudioSource& audioSource,
		const CAudioDestination& audioDestination, const OV<Float32>& processingSampleRate) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Bits
	//	Use highest of source or destination
	// Sample Rate
	//	Use highest of source or destination
	// Channel Map
	//	change at source
	// Sample Type
	//	Prefer processing as Float32 if either source or destination specifies
	// Endian
	//	Prefer native endian.  If either source or destination is not native endian, then convert to native at that end
	// Interleaved
	//	Use destination

	// Setup
			TArray<SAudioProcessingSetup>	audioSourceAudioProcessingSetups = audioSource.getOutputSetups();
	const	SAudioProcessingSetup&			audioSourceFirstAudioProcessingSetup =
													audioSourceAudioProcessingSetups.getFirst();

			TArray<SAudioProcessingSetup>	audioDestinationProcessingSetups = audioDestination.getInputSetups();
	const	SAudioProcessingSetup&			audioDestinationFirstAudioProcessingSetup =
													audioDestinationProcessingSetups.getFirst();

	// Compose bits
	UInt8	audioSourceBits = audioSourceFirstAudioProcessingSetup.getBitsInfo().getValue();
	UInt8	bits;
	if (audioDestinationFirstAudioProcessingSetup.getBitsInfo().getOption() ==
			SAudioProcessingSetup::BitsInfo::kUnchanged)
		// Use first source setup
		bits = audioSourceBits;
	else
		// Use highest of source and destinaion
		bits = std::max<UInt8>(audioSourceBits, audioDestinationFirstAudioProcessingSetup.getBitsInfo().getValue());

	// Compose sample rate
	Float32	sampleRate;
	if (processingSampleRate.hasValue())
		// We have been given a specific value
		sampleRate = *processingSampleRate;
	else {
		// Setup
		Float32	audioSourceSampleRate = audioSourceFirstAudioProcessingSetup.getSampleRateInfo().getValue();
		if (audioDestinationFirstAudioProcessingSetup.getSampleRateInfo().getOption() ==
				SAudioProcessingSetup::SampleRateInfo::kUnchanged)
			// Use first source setup
			sampleRate = audioSourceSampleRate;
		else
			// Use highest of source and destination
			sampleRate =
					std::max<Float32>(audioSourceSampleRate,
							audioDestinationFirstAudioProcessingSetup.getSampleRateInfo().getValue());
	}

	// Compose channel map
	EAudioChannelMap	audioSourceChannelMap =
								audioSourceFirstAudioProcessingSetup.getChannelMapInfo().getValue();
	EAudioChannelMap	channelMap;
	if (audioDestinationFirstAudioProcessingSetup.getChannelMapInfo().getOption() ==
			SAudioProcessingSetup::ChannelMapInfo::kUnchanged)
		// Use first source setup
		channelMap = audioSourceChannelMap;
	else {
		// Setup
		EAudioChannelMap	destinationChannelMap =
									audioDestinationFirstAudioProcessingSetup.getChannelMapInfo().getValue();
		if (AUDIOCHANNELMAP_ISUNKNOWN(audioSourceChannelMap) || AUDIOCHANNELMAP_ISUNKNOWN(destinationChannelMap) ||
				(audioSourceChannelMap == destinationChannelMap) ||
				CAudioChannelMapper::canPerform(audioSourceChannelMap, destinationChannelMap))
			// Use source
			channelMap = audioSourceChannelMap;
		else {
			// Both audio source and destination have a specific channel map, they are not the same, and we currently
			//	have no way to map
			AssertFailUnimplemented();
			channelMap = audioSourceChannelMap;
		}
	}

	// Compose sample type
	SAudioProcessingFormat::SampleType	sampleType;
	if ((audioSourceFirstAudioProcessingSetup.getSampleTypeOption() == SAudioProcessingSetup::kSampleTypeFloat) ||
			(audioDestinationFirstAudioProcessingSetup.getSampleTypeOption() ==
					SAudioProcessingSetup::kSampleTypeFloat))
		// Use float
		sampleType = SAudioProcessingFormat::kSampleTypeFloat;
	else
		// Use signed integer
		sampleType = SAudioProcessingFormat::kSampleTypeSignedInteger;

	// Compose endian
	SAudioProcessingFormat::Endian	endian;
	if (audioSourceFirstAudioProcessingSetup.getEndianOption() ==
			audioDestinationFirstAudioProcessingSetup.getEndianOption())
		// Use same as source and destination
		endian =
				(audioSourceFirstAudioProcessingSetup.getEndianOption() == SAudioProcessingSetup::kEndianBig) ?
						SAudioProcessingFormat::kEndianBig : SAudioProcessingFormat::kEndianLittle;
	else
		// Use native
		endian = SAudioProcessingFormat::kEndianNative;


	// Compose interleaved
	SAudioProcessingFormat::Interleaved	interleaved =
												(audioDestinationFirstAudioProcessingSetup.getInterleavedOption() ==
																SAudioProcessingSetup::kInterleaved) ?
														SAudioProcessingFormat::kInterleaved :
														SAudioProcessingFormat::kNonInterleaved;

	return SAudioProcessingFormat(bits, sampleRate, channelMap, sampleType, endian, interleaved);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaEngine::ConnectResult CMediaEngine::connect(const I<CAudioProcessor>& audioProcessorSource,
		const I<CAudioProcessor>& audioProcessorDestination, const SAudioProcessingFormat& audioProcessingFormat) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TArray<SAudioProcessingSetup>	audioProcessorSourceAudioProcessingSetups = audioProcessorSource->getOutputSetups();
	TArray<SAudioProcessingSetup>	audioProcessorDestinationAudioProcessingSetups =
											audioProcessorDestination->getInputSetups();

	// Determine if can connect directly
	OI<SAudioProcessingFormat>	commonAudioProcessingFormat =
										sDetermineCommonAudioProcessingFormat(audioProcessorSourceAudioProcessingSetups,
												audioProcessorDestinationAudioProcessingSetups, audioProcessingFormat);
	if (commonAudioProcessingFormat.hasInstance()) {
		// Connect directly
		OI<SError>	error = audioProcessorSource->setOutputFormat(*commonAudioProcessingFormat);
		ReturnValueIfError(error, ConnectResult(*error));

		audioProcessorDestination->connectInput(audioProcessorSource, *commonAudioProcessingFormat);

		return *commonAudioProcessingFormat;
	} else {
		// Requires converter and/or channel mapper
		SAudioProcessingFormats	audioProcessingFormats =
										sComposeAudioProcessingFormats(
												audioProcessorSourceAudioProcessingSetups.getFirst(),
												audioProcessorDestinationAudioProcessingSetups.getFirst(),
												audioProcessingFormat);
		bool					requiresConverter =
										!audioProcessingFormats.doBitsMatch() ||
												!audioProcessingFormats.doSampleRatesMatch() ||
												!audioProcessingFormats.doInterleavedsMatch();
		bool					requiresChannelMapper = !audioProcessingFormats.doChannelMapsMatch();
		OI<SError>				error;
		if (requiresConverter && requiresChannelMapper) {
			// Requires converter and channel mapper
			SAudioProcessingFormat	intermediateAudioProcessingFormat(
											audioProcessingFormats.mSourceAudioProcessingFormat.getBits(),
											audioProcessingFormats.mSourceAudioProcessingFormat.getSampleRate(),
											audioProcessingFormats.mDestinationAudioProcessingFormat.getChannelMap(),
											audioProcessingFormats.mSourceAudioProcessingFormat.getSampleType(),
											audioProcessingFormats.mSourceAudioProcessingFormat.getEndian(),
											audioProcessingFormats.mSourceAudioProcessingFormat.getInterleaved());

			// Setup
			I<CAudioChannelMapper>	audioChannelMapper(new CAudioChannelMapper());
			I<CAudioConverter>		audioConverter = createAudioConverter();

			// Connect
			error =
					audioProcessorDestination->connectInput((I<CAudioProcessor>&) audioConverter,
							audioProcessingFormats.mDestinationAudioProcessingFormat);
			if (error.hasInstance()) return ConnectResult(*error);

			error =
					audioConverter->connectInput((I<CAudioProcessor>&) audioChannelMapper,
							intermediateAudioProcessingFormat);
			if (error.hasInstance()) return ConnectResult(*error);

			error =
					audioChannelMapper->connectInput(audioProcessorSource,
							audioProcessingFormats.mSourceAudioProcessingFormat);
			if (error.hasInstance()) return ConnectResult(*error);
		} else if (requiresConverter) {
			// Requires converter
			I<CAudioConverter>	audioConverter = createAudioConverter();

			// Connect
			error =
					audioProcessorDestination->connectInput((I<CAudioProcessor>&) audioConverter,
							audioProcessingFormats.mDestinationAudioProcessingFormat);
			if (error.hasInstance()) return ConnectResult(*error);

			error =
					audioConverter->connectInput(audioProcessorSource,
							audioProcessingFormats.mSourceAudioProcessingFormat);
			if (error.hasInstance()) return ConnectResult(*error);
		} else {
			// Requires channel mapper
			I<CAudioChannelMapper>	audioChannelMapper(new CAudioChannelMapper());

			// Connect
			error =
					audioProcessorDestination->connectInput((I<CAudioProcessor>&) audioChannelMapper,
							audioProcessingFormats.mDestinationAudioProcessingFormat);
			if (error.hasInstance()) return ConnectResult(*error);

			error =
					audioChannelMapper->connectInput(audioProcessorSource,
							audioProcessingFormats.mSourceAudioProcessingFormat);
			if (error.hasInstance()) return ConnectResult(*error);
		}

		return audioProcessingFormats.mSourceAudioProcessingFormat;
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
OI<SAudioProcessingFormat> sDetermineCommonAudioProcessingFormat(
		const TArray<SAudioProcessingSetup>& sourceAudioProcessingSetups,
		const TArray<SAudioProcessingSetup>& destinationAudioProcessingSetups,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate source audio processing setups
	for (TIteratorD<SAudioProcessingSetup> sourceIterator = sourceAudioProcessingSetups.getIterator();
			sourceIterator.hasValue(); sourceIterator.advance()) {
		// Setup
		const	SAudioProcessingSetup&						sourceAudioProcessingSetup = sourceIterator.getValue();
		const	SAudioProcessingSetup::BitsInfo&			sourceBitsInfo = sourceAudioProcessingSetup.getBitsInfo();
		const	SAudioProcessingSetup::SampleRateInfo&		sourceSampleRateInfo =
																	sourceAudioProcessingSetup.getSampleRateInfo();
		const	SAudioProcessingSetup::ChannelMapInfo&		sourceChannelMapInfo =
																	sourceAudioProcessingSetup.getChannelMapInfo();
				SAudioProcessingSetup::SampleTypeOption		sourceSampleTypeOption =
																	sourceAudioProcessingSetup.getSampleTypeOption();
				SAudioProcessingSetup::EndianOption			sourceEndianOption =
																	sourceAudioProcessingSetup.getEndianOption();
				SAudioProcessingSetup::InterleavedOption	sourceInterleavedOption =
																	sourceAudioProcessingSetup.getInterleavedOption();

		// Iterate destination audio processing setups
		for (TIteratorD<SAudioProcessingSetup> destinationIterator = destinationAudioProcessingSetups.getIterator();
				destinationIterator.hasValue(); destinationIterator.advance()) {
			// Setup
			const	SAudioProcessingSetup&						destinationAudioProcessingSetup =
																		destinationIterator.getValue();
			const	SAudioProcessingSetup::BitsInfo&			destinationBitsInfo =
																		destinationAudioProcessingSetup.getBitsInfo();
			const	SAudioProcessingSetup::SampleRateInfo&		destinationSampleRateInfo =
																		destinationAudioProcessingSetup
																				.getSampleRateInfo();
			const	SAudioProcessingSetup::ChannelMapInfo&		destinationChannelMapInfo =
																		destinationAudioProcessingSetup
																				.getChannelMapInfo();
					SAudioProcessingSetup::SampleTypeOption		destinationSampleTypeOption =
																		destinationAudioProcessingSetup
																				.getSampleTypeOption();
					SAudioProcessingSetup::EndianOption			destinationEndianOption =
																		destinationAudioProcessingSetup
																				.getEndianOption();
					SAudioProcessingSetup::InterleavedOption	destinationInterleavedOption =
																		destinationAudioProcessingSetup
																				.getInterleavedOption();

			// Check bits
			UInt8	bits;
			if (!sourceBitsInfo.isSpecified() && !destinationBitsInfo.isSpecified())
				// Neither source nor destination are specified, use processing bits
				bits = audioProcessingFormat.getBits();
			else if (!sourceBitsInfo.isSpecified())
				// Source is not specified, use destination bits
				bits = destinationBitsInfo.getValue();
			else if (!destinationBitsInfo.isSpecified())
				// Destination is not specified, use source bits
				bits = sourceBitsInfo.getValue();
			else if (sourceBitsInfo.getValue() == destinationBitsInfo.getValue())
				// Both are specified and match
				bits = sourceBitsInfo.getValue();
			else
				// Both are specified and do not match
				continue;

			// Check sample rate
			Float32	sampleRate;
			if (!sourceSampleRateInfo.isSpecified() && !destinationSampleRateInfo.isSpecified())
				// Neither source nor destination are specified, use processing sample rate
				sampleRate = audioProcessingFormat.getSampleRate();
			else if (!sourceSampleRateInfo.isSpecified())
				// Source is not specified, use destination sample rate
				sampleRate = destinationSampleRateInfo.getValue();
			else if (!destinationSampleRateInfo.isSpecified())
				// Destination is not specified, use source samnple rate
				sampleRate = sourceSampleRateInfo.getValue();
			else if (sourceSampleRateInfo.getValue() == destinationSampleRateInfo.getValue())
				// Both are specified and match
				sampleRate = sourceSampleRateInfo.getValue();
			else
				// Both are specified and do not match
				continue;

			// Check channel map
			EAudioChannelMap	channelMap;
			if (!sourceChannelMapInfo.isSpecified() && !destinationChannelMapInfo.isSpecified())
				// Neither source nor destination are specified, use processing channel map
				channelMap = audioProcessingFormat.getChannelMap();
			else if (!sourceChannelMapInfo.isSpecified())
				// Source is not specified, use destination channel map
				channelMap = destinationChannelMapInfo.getValue();
			else if (!destinationChannelMapInfo.isSpecified())
				// Destination is not specifiefd, use source channel map
				channelMap = sourceChannelMapInfo.getValue();
			else {
				// Setup
				EAudioChannelMap	sourceChannelMap = sourceChannelMapInfo.getValue();
				EAudioChannelMap	destinationChannelMap = destinationChannelMapInfo.getValue();
				if (AUDIOCHANNELMAP_CHANNELCOUNT(sourceChannelMap) ==
						AUDIOCHANNELMAP_CHANNELCOUNT(destinationChannelMap)) {
					// Channel counts match
					if (AUDIOCHANNELMAP_ISUNKNOWN(sourceChannelMap) && AUDIOCHANNELMAP_ISUNKNOWN(destinationChannelMap))
						// No channel map
						channelMap = sourceChannelMap;
					else if (AUDIOCHANNELMAP_ISUNKNOWN(sourceChannelMap))
						// No source channel map
						channelMap = destinationChannelMap;
					else if (AUDIOCHANNELMAP_ISUNKNOWN(destinationChannelMap))
						// No destination channel map
						channelMap = sourceChannelMap;
					else if (sourceChannelMap == destinationChannelMap)
						// Channel maps match
						channelMap = sourceChannelMap;
					else
						// Channel maps do not match
						continue;
				} else
					// Channel counts do not match
					continue;
			}

			// Check sample type
			bool	isFloat;
			if (!sourceAudioProcessingSetup.isSampleTypeOptionSpecified() &&
					!destinationAudioProcessingSetup.isSampleTypeOptionSpecified())
				// Neither source nor destination are specified, use processing sample type
				isFloat = audioProcessingFormat.getIsFloat();
			else if (!sourceAudioProcessingSetup.isSampleTypeOptionSpecified())
				// Source is not specified, use destination sample type
				isFloat = destinationSampleTypeOption == SAudioProcessingSetup::kSampleTypeFloat;
			else if (!destinationAudioProcessingSetup.isSampleTypeOptionSpecified())
				// Destination is not specified, use source sample type
				isFloat = sourceSampleTypeOption == SAudioProcessingSetup::kSampleTypeFloat;
			else if (sourceSampleTypeOption == destinationSampleTypeOption)
				// Both are specified and match
				isFloat = sourceSampleTypeOption == SAudioProcessingSetup::kSampleTypeFloat;
			else
				// Both are specified and do not match
				continue;

			// Check endian
			bool	isBigEndian;
			if (!sourceAudioProcessingSetup.isEndianOptionSpecified() &&
					!destinationAudioProcessingSetup.isEndianOptionSpecified())
				// Neither source nor destination are specified, use processing endian
				isBigEndian = audioProcessingFormat.getIsBigEndian();
			else if (!sourceAudioProcessingSetup.isEndianOptionSpecified())
				// Source is not specified, use destination endian
				isBigEndian = destinationEndianOption == SAudioProcessingSetup::kEndianBig;
			else if (!destinationAudioProcessingSetup.isEndianOptionSpecified())
				// Destination is not specified, use source endian
				isBigEndian = sourceEndianOption == SAudioProcessingSetup::kEndianBig;
			else if (sourceEndianOption == destinationEndianOption)
				// Both are specified and match
				isBigEndian = sourceEndianOption == SAudioProcessingSetup::kEndianBig;
			else
				// Both are specified and do not match
				continue;

			// Check interleaved
			bool	isInterleaved;
			if (!sourceAudioProcessingSetup.isInterleavedOptionSpecified() &&
					!destinationAudioProcessingSetup.isInterleavedOptionSpecified())
				// Neither source nor destination are specified, use processing interleaved
				isInterleaved = audioProcessingFormat.getIsInterleaved();
			else if (!sourceAudioProcessingSetup.isInterleavedOptionSpecified())
				// Source is not specified, use destination interleaved
				isInterleaved = destinationInterleavedOption == SAudioProcessingSetup::kInterleaved;
			else if (!destinationAudioProcessingSetup.isInterleavedOptionSpecified())
				// Destination is not specified, use source interleaved
				isInterleaved = sourceInterleavedOption == SAudioProcessingSetup::kInterleaved;
			else if (sourceInterleavedOption == destinationInterleavedOption)
				// Both are specified and match
				isInterleaved = sourceInterleavedOption == SAudioProcessingSetup::kInterleaved;
			else
				// Both are specified and do not match
				continue;

			// We can connect directly
			return OI<SAudioProcessingFormat>(new SAudioProcessingFormat(bits, sampleRate, channelMap,
					isFloat ?
							SAudioProcessingFormat::kSampleTypeFloat : SAudioProcessingFormat::kSampleTypeSignedInteger,
					isBigEndian ? SAudioProcessingFormat::kEndianBig : SAudioProcessingFormat::kEndianLittle,
					isInterleaved ? SAudioProcessingFormat::kInterleaved : SAudioProcessingFormat::kNonInterleaved));
		}
	}

	return OI<SAudioProcessingFormat>();
}

//----------------------------------------------------------------------------------------------------------------------
SAudioProcessingFormats sComposeAudioProcessingFormats(const SAudioProcessingSetup& sourceAudioProcessingSetup,
		const SAudioProcessingSetup& destinationAudioProcessingSetup,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SAudioProcessingSetup::BitsInfo&			sourceBitsInfo = sourceAudioProcessingSetup.getBitsInfo();
	const	SAudioProcessingSetup::SampleRateInfo&		sourceSampleRateInfo =
																sourceAudioProcessingSetup.getSampleRateInfo();
	const	SAudioProcessingSetup::ChannelMapInfo&		sourceChannelMapInfo =
																sourceAudioProcessingSetup.getChannelMapInfo();
			SAudioProcessingSetup::SampleTypeOption		sourceSampleTypeOption =
																sourceAudioProcessingSetup.getSampleTypeOption();
			SAudioProcessingSetup::EndianOption			sourceEndianOption =
																sourceAudioProcessingSetup.getEndianOption();
			SAudioProcessingSetup::InterleavedOption	sourceInterleavedOption =
																sourceAudioProcessingSetup.getInterleavedOption();

	const	SAudioProcessingSetup::BitsInfo&			destinationBitsInfo =
																destinationAudioProcessingSetup.getBitsInfo();
	const	SAudioProcessingSetup::SampleRateInfo&		destinationSampleRateInfo =
																destinationAudioProcessingSetup.getSampleRateInfo();
	const	SAudioProcessingSetup::ChannelMapInfo&		destinationChannelMapInfo =
																destinationAudioProcessingSetup.getChannelMapInfo();
			SAudioProcessingSetup::SampleTypeOption		destinationSampleTypeOption =
																destinationAudioProcessingSetup.getSampleTypeOption();
			SAudioProcessingSetup::EndianOption			destinationEndianOption =
																destinationAudioProcessingSetup.getEndianOption();
			SAudioProcessingSetup::InterleavedOption	destinationInterleavedOption =
																destinationAudioProcessingSetup.getInterleavedOption();

	// Setup bits
	UInt8	sourceBits, destinationBits;
	if (!sourceBitsInfo.isSpecified() && !destinationBitsInfo.isSpecified())
		// Neither source nor destination is specified, use processing bits
		sourceBits = destinationBits = audioProcessingFormat.getBits();
	else if (!sourceBitsInfo.isSpecified())
		// Source is not specified, use destination bits
		sourceBits = destinationBits = destinationBitsInfo.getValue();
	else if (!destinationBitsInfo.isSpecified())
		// Destination is not specified, use source bits
		sourceBits = destinationBits = sourceBitsInfo.getValue();
	else {
		// Both are specified
		sourceBits = sourceBitsInfo.getValue();
		destinationBits = destinationBitsInfo.getValue();
	}

	// Setup sample rate
	Float32	sourceSampleRate, destinationSampleRate;
	if (!sourceSampleRateInfo.isSpecified() && !destinationSampleRateInfo.isSpecified())
		// Neither source nor destination is specified, use processing sample rate
		sourceSampleRate = destinationSampleRate = audioProcessingFormat.getSampleRate();
	else if (!sourceSampleRateInfo.isSpecified())
		// Source is not specified, use destination sample rate
		sourceSampleRate = destinationSampleRate = destinationSampleRateInfo.getValue();
	else if (!destinationSampleRateInfo.isSpecified())
		// Destination is not specified, use source sample rate
		sourceSampleRate = destinationSampleRate = sourceSampleRateInfo.getValue();
	else {
		// Both are specified
		sourceSampleRate = sourceSampleRateInfo.getValue();
		destinationSampleRate = destinationSampleRateInfo.getValue();
	}

	// Setup channel map
	EAudioChannelMap	sourceChannelMap, destinationChannelMap;
	if (!sourceChannelMapInfo.isSpecified() && !destinationChannelMapInfo.isSpecified())
		// Neither source nor destination is specified, use processing channel map
		sourceChannelMap = destinationChannelMap = audioProcessingFormat.getChannelMap();
	else if (!sourceChannelMapInfo.isSpecified())
		// Source is not specified, use destination channel map
		sourceChannelMap = destinationChannelMap = destinationChannelMapInfo.getValue();
	else if (!destinationChannelMapInfo.isSpecified())
		// Destination is not specified, use source channel map
		sourceChannelMap = destinationChannelMap = sourceChannelMapInfo.getValue();
	else {
		// Both are specified
		sourceChannelMap = sourceChannelMapInfo.getValue();
		destinationChannelMap = destinationChannelMapInfo.getValue();
	}

	// Setup sample type
	bool	sourceIsFloat, destinationIsFloat;
	if (!sourceAudioProcessingSetup.isSampleTypeOptionSpecified() &&
			!destinationAudioProcessingSetup.isSampleTypeOptionSpecified())
		// Neither source nor destination is specified, use processing sample type
		sourceIsFloat = destinationIsFloat = audioProcessingFormat.getIsFloat();
	else if (!sourceAudioProcessingSetup.isSampleTypeOptionSpecified())
		// Source is not specified, use destination sample type
		sourceIsFloat = destinationIsFloat = destinationSampleTypeOption == SAudioProcessingSetup::kSampleTypeFloat;
	else if (!destinationAudioProcessingSetup.isSampleTypeOptionSpecified())
		// Destination is not specified, use source sample type
		sourceIsFloat = destinationIsFloat = sourceSampleTypeOption == SAudioProcessingSetup::kSampleTypeFloat;
	else {
		// Both are specified
		sourceIsFloat = sourceSampleTypeOption == SAudioProcessingSetup::kSampleTypeFloat;
		destinationIsFloat = destinationSampleTypeOption == SAudioProcessingSetup::kSampleTypeFloat;
	}

	// Setup endian
	bool	sourceIsBigEndian, destinationIsBigEndian;
	if (!sourceAudioProcessingSetup.isEndianOptionSpecified() &&
			!destinationAudioProcessingSetup.isEndianOptionSpecified())
		// Neither source nor destination is specified, use processing endian
		sourceIsBigEndian = destinationIsBigEndian = audioProcessingFormat.getIsBigEndian();
	else if (!sourceAudioProcessingSetup.isEndianOptionSpecified())
		// Source is not specified, use destination endian
		sourceIsBigEndian = destinationIsBigEndian = destinationEndianOption == SAudioProcessingSetup::kEndianBig;
	else if (!destinationAudioProcessingSetup.isEndianOptionSpecified())
		// Destination is not specified, use source endian
		sourceIsBigEndian = destinationIsBigEndian = sourceEndianOption == SAudioProcessingSetup::kEndianBig;
	else {
		// Both are specified
		sourceIsBigEndian = sourceEndianOption == SAudioProcessingSetup::kEndianBig;
		destinationIsBigEndian = destinationEndianOption == SAudioProcessingSetup::kEndianBig;
	}

	// Setup interleaved
	bool	sourceIsInterleaved, destinationIsInterleaved;
	if (!sourceAudioProcessingSetup.isInterleavedOptionSpecified() &&
			!destinationAudioProcessingSetup.isInterleavedOptionSpecified())
		// Neither source nor destination is specified, use processing interleaved
		sourceIsInterleaved = destinationIsInterleaved = audioProcessingFormat.getIsInterleaved();
	else if (!sourceAudioProcessingSetup.isInterleavedOptionSpecified())
		// Source is not specified, use destination interleaved
		sourceIsInterleaved = destinationIsInterleaved =
				destinationInterleavedOption == SAudioProcessingSetup::kInterleaved;
	else if (!destinationAudioProcessingSetup.isInterleavedOptionSpecified())
		// Destination is not specified, use source interleaved
		sourceIsInterleaved = destinationIsInterleaved =
				sourceInterleavedOption == SAudioProcessingSetup::kInterleaved;
	else {
		// Both are specified
		sourceIsInterleaved = sourceInterleavedOption == SAudioProcessingSetup::kInterleaved;
		destinationIsInterleaved = destinationInterleavedOption == SAudioProcessingSetup::kInterleaved;
	}

	return SAudioProcessingFormats(
			SAudioProcessingFormat(sourceBits, sourceSampleRate, sourceChannelMap,
					sourceIsFloat ?
							SAudioProcessingFormat::kSampleTypeFloat : SAudioProcessingFormat::kSampleTypeSignedInteger,
					sourceIsBigEndian ? SAudioProcessingFormat::kEndianBig : SAudioProcessingFormat::kEndianLittle,
					sourceIsInterleaved ?
							SAudioProcessingFormat::kInterleaved : SAudioProcessingFormat::kNonInterleaved),
			SAudioProcessingFormat(destinationBits, destinationSampleRate, destinationChannelMap,
					destinationIsFloat ?
							SAudioProcessingFormat::kSampleTypeFloat : SAudioProcessingFormat::kSampleTypeSignedInteger,
					destinationIsBigEndian ? SAudioProcessingFormat::kEndianBig : SAudioProcessingFormat::kEndianLittle,
					destinationIsInterleaved ?
							SAudioProcessingFormat::kInterleaved : SAudioProcessingFormat::kNonInterleaved));
}
