//----------------------------------------------------------------------------------------------------------------------
//	CMediaEngine.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaEngine.h"

#include "CAudioChannelMapper.h"
#include "CAudioDecoder.h"
#include "CAudioDeinterleaver.h"
#include "CAudioInterleaver.h"
#include "CCodecRegistry.h"
#include "CVideoDecoder.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local types
struct SAudioProcessingFormats {
			// Lifecycle Methods
			SAudioProcessingFormats(const SAudio::ProcessingFormat& sourceAudioProcessingFormat,
					const SAudio::ProcessingFormat& destinationAudioProcessingFormat) :
				mSourceAudioProcessingFormat(sourceAudioProcessingFormat),
						mDestinationAudioProcessingFormat(destinationAudioProcessingFormat)
				{}

			// Instance methods
	bool	doBitsMatch() const
				{ return mSourceAudioProcessingFormat.getBits() == mDestinationAudioProcessingFormat.getBits(); }
	bool	doSampleRatesMatch() const
				{ return mSourceAudioProcessingFormat.getSampleRate() ==
						mDestinationAudioProcessingFormat.getSampleRate(); }
	bool	doAudioChannelMapsMatch() const
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
	SAudio::ProcessingFormat	mSourceAudioProcessingFormat;
	SAudio::ProcessingFormat	mDestinationAudioProcessingFormat;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	OV<SAudio::ProcessingFormat>	sDetermineCommonAudioProcessingFormat(
												const TArray<SAudio::ProcessingSetup>& sourceAudioProcessingSetups,
												const TArray<SAudio::ProcessingSetup>& destinationAudioProcessingSetups,
												const SAudio::ProcessingFormat& audioProcessingFormat);
static	SAudioProcessingFormats			sComposeAudioProcessingFormats(
												const SAudio::ProcessingSetup& sourceAudioProcessingSetup,
												const SAudio::ProcessingSetup& destinationAudioProcessingSetup,
												const SAudio::ProcessingFormat& audioProcessingFormat);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaEngine

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
I<CAudioSource> CMediaEngine::getAudioSource(const CMediaTrackInfos::AudioTrackInfo& audioTrackInfo,
		const CString& identifier) const
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CAudioSource>(new CAudioDecoder(audioTrackInfo.mMediaTrackFormat, *audioTrackInfo.mCodec, identifier));
}

//----------------------------------------------------------------------------------------------------------------------
I<CVideoSource> CMediaEngine::getVideoSource(const CMediaTrackInfos::VideoTrackInfo& videoTrackInfo,
		const CString& identifier) const
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CVideoSource>(
			new CVideoDecoder(videoTrackInfo.mMediaTrackFormat, *videoTrackInfo.mCodec,
					CVideoProcessor::Format(videoTrackInfo.mMediaTrackFormat.getFrameSize(),
							videoTrackInfo.mMediaTrackFormat.getFramerate()),
					identifier));
}

