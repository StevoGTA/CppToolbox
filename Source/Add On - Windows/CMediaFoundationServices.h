//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationServices.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CImage.h"
#include "CLogServices.h"
#include "CMediaPacketSource.h"
#include "CVideoFrame.h"
#include "SError-Windows.h"
#include "TResult-Windows.h"

#include <functional>

#undef Delete
#include <mftransform.h>
#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationServices

class CMediaFoundationServices {
	// CreateAudioMediaTypeOptions
	public:
		enum CreateAudioMediaTypeOptions {
			kCreateAudioMediaTypeOptionsNone					= 0,
			kCreateAudioMediaTypeOptionsPreferWAVEFORMATEX		= 1 << 0,
		};

	// ProcessOutputInfo
	public:
		struct ProcessOutputInfo {
			// Procs
			typedef TCIResult<IMFSample>	(*ReadInputSampleProc)(void* userData);
			typedef	OV<SError>				(*FillInputBufferProc)(IMFSample* sample, IMFMediaBuffer* mediaBuffer,
													void* userData);
			typedef	OV<SError>				(*NoteFormatChangedProc)(IMFMediaType* mediaType, void* userData);

									// Methods
									ProcessOutputInfo(ReadInputSampleProc readInputSampleProc,
											NoteFormatChangedProc noteFormatChangedProc, void* userData) :
										mReadInputSampleProc(readInputSampleProc),
												mFillInputBufferProc(NULL),
												mNoteFormatChangedProc(noteFormatChangedProc),
												mUserData(userData)
										{}
									ProcessOutputInfo(FillInputBufferProc fillInputBufferProc,
											const OCI<IMFSample>& inputSample, void* userData) :
										mReadInputSampleProc(NULL),
												mFillInputBufferProc(fillInputBufferProc), mInputSample(inputSample),
												mNoteFormatChangedProc(NULL),
												mUserData(userData)
										{}

			TCIResult<IMFSample>	getInputSample() const
										{
											// Check situation
											if (mReadInputSampleProc != NULL)
												// Proc will do all
												return mReadInputSampleProc(mUserData);
											else {
												// Get media buffer from provided input sample
												IMFMediaBuffer*	mediaBuffer;
												HRESULT			result =
																		mInputSample->GetBufferByIndex(0, &mediaBuffer);
												ReturnValueIfFailed(result, OSSTR("GetBufferByIndex for inputSample"),
														TCIResult<IMFSample>(SErrorFromHRESULT(result)));

												// Call proc
												OV<SError>	error =
																	mFillInputBufferProc(*mInputSample, mediaBuffer,
																			mUserData);
												if (error.hasValue()) {
													// Cleanup
													mediaBuffer->Release();

													return TCIResult<IMFSample>(*error);
												}

												return TCIResult<IMFSample>(mInputSample);
											}
										}
			OV<SError>				noteFormatChanged(IMFMediaType* mediaType) const
										{ return (mNoteFormatChangedProc != NULL) ?
												mNoteFormatChangedProc(mediaType, mUserData) : OV<SError>(); }

			// Properties
			private:
				ReadInputSampleProc		mReadInputSampleProc;

				FillInputBufferProc		mFillInputBufferProc;
				OCI<IMFSample>			mInputSample;

				NoteFormatChangedProc	mNoteFormatChangedProc;

				void*					mUserData;
		};

	// Methods
	public:
														// Class methods
		static	TCIResult<IMFTransform>					createTransform(MFT_REGISTER_TYPE_INFO registerTypeInfo,
																GUID guidCategory, UINT32 flags);
		static	TCIResult<IMFTransform>					createAudioTransform(GUID guidCategory, GUID guid);

		static	void									iterateOutputMediaTypes(IMFTransform* transform,
																std::function<void(IMFMediaType* mediaType,
																		Float32 sampleRate,
																		const SAudio::ChannelMap& audioChannelMap,
																		UInt32 bitrate)> mediaTypeProc);

		static	OV<SError>								setInputType(IMFTransform* transform, const GUID& guid,
																UInt8 bits, Float32 sampleRate,
																const SAudio::ChannelMap& audioChannelMap,
																const OV<UInt32>& bytesPerFrame = OV<UInt32>(),
																const OV<UInt32>& bytesPerSecond = OV<UInt32>(),
																const OV<CData>& userData = OV<CData>(),
																CreateAudioMediaTypeOptions options =
																		kCreateAudioMediaTypeOptionsNone);
		static	OV<SError>								setInputType(IMFTransform* transform,
																const SAudio::ProcessingFormat& audioProcessingFormat);
		static	OV<SError>								setOutputType(IMFTransform* transform, const GUID& guid,
																const OV<UInt8>& bits, Float32 sampleRate,
																const SAudio::ChannelMap& audioChannelMap,
																const OV<UInt32>& bitrate = OV<UInt32>(),
																const OV<UInt32>& bytesPerFrame = OV<UInt32>(),
																const OV<UInt32>& bytesPerSecond = OV<UInt32>(),
																std::function<bool(IMFMediaType* mediaType)>
																				isApplicableProc =
																		[](IMFMediaType* mediaType){ return true; });
		static	OV<SError>								setOutputType(IMFTransform* transform,
																const SAudio::ProcessingFormat& audioProcessingFormat);

		static	TCIResult<IMFSample>					createSample(UInt32 size);
		static	TCIResult<IMFSample>					createSample(const CData& data);
		static	OV<SError>								resizeSample(IMFSample* sample, UInt32 size);
		static	TVResult<CAudioProcessor::SourceInfo>	load(IMFMediaBuffer* mediaBuffer,
															CAudioProcessor& audioProcessor,
															const SAudio::ProcessingFormat& audioProcessingFormat);
		static	OV<SError>								load(IMFMediaBuffer* mediaBuffer,
																CMediaPacketSource& mediaPacketSource);
		static	TVResult<CImage>						imageForVideoSample(const CVideoFrame& videoFrame);

		static	OV<SError>								processOutput(IMFTransform* transform, IMFSample* outputSample,
																const ProcessOutputInfo& processOutputInfo);

		static	TVResult<UInt32>						completeWrite(IMFSample* sample, UInt32 frameOffset,
																CAudioFrames& audioFrames,
																const SAudio::ProcessingFormat& audioProcessingFormat);

		static	OV<SError>								flush(IMFTransform* transform);

#if defined(DEBUG)
		static	OV<SError>								log(IMFMediaType* mediaType);
#endif
};
