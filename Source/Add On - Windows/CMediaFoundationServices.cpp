//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationServices.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaFoundationServices.h"

#include <mfapi.h>
#include <Mferror.h>
#include <strsafe.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

HRESULT LogMediaType(IMFMediaType *pType);

LPCWSTR GetGUIDNameConst(const GUID& guid);
HRESULT GetGUIDName(const GUID& guid, WCHAR **ppwsz);

HRESULT LogAttributeValueByIndex(IMFAttributes *pAttr, UINT32 index);
HRESULT SpecialCaseAttributeValue(GUID guid, const PROPVARIANT& var);

void DBGMSG(PCWSTR format, ...);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationServices

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFTransform> CMediaFoundationServices::createTransformForAudioDecode(const GUID& guid,
		const SAudioProcessingFormat& audioProcessingFormat, const OI<CData>& userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Enum Audio Codecs to find Audio Decoder
	MFT_REGISTER_TYPE_INFO	info = {MFMediaType_Audio, guid};
	IMFActivate**			activates;
	UINT32					count;
	result =
			MFTEnumEx(MFT_CATEGORY_AUDIO_DECODER,
					MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER, &info, NULL,
					&activates, &count);
	ReturnValueIfFailed(result, "MFTEnumEx", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	// Create the Audio Decoder
	IMFTransform*	transform;
	result = activates[0]->ActivateObject(IID_PPV_ARGS(&transform));
	OCI<IMFTransform>	audioDecoder(transform);

	for (UINT32 i = 0; i < count; i++)
		// Release
		activates[i]->Release();
	CoTaskMemFree(activates);

	ReturnValueIfFailed(result, "ActivateObject", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	// Setup input media type
	IMFMediaType*	mediaType;
	result = MFCreateMediaType(&mediaType);
	ReturnValueIfFailed(result, "MFCreateMediaType for input", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));
	CI<IMFMediaType>	inputMediaType(mediaType);

	result = inputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	ReturnValueIfFailed(result, "SetGUID of MF_MT_MAJOR_TYPE for input",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = inputMediaType->SetGUID(MF_MT_SUBTYPE, guid);
	ReturnValueIfFailed(result, "SetGUID of MF_MT_SUBTYPE for input",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = inputMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 32);
	ReturnValueIfFailed(result, "SetUINT32 of MF_MT_AUDIO_BITS_PER_SAMPLE for input",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = inputMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, (UINT32) audioProcessingFormat.getSampleRate());
	ReturnValueIfFailed(result, "SetUINT32 of MF_MT_AUDIO_SAMPLES_PER_SECOND for input",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = inputMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audioProcessingFormat.getChannels());
	ReturnValueIfFailed(result, "SetUINT32 of MF_MT_AUDIO_NUM_CHANNELS for input",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	if (userData.hasInstance()) {
		result =
				inputMediaType->SetBlob(MF_MT_USER_DATA, (const UINT8*) userData->getBytePtr(),
						(UINT32) userData->getSize());
		ReturnValueIfFailed(result, "SetBlob of MF_MT_USER_DATA for input",
				TCIResult<IMFTransform>(SErrorFromHRESULT(result)));
	}

	result = audioDecoder->SetInputType(0, *inputMediaType, 0);
	ReturnValueIfFailed(result, "SetInputType", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	// Setup output media type
	result = MFCreateMediaType(&mediaType);
	ReturnValueIfFailed(result, "MFCreateMediaType for output", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));
	CI<IMFMediaType>	outputMediaType(mediaType);

	result = outputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	ReturnValueIfFailed(result, "SetGUID of MF_MT_MAJOR_TYPE for output",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = outputMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
	ReturnValueIfFailed(result, "SetGUID of MF_MT_SUBTYPE for output",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = outputMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 32);
	ReturnValueIfFailed(result, "SetSetUINT32UID of MF_MT_AUDIO_BITS_PER_SAMPLE for output",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = outputMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, (UINT32) audioProcessingFormat.getSampleRate());
	ReturnValueIfFailed(result, "SetUINT32 of MF_MT_AUDIO_SAMPLES_PER_SECOND for output",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = outputMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audioProcessingFormat.getChannels());
	ReturnValueIfFailed(result, "SetUINT32 of MF_MT_AUDIO_NUM_CHANNELS for output",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = outputMediaType->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, 1);
	ReturnValueIfFailed(result, "SetUINT32 of MF_MT_AUDIO_PREFER_WAVEFORMATEX for output",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = audioDecoder->SetOutputType(0, *outputMediaType, 0);
	ReturnValueIfFailed(result, "SetOutputType", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	return TCIResult<IMFTransform>(audioDecoder);
}

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFTransform> CMediaFoundationServices::createTransformForVideoDecode(const GUID& guid)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Enum Video Codecs to find Video Decoder
	MFT_REGISTER_TYPE_INFO	info = {MFMediaType_Video, guid};
	IMFActivate**			activates;
	UINT32					count;
	result =
			MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER,
					MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER, &info, NULL,
					&activates, &count);
	ReturnValueIfFailed(result, "MFTEnumEx", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	// Create the Video Decoder
	IMFTransform*	transform;
	result = activates[0]->ActivateObject(IID_PPV_ARGS(&transform));
	OCI<IMFTransform>	videoDecoder(transform);

	for (UINT32 i = 0; i < count; i++)
		// Release
		activates[i]->Release();
	CoTaskMemFree(activates);

	ReturnValueIfFailed(result, "ActivateObject", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	// Setup input media type
	IMFMediaType*	mediaType;
	result = MFCreateMediaType(&mediaType);
	ReturnValueIfFailed(result, "MFCreateMediaType for input", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));
	CI<IMFMediaType>	inputMediaType(mediaType);

	result = inputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	ReturnValueIfFailed(result, "SetGUID of MF_MT_MAJOR_TYPE for input",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = inputMediaType->SetGUID(MF_MT_SUBTYPE, guid);
	ReturnValueIfFailed(result, "SetGUID of MF_MT_SUBTYPE for input",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = MFSetAttributeSize(*inputMediaType, MF_MT_FRAME_SIZE, 320, 480);
	ReturnValueIfFailed(result, "MFSetAttributeSize for input",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = videoDecoder->SetInputType(0, *inputMediaType, 0);
	ReturnValueIfFailed(result, "SetInputType", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	// Setup output media type
	result = MFCreateMediaType(&mediaType);
	ReturnValueIfFailed(result, "MFCreateMediaType for output", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));
	CI<IMFMediaType>	outputMediaType(mediaType);

	result = outputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	ReturnValueIfFailed(result, "SetGUID of MF_MT_MAJOR_TYPE for output",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = outputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
	ReturnValueIfFailed(result, "SetGUID of MF_MT_SUBTYPE for output",
			TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = MFSetAttributeSize(*outputMediaType, MF_MT_FRAME_SIZE, 320, 480);
	ReturnValueIfFailed(result, "MFSetAttributeSize for output", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	result = videoDecoder->SetOutputType(0, *outputMediaType, 0);
	ReturnValueIfFailed(result, "SetOutputType", TCIResult<IMFTransform>(SErrorFromHRESULT(result)));

	return TCIResult<IMFTransform>(videoDecoder);
}

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CMediaFoundationServices::createSample(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Create sample
	IMFSample*	tempSample;
	result = MFCreateSample(&tempSample);
	ReturnValueIfFailed(result, "MFCreateSample", TCIResult<IMFSample>(SErrorFromHRESULT(result)));
	OCI<IMFSample>	sample(tempSample);

	// Create memory buffer
	IMFMediaBuffer*	tempMediaBuffer;
	result = MFCreateMemoryBuffer(size, &tempMediaBuffer);
	ReturnValueIfFailed(result, "MFCreateMemoryBuffer", TCIResult<IMFSample>(SErrorFromHRESULT(result)));
	CI<IMFMediaBuffer>	mediaBuffer(tempMediaBuffer);

	// Add buffer to sample
	result = sample->AddBuffer(*mediaBuffer);
	ReturnValueIfFailed(result, "AddBuffer", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	return TCIResult<IMFSample>(sample);
}

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CMediaFoundationServices::createSample(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Create sample
	IMFSample*	tempSample;
	result = MFCreateSample(&tempSample);
	ReturnValueIfFailed(result, "MFCreateSample", TCIResult<IMFSample>(SErrorFromHRESULT(result)));
	OCI<IMFSample>	sample(tempSample);

	// Create memory buffer
	IMFMediaBuffer*	tempMediaBuffer;
	result = MFCreateMemoryBuffer((DWORD) data.getSize(), &tempMediaBuffer);
	ReturnValueIfFailed(result, "MFCreateMemoryBuffer", TCIResult<IMFSample>(SErrorFromHRESULT(result)));
	CI<IMFMediaBuffer>	mediaBuffer(tempMediaBuffer);

	// Add buffer to sample
	result = sample->AddBuffer(*mediaBuffer);
	ReturnValueIfFailed(result, "AddBuffer", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	// Lock media buffer
	BYTE*	bytePtr;
	DWORD	length;
	result = tempMediaBuffer->Lock(&bytePtr, NULL, &length);
	ReturnValueIfFailed(result, "Lock for media buffer", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	// Copy data
	::memcpy(bytePtr, data.getBytePtr(), data.getSize());

	// Set current length
	result = tempMediaBuffer->SetCurrentLength((DWORD) data.getSize());
	ReturnValueIfFailed(result, "SetCurrentLength", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	// Unlock media buffer
	result = tempMediaBuffer->Unlock();
	ReturnValueIfFailed(result, "Unlock for media buffer", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	return TCIResult<IMFSample>(sample);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMediaFoundationServices::resizeSample(IMFSample* sample, UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove existing memory buffer
	HRESULT	result = sample->RemoveAllBuffers();

	// Create memory buffer
	IMFMediaBuffer*	tempMediaBuffer;
	result = MFCreateMemoryBuffer(size, &tempMediaBuffer);
	ReturnErrorIfFailed(result, "MFCreateMemoryBuffer in CH264VideoCodecInternals::noteFormatChanged");
	OCI<IMFMediaBuffer>	mediaBuffer(tempMediaBuffer);

	// Add buffer to sample
	result = sample->AddBuffer(*mediaBuffer);
	ReturnErrorIfFailed(result, "AddBuffer in CH264VideoCodecInternals::noteFormatChanged");

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMediaFoundationServices::load(IMFMediaBuffer* mediaBuffer, CPacketMediaReader& packetMediaReader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Lock media buffer
	BYTE*	bytePtr;
	HRESULT	result;
	result = mediaBuffer->Lock(&bytePtr, NULL, NULL);
	ReturnErrorIfFailed(result, "Lock");

	// Read next media packet
	TVResult<UInt32>	byteCount = packetMediaReader.readNextMediaPacket(bytePtr);
	if (byteCount.hasError()) {
		// Unlock
		mediaBuffer->Unlock();

		return OI<SError>(byteCount.getError());
	}

	// Update current length
	result = mediaBuffer->SetCurrentLength((DWORD) byteCount.getValue());
	if (FAILED(result)) {
		// Unlock
		mediaBuffer->Unlock();

		ReturnError(result, "SetCurrentLength");
	}

	// Unlock media buffer
	result = mediaBuffer->Unlock();
	ReturnErrorIfFailed(result, "Unlock");

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMediaFoundationServices::processOutput(IMFTransform* transform, IMFSample* outputSample,
		CMediaFoundationServices::ProcessOutputInfo& processOutputInfo)
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
			return OI<SError>();
		else if (result == MF_E_TRANSFORM_STREAM_CHANGE) {
			// Input stream format change
			if (outputDataBuffer.dwStatus == MFT_OUTPUT_DATA_BUFFER_FORMAT_CHANGE) {
				// Output data buffer format change
				IMFMediaType*	mediaType = NULL;
				result = transform->GetOutputAvailableType(0, 0, &mediaType);
				ReturnErrorIfFailed(result, "GetOutputAvailableType for format change");

				// Call proc
				OI<SError>	error = processOutputInfo.noteFormatChanged(mediaType);
				ReturnErrorIfError(error);

				// Set output type
				result = transform->SetOutputType(0, mediaType, 0);
				ReturnErrorIfFailed(result, "SetOutputType for format change");
			} else {
				// Unexpected setup
				ReturnError(E_NOTIMPL, "Input stream format changed, but no updated output format given");
			}
		} else if (result == MF_E_TRANSFORM_NEED_MORE_INPUT) {
			// Get input sample
			TCIResult<IMFSample>	inputSample = processOutputInfo.getInputSample();
			ReturnErrorIfResultError(inputSample);

			// Process input
			result = transform->ProcessInput(0, *inputSample.getInstance(), 0);
			ReturnErrorIfFailed(result, "ProcessInput");
		} else
			// Error
			ReturnError(result, "ProcessOutput");
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<OV<UInt32> > CMediaFoundationServices::completeWrite(IMFSample* sample, UInt32 sampleByteOffset,
		UInt32 maxByteCount, CAudioFrames& audioFrames, const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Copy
	IMFMediaBuffer*	mediaBuffer;
	HRESULT	result = sample->GetBufferByIndex(0, &mediaBuffer);
	ReturnValueIfFailed(result, "GetBufferByIndex for outputSample", TVResult<OV<UInt32> >(SErrorFromHRESULT(result)));

	BYTE*	sampleBytePtr;
	DWORD	sampleLength;
	result = mediaBuffer->Lock(&sampleBytePtr, NULL, &sampleLength);
	ReturnValueIfFailed(result, "Lock for outputSample", TVResult<OV<UInt32> >(SErrorFromHRESULT(result)));

	UInt32	byteCount = std::min<UInt32>(sampleLength - sampleByteOffset, maxByteCount);

	::memcpy(audioFrames.getBuffersAsWrite()[0], sampleBytePtr + sampleByteOffset, byteCount);
	audioFrames.completeWrite(byteCount / audioProcessingFormat.getBytesPerFrame());

	// Check how much of the media buffer was used
	if (byteCount < (sampleLength - sampleByteOffset)) {
		// Still have more to use
		result = mediaBuffer->Unlock();
		ReturnValueIfFailed(result, "Unlock for outputSample", TVResult<OV<UInt32> >(SErrorFromHRESULT(result)));

		return TVResult<OV<UInt32> >(OV<UInt32>(sampleByteOffset + byteCount));
	} else {
		// Used entire buffer
		result = mediaBuffer->SetCurrentLength(0);
		ReturnValueIfFailed(result, "SetCurrentLength", TVResult<OV<UInt32> >(SErrorFromHRESULT(result)));

		result = mediaBuffer->Unlock();
		ReturnValueIfFailed(result, "Unlock for outputSample", TVResult<OV<UInt32> >(SErrorFromHRESULT(result)));

		return TVResult<OV<UInt32> >(OV<UInt32>());
	}
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMediaFoundationServices::flush(IMFTransform* transform)
//----------------------------------------------------------------------------------------------------------------------
{
	HRESULT	result = transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
	ReturnErrorIfFailed(result, "ProcessMessage MFT_MESSAGE_COMMAND_FLUSH");

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

// TODO: Cleanup these procs

//----------------------------------------------------------------------------------------------------------------------
HRESULT LogMediaType(IMFMediaType* mediaType)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Get attribute count
    UINT32	attributeCount;
    result = mediaType->GetCount(&attributeCount);
	if (FAILED(result)) return result;

	// Check attribute count
	if (attributeCount > 0)
		// Log attributes
		for (UINT32 i = 0; i < attributeCount; i++) {
			// Log attribute
			result = LogAttributeValueByIndex(mediaType, i);
			if (FAILED(result)) return result;
		}
	else
		CLogServices::logMessage(OSSTR("Empty media type."));

	return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------
HRESULT LogAttributeValueByIndex(IMFAttributes* pAttr, UINT32 index)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup

    WCHAR *pGuidName = NULL;
    WCHAR *pGuidValName = NULL;

    GUID guid = { 0 };

    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = pAttr->GetItemByIndex(index, &guid, &var);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetGUIDName(guid, &pGuidName);
    if (FAILED(hr))
    {
        goto done;
    }

    DBGMSG(L"\t%s\t", pGuidName);

    hr = SpecialCaseAttributeValue(guid, var);
    if (FAILED(hr))
    {
        goto done;
    }
    if (hr == S_FALSE)
    {
        switch (var.vt)
        {
        case VT_UI4:
            DBGMSG(L"%d", var.ulVal);
            break;

        case VT_UI8:
            DBGMSG(L"%I64d", var.uhVal);
            break;

        case VT_R8:
            DBGMSG(L"%f", var.dblVal);
            break;

        case VT_CLSID:
            hr = GetGUIDName(*var.puuid, &pGuidValName);
            if (SUCCEEDED(hr))
            {
                DBGMSG(pGuidValName);
            }
            break;

        case VT_LPWSTR:
            DBGMSG(var.pwszVal);
            break;

        case VT_VECTOR | VT_UI1:
            DBGMSG(L"<<byte array>>");
            break;

        case VT_UNKNOWN:
            DBGMSG(L"IUnknown");
            break;

        default:
            DBGMSG(L"Unexpected attribute type (vt = %d)", var.vt);
            break;
        }
    }

done:
    DBGMSG(L"\n");
    CoTaskMemFree(pGuidName);
    CoTaskMemFree(pGuidValName);
    PropVariantClear(&var);
    return hr;
}

//----------------------------------------------------------------------------------------------------------------------
HRESULT GetGUIDName(const GUID& guid, WCHAR **ppwsz)
//----------------------------------------------------------------------------------------------------------------------
{
    HRESULT hr = S_OK;
    WCHAR *pName = NULL;

    LPCWSTR pcwsz = GetGUIDNameConst(guid);
    if (pcwsz)
    {
        size_t cchLength = 0;
    
        hr = StringCchLength(pcwsz, STRSAFE_MAX_CCH, &cchLength);
        if (FAILED(hr))
        {
            goto done;
        }
        
        pName = (WCHAR*)CoTaskMemAlloc((cchLength + 1) * sizeof(WCHAR));

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
        hr = StringFromCLSID(guid, &pName);
    }

done:
    if (FAILED(hr))
    {
        *ppwsz = NULL;
        CoTaskMemFree(pName);
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
    Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);
    DBGMSG(L"%d x %d", uHigh, uLow);
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

    DBGMSG(L"(%f,%f) (%d,%d)", OffsetToFloat(pArea->OffsetX), OffsetToFloat(pArea->OffsetY), 
        pArea->Area.cx, pArea->Area.cy);
    return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------
HRESULT SpecialCaseAttributeValue(GUID guid, const PROPVARIANT& var)
//----------------------------------------------------------------------------------------------------------------------
{
    if ((guid == MF_MT_FRAME_RATE) || (guid == MF_MT_FRAME_RATE_RANGE_MAX) ||
        (guid == MF_MT_FRAME_RATE_RANGE_MIN) || (guid == MF_MT_FRAME_SIZE) ||
        (guid == MF_MT_PIXEL_ASPECT_RATIO))
    {
        // Attributes that contain two packed 32-bit values.
        LogUINT32AsUINT64(var);
    }
    else if ((guid == MF_MT_GEOMETRIC_APERTURE) || 
             (guid == MF_MT_MINIMUM_DISPLAY_APERTURE) || 
             (guid == MF_MT_PAN_SCAN_APERTURE))
    {
        // Attributes that an MFVideoArea structure.
        return LogVideoArea(var);
    }
    else
    {
        return S_FALSE;
    }
    return S_OK;
}

//----------------------------------------------------------------------------------------------------------------------
void DBGMSG(PCWSTR format, ...)
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
LPCWSTR GetGUIDNameConst(const GUID& guid)
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