//----------------------------------------------------------------------------------------------------------------------
SAudio::ProcessingFormat CMediaEngine::composeAudioProcessingFormat(const CAudioSource& audioSource,
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
			TArray<SAudio::ProcessingSetup>	audioSourceAudioProcessingSetups = audioSource.getOutputSetups();
	const	SAudio::ProcessingSetup&		audioSourceFirstAudioProcessingSetup =
													audioSourceAudioProcessingSetups.getFirst();

			TArray<SAudio::ProcessingSetup>	audioDestinationProcessingSetups = audioDestination.getInputSetups();
	const	SAudio::ProcessingSetup&		audioDestinationFirstAudioProcessingSetup =
													audioDestinationProcessingSetups.getFirst();

	// Compose bits
	UInt8	audioSourceBits = audioSourceFirstAudioProcessingSetup.getBitsInfo().getValue();
	UInt8	bits;
	if (audioDestinationFirstAudioProcessingSetup.getBitsInfo().getOption() ==
			SAudio::ProcessingSetup::BitsInfo::kUnchanged)
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
				SAudio::ProcessingSetup::SampleRateInfo::kUnchanged)
			// Use first source setup
			sampleRate = audioSourceSampleRate;
		else
			// Use highest of source and destination
			sampleRate =
					std::max<Float32>(audioSourceSampleRate,
							audioDestinationFirstAudioProcessingSetup.getSampleRateInfo().getValue());
	}

	// Compose channel map
	const	SAudio::ChannelMap&	audioSourceChannelMap =
										audioSourceFirstAudioProcessingSetup.getChannelMapInfo().getValue();
			SAudio::ChannelMap	audioChannelMap((UInt8) 0);
	if (audioDestinationFirstAudioProcessingSetup.getChannelMapInfo().getOption() ==
			SAudio::ProcessingSetup::ChannelMapInfo::kUnchanged)
		// Use first source setup
		audioChannelMap = audioSourceChannelMap;
	else {
		// Setup
		const	SAudio::ChannelMap&	destinationAudioChannelMap =
											audioDestinationFirstAudioProcessingSetup.getChannelMapInfo().getValue();
		if (audioSourceChannelMap.hasUnknownOrder() || destinationAudioChannelMap.hasUnknownOrder() ||
				(audioSourceChannelMap == destinationAudioChannelMap) ||
				CAudioChannelMapper::canPerform(audioSourceChannelMap, destinationAudioChannelMap))
			// Use source
			audioChannelMap = audioSourceChannelMap;
		else {
			// Both audio source and destination have a specific channel map, they are not the same, and we currently
			//	have no way to map
			AssertFailUnimplemented();
			audioChannelMap = audioSourceChannelMap;
		}
	}

	// Compose sample type
	SAudio::ProcessingFormat::SampleType	sampleType;
	if ((audioSourceFirstAudioProcessingSetup.getSampleTypeOption() == SAudio::ProcessingSetup::kSampleTypeFloat) ||
			(audioDestinationFirstAudioProcessingSetup.getSampleTypeOption() ==
					SAudio::ProcessingSetup::kSampleTypeFloat))
		// Use float
		sampleType = SAudio::ProcessingFormat::kSampleTypeFloat;
	else
		// Use signed integer
		sampleType = SAudio::ProcessingFormat::kSampleTypeSignedInteger;

	// Compose endian
	SAudio::ProcessingFormat::Endian	endian;
	if (audioSourceFirstAudioProcessingSetup.getEndianOption() ==
			audioDestinationFirstAudioProcessingSetup.getEndianOption())
		// Use same as source and destination
		endian =
				(audioSourceFirstAudioProcessingSetup.getEndianOption() == SAudio::ProcessingSetup::kEndianBig) ?
						SAudio::ProcessingFormat::kEndianBig : SAudio::ProcessingFormat::kEndianLittle;
	else
		// Use native
		endian = SAudio::ProcessingFormat::kEndianNative;

	// Compose interleaved
	SAudio::ProcessingFormat::Interleaved	interleaved =
												(audioDestinationFirstAudioProcessingSetup.getInterleavedOption() ==
																SAudio::ProcessingSetup::kInterleaved) ?
														SAudio::ProcessingFormat::kInterleaved :
														SAudio::ProcessingFormat::kNonInterleaved;

	return SAudio::ProcessingFormat(bits, sampleRate, audioChannelMap, sampleType, endian, interleaved);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SAudio::ProcessingFormat> CMediaEngine::connect(const I<CAudioProcessor>& audioProcessorSource,
		const I<CAudioProcessor>& audioProcessorDestination, const SAudio::ProcessingFormat& audioProcessingFormat) const
