//----------------------------------------------------------------------------------------------------------------------
//	CAudioConverter-MediaFoundation.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioProcessor.h"

// See https://github.com/thestk/rtaudio
// See https://sourceforge.net/p/playpcmwin/wiki/HowToUseResamplerMFT/
// See https://github.com/naudio/NAudio/blob/master/Docs/Resampling.md
// See https://www.codeproject.com/articles/501521/how-to-convert-between-most-audio-formats-in-net

//#ifndef INITGUID
//  #define INITGUID
//#endif

#undef Delete

//#include <Windows.h>

#include <mfapi.h>
#include <mferror.h>
//#include <mfplay.h>
#include <mftransform.h>
#include <wmcodecdsp.h>

//#include <audioclient.h>
//#include <avrt.h>
//#include <mmdeviceapi.h>
//#include <functiondiscoverykeys_devpkey.h>

//#ifndef MF_E_TRANSFORM_NEED_MORE_INPUT
//  #define MF_E_TRANSFORM_NEED_MORE_INPUT _HRESULT_TYPEDEF_(0xc00d6d72)
//#endif

//#ifndef MFSTARTUP_NOSOCKET
//  #define MFSTARTUP_NOSOCKET 0x1
//#endif

//#pragma comment( lib, "ksuser" )
//#pragma comment( lib, "mfplat.lib" )
//#pragma comment( lib, "mfuuid.lib" )
//#pragma comment( lib, "wmcodecdspuuid" )

//#define SAFE_RELEASE( objectPtr )\
//if ( objectPtr )\
//{\
//  objectPtr->Release();\
//  objectPtr = NULL;\
//}

#define Delete(x)		{ delete x; x = nil; }

//typedef HANDLE ( __stdcall *TAvSetMmThreadCharacteristicsPtr )( LPCWSTR TaskName, LPDWORD TaskIndex );

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioConverterInternals

class CAudioConverterInternals {
	public:
		CAudioConverterInternals()
			{
int	abc = WINAPI_FAMILY;
			}
		~CAudioConverterInternals()
			{
				// 8. Send stream stop messages to Resampler

				_transform->ProcessMessage( MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0 );
				_transform->ProcessMessage( MFT_MESSAGE_NOTIFY_END_STREAMING, 0 );

				// 9. Cleanup

				MFShutdown();

				SAFE_RELEASE( _transformUnk );
				SAFE_RELEASE( _transform );
				SAFE_RELEASE( _mediaType );
				SAFE_RELEASE( _inputMediaType );
				SAFE_RELEASE( _outputMediaType );

				#ifdef __IWMResamplerProps_FWD_DEFINED__
					SAFE_RELEASE( _resamplerProps );
				#endif
			}

		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;
		OI<SAudioProcessingFormat>	mOutputAudioProcessingFormat;

  unsigned int _bytesPerSample;
  unsigned int _channelCount;
  float _sampleRatio;

  IUnknown* _transformUnk;
  IMFTransform* _transform;
  IMFMediaType* _mediaType;
  IMFMediaType* _inputMediaType;
  IMFMediaType* _outputMediaType;

