//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationServices.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "CLogServices.h"
#include "SMediaPacket.h"
#include "SAudioFormats.h"
#include "SError-Windows.h"
#include "TResult-Windows.h"

#undef Delete

#include <mftransform.h>

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationServices

class CMediaFoundationServices {
	// ProcessOutputInfo
	public:
		struct ProcessOutputInfo {
			// Procs
			typedef TCIResult<IMFSample>	(*ReadInputSampleProc)(void* userData);
			typedef	OI<SError>				(*FillInputBufferProc)(IMFSample* sample, IMFMediaBuffer* mediaBuffer,
													void* userData);
			typedef	OI<SError>				(*NoteFormatChangedProc)(IMFMediaType* mediaType, void* userData);

									// Methods
									ProcessOutputInfo(ReadInputSampleProc readInputSampleProc,
											NoteFormatChangedProc noteFormatChangedProc, void* userData) :
										mReadInputSampleProc(readInputSampleProc),
												mFillInputBufferProc(NULL), mInputSample(NULL),
												mNoteFormatChangedProc(noteFormatChangedProc),
												mUserData(userData)
										{}
									ProcessOutputInfo(FillInputBufferProc fillInputBufferProc,
											OCI<IMFSample> inputSample, void* userData) :
										mReadInputSampleProc(NULL),
												mFillInputBufferProc(fillInputBufferProc), mInputSample(inputSample),
												mNoteFormatChangedProc(NULL),
												mUserData(userData)
										{}

			TCIResult<IMFSample>	getInputSample()
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
												ReturnValueIfFailed(result, "GetBufferByIndex for inputSample",
														TCIResult<IMFSample>(SErrorFromHRESULT(result)));

												// Call proc
												OI<SError>	error =
																	mFillInputBufferProc(*mInputSample, mediaBuffer,
																			mUserData);
												ReturnValueIfError(error, TCIResult<IMFSample>(*error));

												return TCIResult<IMFSample>(mInputSample);
											}
										}
			OI<SError>				noteFormatChanged(IMFMediaType* mediaType) const
										{ return mNoteFormatChangedProc(mediaType, mUserData); }
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
		static	TCIResult<IMFTransform>	createTransformForAudioDecode(const GUID& guid,
												const SAudioProcessingFormat& audioProcessingFormat,
												const OI<CData>& userData = OI<CData>());
		static	TCIResult<IMFTransform>	createTransformForVideoDecode(const GUID& guid);

		static	TCIResult<IMFSample>	createSample(UInt32 size);
		static	TCIResult<IMFSample>	createSample(const CData& data);
		static	OI<SError>				resizeSample(IMFSample* sample, UInt32 size);
		static	OI<SError>				load(IMFMediaBuffer* mediaBuffer, CPacketMediaReader& packetMediaReader);

		static	OI<SError>				processOutput(IMFTransform* transform, IMFSample* outputSample,
												ProcessOutputInfo& processOutputInfo);

		static	OI<SError>				completeWrite(IMFSample* sample, CAudioFrames& audioFrames,
												const SAudioProcessingFormat& audioProcessingFormat);

		static	OI<SError>				flush(IMFTransform* transform);
};