//----------------------------------------------------------------------------------------------------------------------
{
/*
	Source can be interleaved or non-interleaved
	Destination can be interleaved or non-interleaved
	May require Audio Converter if bits or sample rate don't match
		Audio Converters always support interleaved, but may not support non-interleaved
	May require Audio Channel Mapper if Audio Channel Maps don't match

	Possible scenarios
		Converter supports non-interleaved
			Source (interleaved) -> Channel Mapper -> Converter -> Destination (interleaved)
			Source (interleaved) -> Channel Mapper -> Converter -> Destination (non-interleaved)
			Source (non-interleaved) -> Channel Mapper -> Converter -> Destination (interleaved)
			Source (non-interleaved) -> Channel Mapper -> Converter -> Destination (non-interleaved)

			Source (interleaved) -> Converter -> Destination (interleaved)
			Source (interleaved) -> Converter -> Destination (non-interleaved)
			Source (non-interleaved) -> Converter -> Destination (interleaved)
			Source (non-interleaved) -> Converter -> Destination (non-interleaved)

		Converter only supports interleaved
			Source (interleaved) -> Channel Mapper -> Converter -> Destination (interleaved)
			Source (interleaved) -> Channel Mapper -> Converter -> Deinterleaver -> Destination (non-interleaved)
			Source (non-interleaved) -> Channel Mapper -> Interleaver -> Converter -> Destination (interleaved)
			Source (non-interleaved) -> Channel Mapper -> Interleaver -> Converter -> Deinterleaver -> Destination (non-interleaved)

			Source (interleaved) -> Converter -> Destination (interleaved)
			Source (interleaved) -> Converter -> Deinterleaver -> Destination (non-interleaved)
			Source (non-interleaved) -> Interleaver -> Converter -> Destination (interleaved)
			Source (non-interleaved) -> Interleaver -> Converter -> Deinterleaver -> Destination (non-interleaved)

		No Converter
			Source (interleaved) -> Destination (interleaved)
			Source (interleaved) -> Deinterleaver -> Destination (non-interleaved)
			Source (non-interleaved) -> Interleaver -> Destination (interleaved)
			Source (non-interleaved) -> Destination (non-interleaved)

			Source (interleaved) -> Channel Mapper -> Destination (interleaved)
			Source (interleaved) -> Channel Mapper -> Deinterleaver -> Destination (non-interleaved)
			Source (non-interleaved) -> Channel Mapper -> Interleaver -> Destination (interleaved)
			Source (non-interleaved) -> Channel Mapper -> Destination (non-interleaved)
 */

	// Setup
	TArray<SAudio::ProcessingSetup>	audioProcessorSourceAudioProcessingSetups = audioProcessorSource->getOutputSetups();
	TArray<SAudio::ProcessingSetup>	audioProcessorDestinationAudioProcessingSetups =
											audioProcessorDestination->getInputSetups();

	// Determine if can connect directly
	OV<SAudio::ProcessingFormat>	commonAudioProcessingFormat =
										sDetermineCommonAudioProcessingFormat(audioProcessorSourceAudioProcessingSetups,
												audioProcessorDestinationAudioProcessingSetups, audioProcessingFormat);
	if (commonAudioProcessingFormat.hasValue()) {
		// Connect directly
		OV<SError>	error = audioProcessorSource->setOutputFormat(*commonAudioProcessingFormat);
		ReturnValueIfError(error, TVResult<SAudio::ProcessingFormat>(*error));

		audioProcessorDestination->connectInput(audioProcessorSource, *commonAudioProcessingFormat);

		return TVResult<SAudio::ProcessingFormat>(*commonAudioProcessingFormat);
	}

	// Requires intermediate audio processors
	SAudioProcessingFormats		audioProcessingFormats =
										sComposeAudioProcessingFormats(
												audioProcessorSourceAudioProcessingSetups.getFirst(),
												audioProcessorDestinationAudioProcessingSetups.getFirst(),
												audioProcessingFormat);
	I<CAudioProcessor>			currentAudioProcessor(audioProcessorDestination);
	SAudio::ProcessingFormat	currentAudioProcessingFormat(audioProcessingFormats.mDestinationAudioProcessingFormat);
	OV<SError>					error;
	if (!audioProcessingFormats.doBitsMatch() || !audioProcessingFormats.doSampleRatesMatch() ||
			!audioProcessingFormats.doEndiansMatch()) {
		// Requires Audio Converter
		I<CAudioConverter>	audioConverter = createAudioConverter();

		// Check if need Audio Deinterleaver.  Some Audio Converters do not support non-interleaved.  So we supply
		//	a deinterleaver if necessary.
		if (!currentAudioProcessingFormat.getIsInterleaved() && !audioConverter->supportsNoninterleaved()) {
			// Create Audio Deinterleaver
			I<CAudioProcessor>	audioDeinterleaver(new CAudioDeinterleaver());

			// Connect Audio Deinterleaver
			error = currentAudioProcessor->connectInput(audioDeinterleaver, currentAudioProcessingFormat);
			if (error.hasValue()) return TVResult<SAudio::ProcessingFormat>(*error);

			// Update
			currentAudioProcessor = audioDeinterleaver;
			currentAudioProcessingFormat =
					SAudio::ProcessingFormat(currentAudioProcessingFormat.getBits(),
							currentAudioProcessingFormat.getSampleRate(),
							currentAudioProcessingFormat.getChannelMap(),
							currentAudioProcessingFormat.getSampleType(),
							currentAudioProcessingFormat.getEndian(), SAudio::ProcessingFormat::kInterleaved);
		}

		// Connect Audio Converter
		error = currentAudioProcessor->connectInput((I<CAudioProcessor>&) audioConverter, currentAudioProcessingFormat);
		if (error.hasValue()) return TVResult<SAudio::ProcessingFormat>(*error);

		// Update
		currentAudioProcessor = (I<CAudioProcessor>&) audioConverter;
		currentAudioProcessingFormat =
				SAudio::ProcessingFormat(audioProcessingFormats.mSourceAudioProcessingFormat.getBits(),
						audioProcessingFormats.mSourceAudioProcessingFormat.getSampleRate(),
						currentAudioProcessingFormat.getChannelMap(),
						audioProcessingFormats.mSourceAudioProcessingFormat.getSampleType(),
						audioProcessingFormats.mSourceAudioProcessingFormat.getEndian(),
						audioConverter->supportsNoninterleaved() ?
								audioProcessingFormats.mSourceAudioProcessingFormat.getInterleaved() :
								SAudio::ProcessingFormat::kInterleaved);

		// Check if need Audio Interleaver
		if (!audioProcessingFormats.mSourceAudioProcessingFormat.getIsInterleaved() &&
				!audioConverter->supportsNoninterleaved()) {
			// Create Audio Interleaver
			I<CAudioProcessor>	audioInterleaver(new CAudioInterleaver());

			// Connect Audio Interleaver
			error = currentAudioProcessor->connectInput(audioInterleaver, currentAudioProcessingFormat);
			if (error.hasValue()) return TVResult<SAudio::ProcessingFormat>(*error);

			// Update
			currentAudioProcessor = audioInterleaver;
			currentAudioProcessingFormat =
					SAudio::ProcessingFormat(currentAudioProcessingFormat.getBits(),
							currentAudioProcessingFormat.getSampleRate(),
							currentAudioProcessingFormat.getChannelMap(),
							currentAudioProcessingFormat.getSampleType(),
							currentAudioProcessingFormat.getEndian(), SAudio::ProcessingFormat::kNonInterleaved);
		}
	}

	// Check if need Audio Channel Mapper
	if (!audioProcessingFormats.doAudioChannelMapsMatch()) {
		// Requires channel mapper
		I<CAudioChannelMapper>	audioChannelMapper(new CAudioChannelMapper());

		// Connect
		error =
				currentAudioProcessor->connectInput((I<CAudioProcessor>&) audioChannelMapper,
						currentAudioProcessingFormat);
		if (error.hasValue()) return TVResult<SAudio::ProcessingFormat>(*error);

		// Update
		currentAudioProcessor = (I<CAudioProcessor>&) audioChannelMapper;
		currentAudioProcessingFormat =
				SAudio::ProcessingFormat(currentAudioProcessingFormat.getBits(),
						currentAudioProcessingFormat.getSampleRate(),
						audioProcessingFormats.mSourceAudioProcessingFormat.getChannelMap(),
						currentAudioProcessingFormat.getSampleType(),
						currentAudioProcessingFormat.getEndian(),
						currentAudioProcessingFormat.getInterleaved());
	}

	// Check if need Deinterleaver
	if (currentAudioProcessingFormat.getIsInterleaved() &&
			!audioProcessingFormats.mSourceAudioProcessingFormat.getIsInterleaved()) {
		// Create Audio Interleaver
		I<CAudioProcessor>	audioInterleaver(new CAudioInterleaver());

		// Connect Audio Interleaver
		error = currentAudioProcessor->connectInput(audioInterleaver, currentAudioProcessingFormat);
		if (error.hasValue()) return TVResult<SAudio::ProcessingFormat>(*error);

		// Update
		currentAudioProcessor = audioInterleaver;
		currentAudioProcessingFormat =
				SAudio::ProcessingFormat(currentAudioProcessingFormat.getBits(),
						currentAudioProcessingFormat.getSampleRate(),
						currentAudioProcessingFormat.getChannelMap(),
						currentAudioProcessingFormat.getSampleType(),
						currentAudioProcessingFormat.getEndian(), SAudio::ProcessingFormat::kNonInterleaved);
	}

	// Check if need Interleaver
	if (!currentAudioProcessingFormat.getIsInterleaved() &&
			audioProcessingFormats.mSourceAudioProcessingFormat.getIsInterleaved()) {
		// Create Audio Deinterleaver
		I<CAudioProcessor>	audioDeinterleaver(new CAudioDeinterleaver());

		// Connect Audio Deinterleaver
		error = currentAudioProcessor->connectInput(audioDeinterleaver, currentAudioProcessingFormat);
		if (error.hasValue()) return TVResult<SAudio::ProcessingFormat>(*error);

		// Update
		currentAudioProcessor = audioDeinterleaver;
		currentAudioProcessingFormat =
				SAudio::ProcessingFormat(currentAudioProcessingFormat.getBits(),
						currentAudioProcessingFormat.getSampleRate(),
						currentAudioProcessingFormat.getChannelMap(),
						currentAudioProcessingFormat.getSampleType(),
						currentAudioProcessingFormat.getEndian(), SAudio::ProcessingFormat::kInterleaved);
	}

	// Connect source
	error = currentAudioProcessor->connectInput(audioProcessorSource, currentAudioProcessingFormat);
	if (error.hasValue()) return TVResult<SAudio::ProcessingFormat>(*error);

	return TVResult<SAudio::ProcessingFormat>(audioProcessingFormats.mSourceAudioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
OV<SAudio::ProcessingFormat> sDetermineCommonAudioProcessingFormat(
		const TArray<SAudio::ProcessingSetup>& sourceAudioProcessingSetups,
		const TArray<SAudio::ProcessingSetup>& destinationAudioProcessingSetups,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate source audio processing setups
	for (TIteratorD<SAudio::ProcessingSetup> sourceIterator = sourceAudioProcessingSetups.getIterator();
			sourceIterator.hasValue(); sourceIterator.advance()) {
		// Setup
		const	SAudio::ProcessingSetup&					sourceAudioProcessingSetup = sourceIterator.getValue();
		const	SAudio::ProcessingSetup::BitsInfo&			sourceBitsInfo = sourceAudioProcessingSetup.getBitsInfo();
		const	SAudio::ProcessingSetup::SampleRateInfo&	sourceSampleRateInfo =
																	sourceAudioProcessingSetup.getSampleRateInfo();
		const	SAudio::ProcessingSetup::ChannelMapInfo&	sourceChannelMapInfo =
																	sourceAudioProcessingSetup.getChannelMapInfo();
				SAudio::ProcessingSetup::SampleTypeOption	sourceSampleTypeOption =
																	sourceAudioProcessingSetup.getSampleTypeOption();
				SAudio::ProcessingSetup::EndianOption		sourceEndianOption =
																	sourceAudioProcessingSetup.getEndianOption();
				SAudio::ProcessingSetup::InterleavedOption	sourceInterleavedOption =
																	sourceAudioProcessingSetup.getInterleavedOption();

		// Iterate destination audio processing setups
		for (TIteratorD<SAudio::ProcessingSetup> destinationIterator = destinationAudioProcessingSetups.getIterator();
				destinationIterator.hasValue(); destinationIterator.advance()) {
			// Setup
			const	SAudio::ProcessingSetup&					destinationAudioProcessingSetup =
																		destinationIterator.getValue();
			const	SAudio::ProcessingSetup::BitsInfo&			destinationBitsInfo =
																		destinationAudioProcessingSetup.getBitsInfo();
			const	SAudio::ProcessingSetup::SampleRateInfo&	destinationSampleRateInfo =
																		destinationAudioProcessingSetup
																				.getSampleRateInfo();
			const	SAudio::ProcessingSetup::ChannelMapInfo&	destinationChannelMapInfo =
																		destinationAudioProcessingSetup
																				.getChannelMapInfo();
					SAudio::ProcessingSetup::SampleTypeOption	destinationSampleTypeOption =
																		destinationAudioProcessingSetup
																				.getSampleTypeOption();
					SAudio::ProcessingSetup::EndianOption		destinationEndianOption =
																		destinationAudioProcessingSetup
																				.getEndianOption();
					SAudio::ProcessingSetup::InterleavedOption	destinationInterleavedOption =
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
			SAudio::ChannelMap	audioChannelMap((UInt8) 0);
			if (!sourceChannelMapInfo.isSpecified() && !destinationChannelMapInfo.isSpecified())
				// Neither source nor destination are specified, use processing channel map
				audioChannelMap = audioProcessingFormat.getChannelMap();
			else if (!sourceChannelMapInfo.isSpecified())
				// Source is not specified, use destination channel map
				audioChannelMap = destinationChannelMapInfo.getValue();
			else if (!destinationChannelMapInfo.isSpecified())
				// Destination is not specifiefd, use source channel map
				audioChannelMap = sourceChannelMapInfo.getValue();
			else {
				// Setup
				const	SAudio::ChannelMap&	sourceAudioChannelMap = sourceChannelMapInfo.getValue();
				const	SAudio::ChannelMap&	destinationAudioChannelMap = destinationChannelMapInfo.getValue();
				if (sourceAudioChannelMap.getChannels() == destinationAudioChannelMap.getChannels()) {
					// Channel counts match
					if (sourceAudioChannelMap.hasUnknownOrder() && destinationAudioChannelMap.hasUnknownOrder())
						// Both source and destination have unknown order
						audioChannelMap = sourceAudioChannelMap;
					else if (sourceAudioChannelMap.hasUnknownOrder())
						// Source has unknown order
						audioChannelMap = destinationAudioChannelMap;
					else if (destinationAudioChannelMap.hasUnknownOrder())
						// Destination has unknown order
						audioChannelMap = sourceAudioChannelMap;
					else if (sourceAudioChannelMap == destinationAudioChannelMap)
						// Channel maps match
						audioChannelMap = sourceAudioChannelMap;
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
				isFloat = destinationSampleTypeOption == SAudio::ProcessingSetup::kSampleTypeFloat;
			else if (!destinationAudioProcessingSetup.isSampleTypeOptionSpecified())
				// Destination is not specified, use source sample type
				isFloat = sourceSampleTypeOption == SAudio::ProcessingSetup::kSampleTypeFloat;
			else if (sourceSampleTypeOption == destinationSampleTypeOption)
				// Both are specified and match
				isFloat = sourceSampleTypeOption == SAudio::ProcessingSetup::kSampleTypeFloat;
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
				isBigEndian = destinationEndianOption == SAudio::ProcessingSetup::kEndianBig;
			else if (!destinationAudioProcessingSetup.isEndianOptionSpecified())
				// Destination is not specified, use source endian
				isBigEndian = sourceEndianOption == SAudio::ProcessingSetup::kEndianBig;
			else if (sourceEndianOption == destinationEndianOption)
				// Both are specified and match
				isBigEndian = sourceEndianOption == SAudio::ProcessingSetup::kEndianBig;
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
				isInterleaved = destinationInterleavedOption == SAudio::ProcessingSetup::kInterleaved;
			else if (!destinationAudioProcessingSetup.isInterleavedOptionSpecified())
				// Destination is not specified, use source interleaved
				isInterleaved = sourceInterleavedOption == SAudio::ProcessingSetup::kInterleaved;
			else if (sourceInterleavedOption == destinationInterleavedOption)
				// Both are specified and match
				isInterleaved = sourceInterleavedOption == SAudio::ProcessingSetup::kInterleaved;
			else
				// Both are specified and do not match
				continue;

			// We can connect directly
			return OV<SAudio::ProcessingFormat>(SAudio::ProcessingFormat(bits, sampleRate, audioChannelMap,
					isFloat ?
							SAudio::ProcessingFormat::kSampleTypeFloat :
							SAudio::ProcessingFormat::kSampleTypeSignedInteger,
					isBigEndian ? SAudio::ProcessingFormat::kEndianBig : SAudio::ProcessingFormat::kEndianLittle,
					isInterleaved ?
							SAudio::ProcessingFormat::kInterleaved : SAudio::ProcessingFormat::kNonInterleaved));
		}
	}

	return OV<SAudio::ProcessingFormat>();
}

//----------------------------------------------------------------------------------------------------------------------
SAudioProcessingFormats sComposeAudioProcessingFormats(const SAudio::ProcessingSetup& sourceAudioProcessingSetup,
		const SAudio::ProcessingSetup& destinationAudioProcessingSetup,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SAudio::ProcessingSetup::BitsInfo&			sourceBitsInfo = sourceAudioProcessingSetup.getBitsInfo();
	const	SAudio::ProcessingSetup::SampleRateInfo&	sourceSampleRateInfo =
																sourceAudioProcessingSetup.getSampleRateInfo();
	const	SAudio::ProcessingSetup::ChannelMapInfo&	sourceChannelMapInfo =
																sourceAudioProcessingSetup.getChannelMapInfo();
			SAudio::ProcessingSetup::SampleTypeOption	sourceSampleTypeOption =
																sourceAudioProcessingSetup.getSampleTypeOption();
			SAudio::ProcessingSetup::EndianOption		sourceEndianOption =
																sourceAudioProcessingSetup.getEndianOption();
			SAudio::ProcessingSetup::InterleavedOption	sourceInterleavedOption =
																sourceAudioProcessingSetup.getInterleavedOption();

	const	SAudio::ProcessingSetup::BitsInfo&			destinationBitsInfo =
																destinationAudioProcessingSetup.getBitsInfo();
	const	SAudio::ProcessingSetup::SampleRateInfo&	destinationSampleRateInfo =
																destinationAudioProcessingSetup.getSampleRateInfo();
	const	SAudio::ProcessingSetup::ChannelMapInfo&	destinationChannelMapInfo =
																destinationAudioProcessingSetup.getChannelMapInfo();
			SAudio::ProcessingSetup::SampleTypeOption	destinationSampleTypeOption =
																destinationAudioProcessingSetup.getSampleTypeOption();
			SAudio::ProcessingSetup::EndianOption		destinationEndianOption =
																destinationAudioProcessingSetup.getEndianOption();
			SAudio::ProcessingSetup::InterleavedOption	destinationInterleavedOption =
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
	SAudio::ChannelMap	sourceAudioChannelMap((UInt8) 0), destinationAudioChannelMap((UInt8) 0);
	if (!sourceChannelMapInfo.isSpecified() && !destinationChannelMapInfo.isSpecified())
		// Neither source nor destination is specified, use processing channel map
		sourceAudioChannelMap = destinationAudioChannelMap = audioProcessingFormat.getChannelMap();
	else if (!sourceChannelMapInfo.isSpecified())
		// Source is not specified, use destination channel map
		sourceAudioChannelMap = destinationAudioChannelMap = destinationChannelMapInfo.getValue();
	else if (!destinationChannelMapInfo.isSpecified())
		// Destination is not specified, use source channel map
		sourceAudioChannelMap = destinationAudioChannelMap = sourceChannelMapInfo.getValue();
	else {
		// Both are specified
		sourceAudioChannelMap = sourceChannelMapInfo.getValue();
		destinationAudioChannelMap = destinationChannelMapInfo.getValue();
	}

	// Setup sample type
	bool	sourceIsFloat, destinationIsFloat;
	if (!sourceAudioProcessingSetup.isSampleTypeOptionSpecified() &&
			!destinationAudioProcessingSetup.isSampleTypeOptionSpecified())
		// Neither source nor destination is specified, use processing sample type
		sourceIsFloat = destinationIsFloat = audioProcessingFormat.getIsFloat();
	else if (!sourceAudioProcessingSetup.isSampleTypeOptionSpecified())
		// Source is not specified, use destination sample type
		sourceIsFloat = destinationIsFloat = destinationSampleTypeOption == SAudio::ProcessingSetup::kSampleTypeFloat;
	else if (!destinationAudioProcessingSetup.isSampleTypeOptionSpecified())
		// Destination is not specified, use source sample type
		sourceIsFloat = destinationIsFloat = sourceSampleTypeOption == SAudio::ProcessingSetup::kSampleTypeFloat;
	else {
		// Both are specified
		sourceIsFloat = sourceSampleTypeOption == SAudio::ProcessingSetup::kSampleTypeFloat;
		destinationIsFloat = destinationSampleTypeOption == SAudio::ProcessingSetup::kSampleTypeFloat;
	}

	// Setup endian
	bool	sourceIsBigEndian, destinationIsBigEndian;
	if (!sourceAudioProcessingSetup.isEndianOptionSpecified() &&
			!destinationAudioProcessingSetup.isEndianOptionSpecified())
		// Neither source nor destination is specified, use processing endian
		sourceIsBigEndian = destinationIsBigEndian = audioProcessingFormat.getIsBigEndian();
	else if (!sourceAudioProcessingSetup.isEndianOptionSpecified())
		// Source is not specified, use destination endian
		sourceIsBigEndian = destinationIsBigEndian = destinationEndianOption == SAudio::ProcessingSetup::kEndianBig;
	else if (!destinationAudioProcessingSetup.isEndianOptionSpecified())
		// Destination is not specified, use source endian
		sourceIsBigEndian = destinationIsBigEndian = sourceEndianOption == SAudio::ProcessingSetup::kEndianBig;
	else {
		// Both are specified
		sourceIsBigEndian = sourceEndianOption == SAudio::ProcessingSetup::kEndianBig;
		destinationIsBigEndian = destinationEndianOption == SAudio::ProcessingSetup::kEndianBig;
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
				destinationInterleavedOption == SAudio::ProcessingSetup::kInterleaved;
	else if (!destinationAudioProcessingSetup.isInterleavedOptionSpecified())
		// Destination is not specified, use source interleaved
		sourceIsInterleaved = destinationIsInterleaved =
				sourceInterleavedOption == SAudio::ProcessingSetup::kInterleaved;
	else {
		// Both are specified
		sourceIsInterleaved = sourceInterleavedOption == SAudio::ProcessingSetup::kInterleaved;
		destinationIsInterleaved = destinationInterleavedOption == SAudio::ProcessingSetup::kInterleaved;
	}

	return SAudioProcessingFormats(
			SAudio::ProcessingFormat(sourceBits, sourceSampleRate, sourceAudioChannelMap,
					sourceIsFloat ?
							SAudio::ProcessingFormat::kSampleTypeFloat :
							SAudio::ProcessingFormat::kSampleTypeSignedInteger,
					sourceIsBigEndian ? SAudio::ProcessingFormat::kEndianBig : SAudio::ProcessingFormat::kEndianLittle,
					sourceIsInterleaved ?
							SAudio::ProcessingFormat::kInterleaved : SAudio::ProcessingFormat::kNonInterleaved),
			SAudio::ProcessingFormat(destinationBits, destinationSampleRate, destinationAudioChannelMap,
					destinationIsFloat ?
							SAudio::ProcessingFormat::kSampleTypeFloat :
							SAudio::ProcessingFormat::kSampleTypeSignedInteger,
					destinationIsBigEndian ?
							SAudio::ProcessingFormat::kEndianBig : SAudio::ProcessingFormat::kEndianLittle,
					destinationIsInterleaved ?
							SAudio::ProcessingFormat::kInterleaved : SAudio::ProcessingFormat::kNonInterleaved));
}