  #ifdef __IWMResamplerProps_FWD_DEFINED__
    IWMResamplerProps* _resamplerProps;
  #endif
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioConverter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioConverter::CAudioConverter() : CAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioConverterInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioConverter::~CAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioConverter::connectInput(const I<CAudioProcessor>& audioProcessor,
	const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
    // 1. Initialization

    MFStartup( MF_VERSION, MFSTARTUP_NOSOCKET );

    // 2. Create Resampler Transform Object

    CoCreateInstance( CLSID_CResamplerMediaObject, NULL, CLSCTX_INPROC_SERVER,
                      IID_IUnknown, ( void** ) &_transformUnk );

    _transformUnk->QueryInterface( IID_PPV_ARGS( &_transform ) );

    #ifdef __IWMResamplerProps_FWD_DEFINED__
      _transformUnk->QueryInterface( IID_PPV_ARGS( &_resamplerProps ) );
      _resamplerProps->SetHalfFilterLength( 60 ); // best conversion quality
    #endif

    // 3. Specify input / output format

    MFCreateMediaType( &_mediaType );
    _mediaType->SetGUID( MF_MT_MAJOR_TYPE, MFMediaType_Audio );
    _mediaType->SetGUID( MF_MT_SUBTYPE, isFloat ? MFAudioFormat_Float : MFAudioFormat_PCM );
    _mediaType->SetUINT32( MF_MT_AUDIO_NUM_CHANNELS, channelCount );
    _mediaType->SetUINT32( MF_MT_AUDIO_SAMPLES_PER_SECOND, inSampleRate );
    _mediaType->SetUINT32( MF_MT_AUDIO_BLOCK_ALIGNMENT, _bytesPerSample * channelCount );
    _mediaType->SetUINT32( MF_MT_AUDIO_AVG_BYTES_PER_SECOND, _bytesPerSample * channelCount * inSampleRate );
    _mediaType->SetUINT32( MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample );
    _mediaType->SetUINT32( MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE );

    MFCreateMediaType( &_inputMediaType );
    _mediaType->CopyAllItems( _inputMediaType );

    _transform->SetInputType( 0, _inputMediaType, 0 );

    MFCreateMediaType( &_outputMediaType );
    _mediaType->CopyAllItems( _outputMediaType );

    _outputMediaType->SetUINT32( MF_MT_AUDIO_SAMPLES_PER_SECOND, outSampleRate );
    _outputMediaType->SetUINT32( MF_MT_AUDIO_AVG_BYTES_PER_SECOND, _bytesPerSample * channelCount * outSampleRate );

    _transform->SetOutputType( 0, _outputMediaType, 0 );

    // 4. Send stream start messages to Resampler

    _transform->ProcessMessage( MFT_MESSAGE_COMMAND_FLUSH, 0 );
    _transform->ProcessMessage( MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0 );
    _transform->ProcessMessage( MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0 );

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAudioConverter::perform(const SMediaPosition& mediaPosition, CAudioData& audioData)
//----------------------------------------------------------------------------------------------------------------------
{
    unsigned int inputBufferSize = _bytesPerSample * _channelCount * inSampleCount;
    if ( _sampleRatio == 1 )
    {
      // no sample rate conversion required
      memcpy( outBuffer, inBuffer, inputBufferSize );
      outSampleCount = inSampleCount;
      return;
    }

    unsigned int outputBufferSize = 0;
    if ( maxOutSampleCount != -1 )
    {
      outputBufferSize = _bytesPerSample * _channelCount * maxOutSampleCount;
    }
    else
    {
      outputBufferSize = ( unsigned int ) ceilf( inputBufferSize * _sampleRatio ) + ( _bytesPerSample * _channelCount );
    }

    IMFMediaBuffer* rInBuffer;
    IMFSample* rInSample;
    BYTE* rInByteBuffer = NULL;

    // 5. Create Sample object from input data

    MFCreateMemoryBuffer( inputBufferSize, &rInBuffer );

    rInBuffer->Lock( &rInByteBuffer, NULL, NULL );
    memcpy( rInByteBuffer, inBuffer, inputBufferSize );
    rInBuffer->Unlock();
    rInByteBuffer = NULL;

    rInBuffer->SetCurrentLength( inputBufferSize );

    MFCreateSample( &rInSample );
    rInSample->AddBuffer( rInBuffer );

    // 6. Pass input data to Resampler

    _transform->ProcessInput( 0, rInSample, 0 );

    SAFE_RELEASE( rInBuffer );
    SAFE_RELEASE( rInSample );

    // 7. Perform sample rate conversion

    IMFMediaBuffer* rOutBuffer = NULL;
    BYTE* rOutByteBuffer = NULL;

    MFT_OUTPUT_DATA_BUFFER rOutDataBuffer;
    DWORD rStatus;
    DWORD rBytes = outputBufferSize; // maximum bytes accepted per ProcessOutput

    // 7.1 Create Sample object for output data

    memset( &rOutDataBuffer, 0, sizeof rOutDataBuffer );
    MFCreateSample( &( rOutDataBuffer.pSample ) );
    MFCreateMemoryBuffer( rBytes, &rOutBuffer );
    rOutDataBuffer.pSample->AddBuffer( rOutBuffer );
    rOutDataBuffer.dwStreamID = 0;
    rOutDataBuffer.dwStatus = 0;
    rOutDataBuffer.pEvents = NULL;

    // 7.2 Get output data from Resampler

    if ( _transform->ProcessOutput( 0, 1, &rOutDataBuffer, &rStatus ) == MF_E_TRANSFORM_NEED_MORE_INPUT )
    {
      outSampleCount = 0;
      SAFE_RELEASE( rOutBuffer );
      SAFE_RELEASE( rOutDataBuffer.pSample );
      return;
    }

    // 7.3 Write output data to outBuffer

    SAFE_RELEASE( rOutBuffer );
    rOutDataBuffer.pSample->ConvertToContiguousBuffer( &rOutBuffer );
    rOutBuffer->GetCurrentLength( &rBytes );

    rOutBuffer->Lock( &rOutByteBuffer, NULL, NULL );
    memcpy( outBuffer, rOutByteBuffer, rBytes );
    rOutBuffer->Unlock();
    rOutByteBuffer = NULL;

    outSampleCount = rBytes / _bytesPerSample / _channelCount;
    SAFE_RELEASE( rOutBuffer );
    SAFE_RELEASE( rOutDataBuffer.pSample );

	return SAudioReadStatus(0.0);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioConverter::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	return OI<SError>();
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioConverter::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup(*mInternals->mOutputAudioProcessingFormat));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioConverter::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup::mUnspecified);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioConverter::setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mOutputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
}
