//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationServices.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaFoundationServices.h"

#include <mfapi.h>
#include <Mferror.h>
#include <strsafe.h>

/*
	Media Foundation is, quite simply, a royal pain to deal with.  It works by setting input and output IMFMediaTypes
		on IMFTransforms.  Trick is that Media Services doesn't necessarily help you much with that.  In some cases you
		can iterate ?all? the possibilities so you can pick the one you want.  In other cases, iterating only returns
		some of the possiblities and may not include the combination you need.  When trying to compose one, it's
		required to assemble the particular combination of properties to make the codec happy, and if it's not, there's
		no additional information as to why.  Often we have iterated all the possiblities to try and determine those set
		of properties so that in the code we can just compose it straight away.

		So, in some cases, we just make one straight way, likely becauwe we painstakingly iterated the possibilities to
		determine the required set of properties.  In other cases, we iterate the possiblities looking for the one we
		want.
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMediaFoundationServices"));
static	SError	sNoMatchingInputMediaType(sErrorDomain, 1, CString(OSSTR("No matching input media type")));
static	SError	sNoMatchingOutputMediaType(sErrorDomain, 2, CString(OSSTR("No matching output media type")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

LPCWSTR sGetGUIDNameConst(const GUID& guid);
HRESULT sGetGUIDName(const GUID& guid, WCHAR **ppwsz);

HRESULT sLogAttributeValueByIndex(IMFAttributes *pAttr, UINT32 index);
HRESULT sSpecialCaseAttributeValue(GUID guid, const PROPVARIANT& var);

void sDBGMSG(PCWSTR format, ...);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationServices

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFTransform> CMediaFoundationServices::createTransform(MFT_REGISTER_TYPE_INFO registerTypeInfo,
		GUID guidCategory, UINT32 flags)
//----------------------------------------------------------------------------------------------------------------------
{
	// Enum to find matchs
	IMFActivate**			activates;
	UINT32					count;
	HRESULT					result = ::MFTEnumEx(guidCategory, flags, NULL, &registerTypeInfo, &activates, &count);
	ReturnValueIfFailed(result, OSSTR("MFTEnumEx"), TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	// Create the transform
	IMFTransform*	transform;
	result = activates[0]->ActivateObject(IID_PPV_ARGS(&transform));

	for (UINT32 i = 0; i < count; i++)
		// Release
		activates[i]->Release();
	::CoTaskMemFree(activates);

	ReturnValueIfFailed(result, OSSTR("ActivateObject"), TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	return TCIResult<IMFTransform>(transform);
}

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFTransform> CMediaFoundationServices::createAudioTransform(GUID guidCategory, GUID guid)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	MFT_REGISTER_TYPE_INFO	registerTypeInfo = {MFMediaType_Audio, guid};

	return createTransform(registerTypeInfo, guidCategory,
			MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaFoundationServices::iterateOutputMediaTypes(IMFTransform* transform,
		std::function<void(IMFMediaType* mediaType, Float32 sampleRate,
				const SAudio::ChannelMap& audioChannelMap, UInt32 bitrate)> mediaTypeProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all available output types
	DWORD	index = 0;
	while (true) {
		// Get next media type
		IMFMediaType*	mediaType;
		HRESULT	result = transform->GetOutputAvailableType(0, index++, &mediaType);
		if (result != S_OK)
			// All done
			break;

		// Get details
		UINT32	sampleRate;
		result = mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
		if (result != S_OK)
			// Error
			continue;

		UINT32	channels;
		result = mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
		if (result != S_OK)
			// Error
			continue;

		UINT32	bitrate;
		result = mediaType->GetUINT32(MF_MT_AVG_BITRATE, &bitrate);
		if (result != S_OK)
			// Error
			continue;

		// Call proc
		mediaTypeProc(mediaType, (Float32) sampleRate, SAudio::ChannelMap::fromRawValue((UInt16) channels), bitrate);
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::setInputType(IMFTransform* transform, const GUID& guid, UInt8 bits,
		Float32 sampleRate, const SAudio::ChannelMap& audioChannelMap, const OV<UInt32>& bytesPerFrame,
		const OV<UInt32>& bytesPerSecond, const OV<CData>& userData, CreateAudioMediaTypeOptions options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all available output types
	DWORD			index = 0;
	HRESULT			result;
	IMFMediaType*	mediaType;
	while (true) {
		// Get next media type
		result = transform->GetInputAvailableType(0, index++, &mediaType);
		if (result != S_OK)
			// No match
			break;

		// Get details
		GUID	testGUID;
		result = mediaType->GetGUID(MF_MT_SUBTYPE, &testGUID);
		if (result != S_OK)
			// Error
			continue;
		if (testGUID != guid)
			// Not a match
			continue;

		UINT32	testBits;
		result = mediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &testBits);
		if (result == S_OK) {
			// Check value
			if ((UInt8) testBits != bits)
				// Not a match
				continue;
		} else if (result == MF_E_ATTRIBUTENOTFOUND) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bits);
			if (result != S_OK)
				// Unable to set
				continue;
		} else
			// Error
			continue;

		UINT32	testSampleRate;
		result = mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &testSampleRate);
		if (result == S_OK) {
			// Check value
			if ((Float32) testSampleRate != sampleRate)
				// Not a match
				continue;
		} else if (result == MF_E_ATTRIBUTENOTFOUND) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, (UINT32) sampleRate);
			if (result != S_OK)
				// Unable to set
				continue;
		} else
			// Error
			continue;

		UINT32	testChannels;
		result = mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &testChannels);
		if (result == S_OK) {
			// Check value
			if ((UInt8) testChannels != audioChannelMap.getChannelCount())
				// Not a match
				continue;
		} else if (result == MF_E_ATTRIBUTENOTFOUND) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audioChannelMap.getChannelCount());
			if (result != S_OK)
				// Unable to set
				continue;
		} else
			// Error
			continue;

		UINT32	preferWAVEFORMATEX;
		result = mediaType->GetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, &preferWAVEFORMATEX);
		if ((result != S_OK) && (options & kCreateAudioMediaTypeOptionsPreferWAVEFORMATEX))
			// Prefers WAVEFORMATEX but can't get property value
			continue;
		if ((preferWAVEFORMATEX == 1) !=
				((options & kCreateAudioMediaTypeOptionsPreferWAVEFORMATEX) ==
						kCreateAudioMediaTypeOptionsPreferWAVEFORMATEX))
			// Not a match
			continue;

		if (bytesPerFrame.hasValue()) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, *bytesPerFrame);
			if (result != S_OK)
				// Unable to set
				continue;
		}

		if (bytesPerSecond.hasValue()) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, *bytesPerSecond);
			if (result != S_OK)
				// Unable to set
				continue;
		}

		// Finalize
		if (userData.hasValue()) {
			// Set User Data
			result =
					mediaType->SetBlob(MF_MT_USER_DATA, (const UINT8*) userData->getBytePtr(),
							(UINT32) userData->getByteCount());
			ReturnErrorIfFailed(result, OSSTR("SetBlob of MF_MT_USER_DATA"));
		}

		// Set output
		result = transform->SetInputType(0, mediaType, 0);
		ReturnErrorIfFailed(result, OSSTR("SetInputType()"));

		return OV<SError>();
	}

	// Try to create media type
	result = ::MFCreateMediaType(&mediaType);
	ReturnErrorIfFailed(result, OSSTR("MFCreateMediaType"));
	CI<IMFMediaType>	mediaTypeInstance(mediaType);

	result = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	ReturnErrorIfFailed(result, OSSTR("SetGUID of MF_MT_MAJOR_TYPE"));

	result = mediaType->SetGUID(MF_MT_SUBTYPE, guid);
	ReturnErrorIfFailed(result, OSSTR("SetGUID of MF_MT_SUBTYPE"));

	result = mediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bits);
	ReturnErrorIfFailed(result, OSSTR("SetUINT32 of MF_MT_AUDIO_BITS_PER_SAMPLE"));

	result = mediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, (UINT32) sampleRate);
	ReturnErrorIfFailed(result, OSSTR("SetUINT32 of MF_MT_AUDIO_SAMPLES_PER_SECOND"));

	result = mediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audioChannelMap.getChannelCount());
	ReturnErrorIfFailed(result, OSSTR("SetUINT32 of MF_MT_AUDIO_NUM_CHANNELS"));

	if (bytesPerFrame.hasValue()) {
		result = mediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, *bytesPerFrame);
		ReturnErrorIfFailed(result, OSSTR("SetUINT32 of MF_MT_AUDIO_BLOCK_ALIGNMENT"));
	}

	if (bytesPerSecond.hasValue()) {
		result = mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, *bytesPerSecond);
		ReturnErrorIfFailed(result, OSSTR("SetUINT32 of MF_MT_AUDIO_AVG_BYTES_PER_SECOND"));
	}

	if (options & kCreateAudioMediaTypeOptionsPreferWAVEFORMATEX) {
		result = mediaType->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, 1);
		ReturnErrorIfFailed(result, OSSTR("SetUINT32 of MF_MT_AUDIO_PREFER_WAVEFORMATEX"));
	}

	if (userData.hasValue()) {
		result =
				mediaType->SetBlob(MF_MT_USER_DATA, (const UINT8*) userData->getBytePtr(),
						(UINT32) userData->getByteCount());
		ReturnErrorIfFailed(result, OSSTR("SetBlob of MF_MT_USER_DATA"));
	}

	// Set input type
	result = transform->SetInputType(0, mediaType, 0);
	ReturnErrorIfFailed(result, OSSTR("SetInputType"));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::setInputType(IMFTransform* transform,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	return setInputType(transform, audioProcessingFormat.getIsFloat() ? MFAudioFormat_Float : MFAudioFormat_PCM,
			audioProcessingFormat.getBits(), audioProcessingFormat.getSampleRate(),
			audioProcessingFormat.getChannelMap(), OV<UInt32>(audioProcessingFormat.getBytesPerFrame()),
			OV<UInt32>(audioProcessingFormat.getBytesPerFrame() * (UInt32) audioProcessingFormat.getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::setOutputType(IMFTransform* transform, const GUID& guid, const OV<UInt8>& bits,
		Float32 sampleRate, const SAudio::ChannelMap& audioChannelMap, const OV<UInt32>& bitrate,
		const OV<UInt32>& bytesPerFrame, const OV<UInt32>& bytesPerSecond,
		std::function<bool(IMFMediaType* mediaType)> isApplicableProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all available output types
	DWORD	index = 0;
	while (true) {
		// Get next media type
		IMFMediaType*	mediaType;
		HRESULT			result = transform->GetOutputAvailableType(0, index++, &mediaType);
		if (result != S_OK)
			// All done
			return OV<SError>(sNoMatchingOutputMediaType);

		// Check if is applicable
		if (!isApplicableProc(mediaType))
			// Not applicable
			continue;

		// Get details
		GUID	testGUID;
		result = mediaType->GetGUID(MF_MT_SUBTYPE, &testGUID);
		if (result != S_OK)
			// Error
			continue;
		if (testGUID != guid)
			// Not a match
			continue;

		if (bits.hasValue()) {
			// Check bits
			UINT32	testBits;
			result = mediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &testBits);
			if (result == S_OK) {
				// Check value
				if (testBits != *bits)
					// Not a match
					continue;
			} else if (result == MF_E_ATTRIBUTENOTFOUND) {
				// Set
				result = mediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, *bits);
				if (result != S_OK)
					// Unable to set
					continue;
			} else
				// Error
				continue;
		}

		UINT32	testSampleRate;
		result = mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &testSampleRate);
		if (result == S_OK) {
			// Check value
			if ((Float32) testSampleRate != sampleRate)
				// Not a match
				continue;
		} else if (result == MF_E_ATTRIBUTENOTFOUND) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, (UINT32) sampleRate);
			if (result != S_OK)
				// Unable to set
				continue;
		} else
			// Error
			continue;

		UINT32	testChannels;
		result = mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &testChannels);
		if (result == S_OK) {
			// Check value
			if ((UInt8) testChannels != audioChannelMap.getChannelCount())
				// Not a match
				continue;
		} else if (result == MF_E_ATTRIBUTENOTFOUND) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audioChannelMap.getChannelCount());
			if (result != S_OK)
				// Unable to set
				continue;
		} else
			// Error
			continue;

		if (bitrate.hasValue()) {
			// Check bitrate
			UINT32	testBitrate;
			result = mediaType->GetUINT32(MF_MT_AVG_BITRATE, &testBitrate);
			if (result != S_OK)
				// Error
				continue;
			if (testBitrate != *bitrate)
				// Not a match
				continue;
		}

		if (bytesPerFrame.hasValue()) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, *bytesPerFrame);
			if (result != S_OK)
				// Unable to set
				continue;
		}

		if (bytesPerSecond.hasValue()) {
			// Set
			result = mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, *bytesPerSecond);
			if (result != S_OK)
				// Unable to set
				continue;
		}

		// Set output
		result = transform->SetOutputType(0, mediaType, 0);
		ReturnErrorIfFailed(result, OSSTR("SetOutputType()"));

		return OV<SError>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::setOutputType(IMFTransform* transform,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	return setOutputType(transform, audioProcessingFormat.getIsFloat() ? MFAudioFormat_Float : MFAudioFormat_PCM,
			OV<UInt8>(audioProcessingFormat.getBits()), audioProcessingFormat.getSampleRate(),
			audioProcessingFormat.getChannelMap(), OV<UInt32>(), OV<UInt32>(audioProcessingFormat.getBytesPerFrame()),
			OV<UInt32>(audioProcessingFormat.getBytesPerFrame() * (UInt32) audioProcessingFormat.getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CMediaFoundationServices::createSample(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Create sample
	IMFSample*	tempSample;
	result = ::MFCreateSample(&tempSample);
	ReturnValueIfFailed(result, OSSTR("MFCreateSample"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));
	TCIResult<IMFSample>	sample(tempSample);

	// Create memory buffer
	IMFMediaBuffer*	tempMediaBuffer;
	result = ::MFCreateMemoryBuffer(size, &tempMediaBuffer);
	ReturnValueIfFailed(result, OSSTR("MFCreateMemoryBuffer"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));
	CI<IMFMediaBuffer>	mediaBuffer(tempMediaBuffer);

	// Add buffer to sample
	result = sample->AddBuffer(*mediaBuffer);
	ReturnValueIfFailed(result, OSSTR("AddBuffer"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	return sample;
}

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CMediaFoundationServices::createSample(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Create sample
	IMFSample*	tempSample;
	result = ::MFCreateSample(&tempSample);
	ReturnValueIfFailed(result, OSSTR("MFCreateSample"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));
	TCIResult<IMFSample>	sample(tempSample);

	// Create memory buffer
	IMFMediaBuffer*	tempMediaBuffer;
	result = ::MFCreateMemoryBuffer((DWORD) data.getByteCount(), &tempMediaBuffer);
	ReturnValueIfFailed(result, OSSTR("MFCreateMemoryBuffer"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));
	CI<IMFMediaBuffer>	mediaBuffer(tempMediaBuffer);

	// Add buffer to sample
	result = sample->AddBuffer(*mediaBuffer);
	ReturnValueIfFailed(result, OSSTR("AddBuffer"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	// Lock media buffer
	BYTE*	bytePtr;
	DWORD	length;
	result = tempMediaBuffer->Lock(&bytePtr, NULL, &length);
	ReturnValueIfFailed(result, OSSTR("Lock for media buffer"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	// Copy data
	::memcpy(bytePtr, data.getBytePtr(), (size_t) data.getByteCount());

	// Set current length
	result = tempMediaBuffer->SetCurrentLength((DWORD) data.getByteCount());
	ReturnValueIfFailed(result, OSSTR("SetCurrentLength"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	// Unlock media buffer
	result = tempMediaBuffer->Unlock();
	ReturnValueIfFailed(result, OSSTR("Unlock for media buffer"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	return sample;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::resizeSample(IMFSample* sample, UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove existing memory buffer
	HRESULT	result = sample->RemoveAllBuffers();

	// Create memory buffer
	IMFMediaBuffer*	tempMediaBuffer;
	result = ::MFCreateMemoryBuffer(size, &tempMediaBuffer);
	ReturnErrorIfFailed(result, OSSTR("MFCreateMemoryBuffer in CH264VideoCodecInternals::noteFormatChanged"));
	CI<IMFMediaBuffer>	mediaBuffer(tempMediaBuffer);

	// Add buffer to sample
	result = sample->AddBuffer(*mediaBuffer);
	ReturnErrorIfFailed(result, OSSTR("AddBuffer in CH264VideoCodecInternals::noteFormatChanged"));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAudioProcessor::SourceInfo> CMediaFoundationServices::load(IMFMediaBuffer* mediaBuffer,
		CAudioProcessor& audioProcessor, const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	bytesPerFrame = audioProcessingFormat.getBytesPerFrame();

	// Lock media buffer
	BYTE*	mediaBufferBytePtr;
	DWORD	mediaBufferByteCount;
	HRESULT	result = mediaBuffer->Lock(&mediaBufferBytePtr, &mediaBufferByteCount, NULL);
	ReturnValueIfFailed(result, OSSTR("Lock"), TVResult<CAudioProcessor::SourceInfo>(SErrorFromHRESULT(result)));

	// Setup Audio Frames
	CAudioFrames	audioFrames(mediaBufferBytePtr, 1, mediaBufferByteCount, mediaBufferByteCount / bytesPerFrame,
							bytesPerFrame);

	// Perform into
	TVResult<CAudioProcessor::SourceInfo>	audioProcessorSourceInfo =
													audioProcessor.CAudioProcessor::performInto(audioFrames);
	if (audioProcessorSourceInfo.hasError()) {
		// Error
		mediaBuffer->Unlock();

		return audioProcessorSourceInfo;
	}

	// Check if need to transmogrify audio frames
	if (audioProcessingFormat.getBits() == 8)
		// Toggle 8 bit
		audioFrames.toggle8BitSignedUnsigned();
	else if (audioProcessingFormat.getIsBigEndian())
		// Toggle endianness
		audioFrames.toggleEndianness(audioProcessingFormat.getBits());

	// Update current length
	result = mediaBuffer->SetCurrentLength(audioFrames.getCurrentFrameCount() * bytesPerFrame);
	if (FAILED(result)) {
		// Unlock
		mediaBuffer->Unlock();

		ReturnValueIfFailed(result, OSSTR("SetCurrentLength"),
				TVResult<CAudioProcessor::SourceInfo>(SErrorFromHRESULT(result)));
	}

	// Unlock media buffer
	result = mediaBuffer->Unlock();
	ReturnValueIfFailed(result, OSSTR("Unlock"), TVResult<CAudioProcessor::SourceInfo>(SErrorFromHRESULT(result)));

	return audioProcessorSourceInfo;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::load(IMFMediaBuffer* mediaBuffer, CMediaPacketSource& mediaPacketSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Lock media buffer
	BYTE*	mediaBufferBytePtr;
	DWORD	mediaBufferByteCount;
	HRESULT	result = mediaBuffer->Lock(&mediaBufferBytePtr, &mediaBufferByteCount, NULL);
	ReturnErrorIfFailed(result, OSSTR("Lock"));

	// Read next media packet
	CData								data(mediaBufferBytePtr, mediaBufferByteCount, false);
	TVResult<TArray<SMedia::Packet> >	mediaPackets = mediaPacketSource.readNextInto(data, 1);
	if (mediaPackets.hasError()) {
		// Unlock
		mediaBuffer->Unlock();

		return OV<SError>(mediaPackets.getError());
	}

	// Update current length
	result = mediaBuffer->SetCurrentLength((DWORD) (*mediaPackets)[0].getByteCount());
	if (FAILED(result)) {
		// Unlock
		mediaBuffer->Unlock();

		ReturnError(result, OSSTR("SetCurrentLength"));
	}

	// Unlock media buffer
	result = mediaBuffer->Unlock();
	ReturnErrorIfFailed(result, OSSTR("Unlock"));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CImage> CMediaFoundationServices::imageForVideoSample(const CVideoFrame& videoFrame)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get 
	IMFMediaBuffer*	mediaBuffer;
	HRESULT			result = videoFrame.getSample()->GetBufferByIndex(0, &mediaBuffer);
	ReturnValueIfFailed(result, OSSTR("GetBufferByIndex for outputSample"),
			TVResult<CImage>(SErrorFromHRESULT(result)));

	BYTE*	mediaBufferBytePtr;
	DWORD	mediaBufferByteCount;
	result = mediaBuffer->Lock(&mediaBufferBytePtr, NULL, &mediaBufferByteCount);
	if (FAILED(result)) {
		// Failed
		LogFailedHRESULT(result, OSSTR("Lock for outputSample"));

		// Cleanup
		mediaBuffer->Release();

		return TVResult<CImage>(SErrorFromHRESULT(result));
	}

	// Create image
	const	S2DSizeU16&	size = videoFrame.getFrameSize();
			CImage		image(CData(mediaBufferBytePtr, mediaBufferByteCount), OV<CImage::Type>(CImage::kTypeNV12),
								S2DSizeS32(size.mWidth, size.mHeight));

	// Unlock
	result = mediaBuffer->Unlock();
	if (FAILED(result)) {
		// Failed
		LogFailedHRESULT(result, OSSTR("Unlock for outputSample"));

		// Cleanup
		mediaBuffer->Release();

		return TVResult<CImage>(SErrorFromHRESULT(result));
	}

	// Cleanup
	mediaBuffer->Release();

	return TVResult<CImage>(image);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::processOutput(IMFTransform* transform, IMFSample* outputSample,
		const CMediaFoundationServices::ProcessOutputInfo& processOutputInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Run as long as needed
	while (true) {
		// Process output
		MFT_OUTPUT_DATA_BUFFER	outputDataBuffer = {0, outputSample, 0, NULL};
		DWORD					status = 0;
		result = transform->ProcessOutput(0, 1, &outputDataBuffer, &status);
		if (result == S_OK)
			// Success
			return OV<SError>();
		else if (result == MF_E_TRANSFORM_STREAM_CHANGE) {
			// Input stream format change
			if (outputDataBuffer.dwStatus == MFT_OUTPUT_DATA_BUFFER_FORMAT_CHANGE) {
				// Output data buffer format change
				IMFMediaType*	mediaType = NULL;
				result = transform->GetOutputAvailableType(0, 0, &mediaType);
				ReturnErrorIfFailed(result, OSSTR("GetOutputAvailableType for format change"));

				// Call proc
				OV<SError>	error = processOutputInfo.noteFormatChanged(mediaType);
				ReturnErrorIfError(error);

				// Set output type
				result = transform->SetOutputType(0, mediaType, 0);
				ReturnErrorIfFailed(result, OSSTR("SetOutputType for format change"));
			} else
				// Unexpected setup
				ReturnError(E_NOTIMPL, "Input stream format changed, but no updated output format given");
		} else if (result == MF_E_TRANSFORM_NEED_MORE_INPUT) {
			// Get input sample
			TCIResult<IMFSample>	inputSample = processOutputInfo.getInputSample();
			ReturnErrorIfResultError(inputSample);

			// Process input
			result = transform->ProcessInput(0, *inputSample.getInstance(), 0);
			ReturnErrorIfFailed(result, OSSTR("ProcessInput"));
		} else
			// Error
			ReturnError(result, OSSTR("ProcessOutput"));
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt32> CMediaFoundationServices::completeWrite(IMFSample* sample, UInt32 frameOffset,
		CAudioFrames& audioFrames, const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Copy
	IMFMediaBuffer*	mediaBuffer;
	HRESULT			result = sample->GetBufferByIndex(0, &mediaBuffer);
	ReturnValueIfFailed(result, OSSTR("GetBufferByIndex for outputSample"),
			TVResult<UInt32>(SErrorFromHRESULT(result)));

	BYTE*	mediaBufferBytePtr;
	DWORD	mediaBufferByteCount;
	result = mediaBuffer->Lock(&mediaBufferBytePtr, NULL, &mediaBufferByteCount);
	if (FAILED(result)) {
		// Failed
		LogFailedHRESULT(result, OSSTR("Lock for outputSample"));

		// Cleanup
		mediaBuffer->Release();

		return TVResult<UInt32>(SErrorFromHRESULT(result));
	}

	// Copy bytes
	UInt32	processedFrameCount = mediaBufferByteCount / audioProcessingFormat.getBytesPerFrame();
	UInt32	copyByteOffset = frameOffset * audioProcessingFormat.getBytesPerFrame();
	if (copyByteOffset < mediaBufferByteCount) {
		// Copy
		UInt32				copyByteCount = mediaBufferByteCount - copyByteOffset;
		CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();
		::memcpy(writeInfo.getSegments()[0], mediaBufferBytePtr + copyByteOffset, copyByteCount);

		// Complete write
		audioFrames.completeWrite(copyByteCount / audioProcessingFormat.getBytesPerFrame());
	}

	// Reset buffer
	result = mediaBuffer->SetCurrentLength(0);
	if (FAILED(result)) {
		// Failed
		LogFailedHRESULT(result, OSSTR("SetCurrentLength"));

		// Cleanup
		mediaBuffer->Release();

		return TVResult<UInt32>(SErrorFromHRESULT(result));
	}

	result = mediaBuffer->Unlock();
	if (FAILED(result)) {
		// Failed
		LogFailedHRESULT(result, OSSTR("Unlock for outputSample"));

		// Cleanup
		mediaBuffer->Release();

		return TVResult<UInt32>(SErrorFromHRESULT(result));
	}

	// Cleanup
	mediaBuffer->Release();

	return TVResult<UInt32>(processedFrameCount);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::flush(IMFTransform* transform)
//----------------------------------------------------------------------------------------------------------------------
{
	HRESULT	result = transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
	ReturnErrorIfFailed(result, OSSTR("ProcessMessage MFT_MESSAGE_COMMAND_FLUSH"));

	return OV<SError>();
}


#if defined(DEBUG)
//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationServices::log(IMFMediaType* mediaType)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Get attribute count
	UINT32	attributeCount;
	result = mediaType->GetCount(&attributeCount);
	ReturnErrorIfFailed(result, OSSTR("GetCount()"));

	// Check attribute count
	if (attributeCount > 0)
		// Log attributes
		for (UINT32 i = 0; i < attributeCount; i++) {
			// Log attribute
			result = sLogAttributeValueByIndex(mediaType, i);
			ReturnErrorIfFailed(result, OSSTR("sLogAttributeValueByIndex()"));
		}
	else
		CLogServices::logMessage(OSSTR("Empty media type."));

	return OV<SError>();
}
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

// TODO: Cleanup these procs

//----------------------------------------------------------------------------------------------------------------------
HRESULT sLogAttributeValueByIndex(IMFAttributes* pAttr, UINT32 index)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup

	WCHAR *pGuidName = NULL;
	WCHAR *pGuidValName = NULL;

	GUID guid = { 0 };

	PROPVARIANT var;
	::PropVariantInit(&var);

	HRESULT hr = pAttr->GetItemByIndex(index, &guid, &var);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = sGetGUIDName(guid, &pGuidName);
	if (FAILED(hr))
	{
		goto done;
	}

	sDBGMSG(L"\t%s\t", pGuidName);

	hr = sSpecialCaseAttributeValue(guid, var);
	if (FAILED(hr))
	{
		goto done;
	}
	if (hr == S_FALSE)
	{
		switch (var.vt)
		{
		case VT_UI4:
			sDBGMSG(L"%d", var.ulVal);
			break;

		case VT_UI8:
			sDBGMSG(L"%I64d", var.uhVal);
			break;

		case VT_R8:
			sDBGMSG(L"%f", var.dblVal);
			break;

		case VT_CLSID:
			hr = sGetGUIDName(*var.puuid, &pGuidValName);
			if (SUCCEEDED(hr))
			{
				sDBGMSG(pGuidValName);
			}
			break;

		case VT_LPWSTR:
			sDBGMSG(var.pwszVal);
			break;

		case VT_VECTOR | VT_UI1:
			sDBGMSG(L"<<byte array>>");
			break;

		case VT_UNKNOWN:
			sDBGMSG(L"IUnknown");
			break;

		default:
			sDBGMSG(L"Unexpected attribute type (vt = %d)", var.vt);
			break;
		}
	}

done:
	sDBGMSG(L"\n");
	::CoTaskMemFree(pGuidName);
	::CoTaskMemFree(pGuidValName);
	::PropVariantClear(&var);
	return hr;
}

//----------------------------------------------------------------------------------------------------------------------
HRESULT sGetGUIDName(const GUID& guid, WCHAR **ppwsz)
//----------------------------------------------------------------------------------------------------------------------
{
	HRESULT hr = S_OK;
	WCHAR *pName = NULL;

	LPCWSTR pcwsz = sGetGUIDNameConst(guid);
	if (pcwsz)
	{
		size_t cchLength = 0;
	
		hr = StringCchLength(pcwsz, STRSAFE_MAX_CCH, &cchLength);
		if (FAILED(hr))
		{
			goto done;
		}
		
		pName = (WCHAR*) ::CoTaskMemAlloc((cchLength + 1) * sizeof(WCHAR));

		if (pName == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto done;
		}

		hr = StringCchCopy(pName, cchLength + 1, pcwsz);
		if (FAILED(hr))
		{
			goto done;
		}
	}
	else
	{
		hr = ::StringFromCLSID(guid, &pName);
	}

done:
	if (FAILED(hr))
	{
		*ppwsz = NULL;
		::CoTaskMemFree(pName);
	}
	else
	{
		*ppwsz = pName;
	}
	return hr;
}

//----------------------------------------------------------------------------------------------------------------------
void LogUINT32AsUINT64(const PROPVARIANT& var)
//----------------------------------------------------------------------------------------------------------------------
{
	UINT32 uHigh = 0, uLow = 0;
	::Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);
	sDBGMSG(L"%d x %d", uHigh, uLow);
}

//----------------------------------------------------------------------------------------------------------------------
float OffsetToFloat(const MFOffset& offset)
//----------------------------------------------------------------------------------------------------------------------
{
	return offset.value + (static_cast<float>(offset.fract) / 65536.0f);
}

//----------------------------------------------------------------------------------------------------------------------
HRESULT LogVideoArea(const PROPVARIANT& var)
//----------------------------------------------------------------------------------------------------------------------
{
	if (var.caub.cElems < sizeof(MFVideoArea))
	{
		return MF_E_BUFFERTOOSMALL;
	}

	MFVideoArea *pArea = (MFVideoArea*)var.caub.pElems;

	sDBGMSG(L"(%f,%f) (%d,%d)", OffsetToFloat(pArea->OffsetX), OffsetToFloat(pArea->OffsetY), 
		pArea->Area.cx, pArea->Area.cy);
	return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------
HRESULT sSpecialCaseAttributeValue(GUID guid, const PROPVARIANT& var)
//----------------------------------------------------------------------------------------------------------------------
{
	if ((guid == MF_MT_FRAME_RATE) || (guid == MF_MT_FRAME_RATE_RANGE_MAX) ||
		(guid == MF_MT_FRAME_RATE_RANGE_MIN) || (guid == MF_MT_FRAME_SIZE) ||
		(guid == MF_MT_PIXEL_ASPECT_RATIO))
	{
		// Attributes that contain two packed 32-bit values.
		::LogUINT32AsUINT64(var);
	}
	else if ((guid == MF_MT_GEOMETRIC_APERTURE) || 
			 (guid == MF_MT_MINIMUM_DISPLAY_APERTURE) || 
			 (guid == MF_MT_PAN_SCAN_APERTURE))
	{
		// Attributes that an MFVideoArea structure.
		return ::LogVideoArea(var);
	}
	else
	{
		return S_FALSE;
	}
	return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------
void sDBGMSG(PCWSTR format, ...)
//----------------------------------------------------------------------------------------------------------------------
{
	va_list args;
	va_start(args, format);

	WCHAR msg[MAX_PATH];

	if (SUCCEEDED(StringCbVPrintf(msg, sizeof(msg), format, args)))
	{
		OutputDebugString(msg);
	}
}

#ifndef IF_EQUAL_RETURN
#define IF_EQUAL_RETURN(param, val) if (val == param) return L#val
#endif

//----------------------------------------------------------------------------------------------------------------------
LPCWSTR sGetGUIDNameConst(const GUID& guid)
//----------------------------------------------------------------------------------------------------------------------
{
	IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_SUBTYPE);
	IF_EQUAL_RETURN(guid, MF_MT_ALL_SAMPLES_INDEPENDENT);
	IF_EQUAL_RETURN(guid, MF_MT_FIXED_SIZE_SAMPLES);
	IF_EQUAL_RETURN(guid, MF_MT_COMPRESSED);
	IF_EQUAL_RETURN(guid, MF_MT_SAMPLE_SIZE);
	IF_EQUAL_RETURN(guid, MF_MT_WRAPPED_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_NUM_CHANNELS);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_SECOND);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BLOCK_ALIGNMENT);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BITS_PER_SAMPLE);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_BLOCK);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_CHANNEL_MASK);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FOLDDOWN_MATRIX);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKREF);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKTARGET);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGREF);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGTARGET);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_PREFER_WAVEFORMATEX);
	IF_EQUAL_RETURN(guid, MF_MT_AAC_PAYLOAD_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION);
	IF_EQUAL_RETURN(guid, MF_MT_FRAME_SIZE);
	IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE);
	IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MAX);
	IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MIN);
	IF_EQUAL_RETURN(guid, MF_MT_PIXEL_ASPECT_RATIO);
	IF_EQUAL_RETURN(guid, MF_MT_DRM_FLAGS);
	IF_EQUAL_RETURN(guid, MF_MT_PAD_CONTROL_FLAGS);
	IF_EQUAL_RETURN(guid, MF_MT_SOURCE_CONTENT_HINT);
	IF_EQUAL_RETURN(guid, MF_MT_VIDEO_CHROMA_SITING);
	IF_EQUAL_RETURN(guid, MF_MT_INTERLACE_MODE);
	IF_EQUAL_RETURN(guid, MF_MT_TRANSFER_FUNCTION);
	IF_EQUAL_RETURN(guid, MF_MT_VIDEO_PRIMARIES);
	//IF_EQUAL_RETURN(guid, MF_MT_CUSTOM_VIDEO_PRIMARIES);
	IF_EQUAL_RETURN(guid, MF_MT_YUV_MATRIX);
	IF_EQUAL_RETURN(guid, MF_MT_VIDEO_LIGHTING);
	IF_EQUAL_RETURN(guid, MF_MT_VIDEO_NOMINAL_RANGE);
	IF_EQUAL_RETURN(guid, MF_MT_GEOMETRIC_APERTURE);
	IF_EQUAL_RETURN(guid, MF_MT_MINIMUM_DISPLAY_APERTURE);
	IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_APERTURE);
	IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_ENABLED);
	IF_EQUAL_RETURN(guid, MF_MT_AVG_BITRATE);
	IF_EQUAL_RETURN(guid, MF_MT_AVG_BIT_ERROR_RATE);
	IF_EQUAL_RETURN(guid, MF_MT_MAX_KEYFRAME_SPACING);
	IF_EQUAL_RETURN(guid, MF_MT_DEFAULT_STRIDE);
	IF_EQUAL_RETURN(guid, MF_MT_PALETTE);
	IF_EQUAL_RETURN(guid, MF_MT_USER_DATA);
	//IF_EQUAL_RETURN(guid, MF_MT_AM_FORMAT_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG_START_TIME_CODE);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG2_PROFILE);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG2_LEVEL);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG2_FLAGS);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG_SEQUENCE_HEADER);
	IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_0);
	IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_0);
	IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_1);
	IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_1);
	IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_SRC_PACK);
	IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_CTRL_PACK);
	//IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_HEADER);
	//IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_FORMAT);
	IF_EQUAL_RETURN(guid, MF_MT_IMAGE_LOSS_TOLERANT); 
	IF_EQUAL_RETURN(guid, MF_MT_MPEG4_SAMPLE_DESCRIPTION);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY);
	//IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_4CC); 
	//IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_WAVE_FORMAT_TAG);
	
	// Media types
	IF_EQUAL_RETURN(guid, MFMediaType_Audio);
	IF_EQUAL_RETURN(guid, MFMediaType_Video);
	IF_EQUAL_RETURN(guid, MFMediaType_Protected);
	IF_EQUAL_RETURN(guid, MFMediaType_SAMI);
	IF_EQUAL_RETURN(guid, MFMediaType_Script);
	IF_EQUAL_RETURN(guid, MFMediaType_Image);
	IF_EQUAL_RETURN(guid, MFMediaType_HTML);
	IF_EQUAL_RETURN(guid, MFMediaType_Binary);
	IF_EQUAL_RETURN(guid, MFMediaType_FileTransfer);

	IF_EQUAL_RETURN(guid, MFVideoFormat_AI44); //     FCC('AI44')
	IF_EQUAL_RETURN(guid, MFVideoFormat_ARGB32); //   D3DFMT_A8R8G8B8 
	IF_EQUAL_RETURN(guid, MFVideoFormat_AYUV); //     FCC('AYUV')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DV25); //     FCC('dv25')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DV50); //     FCC('dv50')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DVH1); //     FCC('dvh1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DVSD); //     FCC('dvsd')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DVSL); //     FCC('dvsl')
	IF_EQUAL_RETURN(guid, MFVideoFormat_H264); //     FCC('H264')
	IF_EQUAL_RETURN(guid, MFVideoFormat_I420); //     FCC('I420')
	IF_EQUAL_RETURN(guid, MFVideoFormat_IYUV); //     FCC('IYUV')
	IF_EQUAL_RETURN(guid, MFVideoFormat_M4S2); //     FCC('M4S2')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MJPG);
	IF_EQUAL_RETURN(guid, MFVideoFormat_MP43); //     FCC('MP43')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MP4S); //     FCC('MP4S')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MP4V); //     FCC('MP4V')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MPG1); //     FCC('MPG1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MSS1); //     FCC('MSS1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MSS2); //     FCC('MSS2')
	IF_EQUAL_RETURN(guid, MFVideoFormat_NV11); //     FCC('NV11')
	IF_EQUAL_RETURN(guid, MFVideoFormat_NV12); //     FCC('NV12')
	IF_EQUAL_RETURN(guid, MFVideoFormat_P010); //     FCC('P010')
	IF_EQUAL_RETURN(guid, MFVideoFormat_P016); //     FCC('P016')
	IF_EQUAL_RETURN(guid, MFVideoFormat_P210); //     FCC('P210')
	IF_EQUAL_RETURN(guid, MFVideoFormat_P216); //     FCC('P216')
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB24); //    D3DFMT_R8G8B8 
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB32); //    D3DFMT_X8R8G8B8 
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB555); //   D3DFMT_X1R5G5B5 
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB565); //   D3DFMT_R5G6B5 
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB8);
	IF_EQUAL_RETURN(guid, MFVideoFormat_UYVY); //     FCC('UYVY')
	IF_EQUAL_RETURN(guid, MFVideoFormat_v210); //     FCC('v210')
	IF_EQUAL_RETURN(guid, MFVideoFormat_v410); //     FCC('v410')
	IF_EQUAL_RETURN(guid, MFVideoFormat_WMV1); //     FCC('WMV1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_WMV2); //     FCC('WMV2')
	IF_EQUAL_RETURN(guid, MFVideoFormat_WMV3); //     FCC('WMV3')
	IF_EQUAL_RETURN(guid, MFVideoFormat_WVC1); //     FCC('WVC1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y210); //     FCC('Y210')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y216); //     FCC('Y216')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y410); //     FCC('Y410')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y416); //     FCC('Y416')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y41P);
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y41T);
	IF_EQUAL_RETURN(guid, MFVideoFormat_YUY2); //     FCC('YUY2')
	IF_EQUAL_RETURN(guid, MFVideoFormat_YV12); //     FCC('YV12')
	IF_EQUAL_RETURN(guid, MFVideoFormat_YVYU);

	IF_EQUAL_RETURN(guid, MFAudioFormat_PCM); //              WAVE_FORMAT_PCM 
	IF_EQUAL_RETURN(guid, MFAudioFormat_Float); //            WAVE_FORMAT_IEEE_FLOAT 
	IF_EQUAL_RETURN(guid, MFAudioFormat_DTS); //              WAVE_FORMAT_DTS 
	IF_EQUAL_RETURN(guid, MFAudioFormat_Dolby_AC3_SPDIF); //  WAVE_FORMAT_DOLBY_AC3_SPDIF 
	IF_EQUAL_RETURN(guid, MFAudioFormat_DRM); //              WAVE_FORMAT_DRM 
	IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV8); //        WAVE_FORMAT_WMAUDIO2 
	IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV9); //        WAVE_FORMAT_WMAUDIO3 
	IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudio_Lossless); // WAVE_FORMAT_WMAUDIO_LOSSLESS 
	IF_EQUAL_RETURN(guid, MFAudioFormat_WMASPDIF); //         WAVE_FORMAT_WMASPDIF 
	IF_EQUAL_RETURN(guid, MFAudioFormat_MSP1); //             WAVE_FORMAT_WMAVOICE9 
	IF_EQUAL_RETURN(guid, MFAudioFormat_MP3); //              WAVE_FORMAT_MPEGLAYER3 
	IF_EQUAL_RETURN(guid, MFAudioFormat_MPEG); //             WAVE_FORMAT_MPEG 
	IF_EQUAL_RETURN(guid, MFAudioFormat_AAC); //              WAVE_FORMAT_MPEG_HEAAC 
	IF_EQUAL_RETURN(guid, MFAudioFormat_ADTS); //             WAVE_FORMAT_MPEG_ADTS_AAC 

	return NULL;
}
