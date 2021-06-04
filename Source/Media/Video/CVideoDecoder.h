//----------------------------------------------------------------------------------------------------------------------
//	CVideoDecoder.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoCodec.h"
#include "CVideoProcessor.h"
#include "SVideoFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoDecoder

class CVideoDecoderInternals;
class CVideoDecoder : public CVideoProcessor {
	// DecodeInfo
	public:
		struct DecodeInfo {
			// Procs - called from the decoder thread.
			typedef	void	(*FrameReadyProc)(CVideoDecoder& videoDecoder, void* userData);
			typedef	void	(*ErrorProc)(const CVideoDecoder& videoDecoder, const SError& error, void* userData);

					// Lifecycle methods
					DecodeInfo(FrameReadyProc frameReadyProc, ErrorProc errorProc, void* userData) :
							mFrameReadyProc(frameReadyProc), mErrorProc(errorProc), mUserData(userData)
							{}
					DecodeInfo(const DecodeInfo& other) :
							mFrameReadyProc(other.mFrameReadyProc), mErrorProc(other.mErrorProc),
									mUserData(other.mUserData)
							{}

					// Instance methods
			void	frameReady(CVideoDecoder& videoDecoder) const
						{ mFrameReadyProc(videoDecoder, mUserData); }
			void	error(const CVideoDecoder& videoDecoder, const SError& error) const
						{ mErrorProc(videoDecoder, error, mUserData); }

			// Properties
			private:
				FrameReadyProc	mFrameReadyProc;
				ErrorProc		mErrorProc;
				void*			mUserData;
		};

	// RenderInfo
	public:
		struct RenderInfo {
			// Procs
			typedef	void	(*CurrentVideoFrameUpdatedProc)(UInt32 trackIndex, const CVideoFrame& videoFrame,
									void* userData);

					// Lifecycle methods
					RenderInfo(CurrentVideoFrameUpdatedProc currentVideoFrameUpdatedProc, void* userData) :
						mCurrentVideoFrameUpdatedProc(currentVideoFrameUpdatedProc), mUserData(userData)
						{}
					RenderInfo(const RenderInfo& other) :
						mCurrentVideoFrameUpdatedProc(other.mCurrentVideoFrameUpdatedProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	currentVideoFrameUpdated(UInt32 trackIndex, const CVideoFrame& videoFrame)
						{ mCurrentVideoFrameUpdatedProc(trackIndex, videoFrame, mUserData); }

			// Properties
			private:
				CurrentVideoFrameUpdatedProc	mCurrentVideoFrameUpdatedProc;
				void*							mUserData;
		};

	// Methods
	public:
					// Lifecycle methods
					CVideoDecoder(const SVideoStorageFormat& videoStorageFormat,
							const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CDataSource>& dataSource,
							const CString& identifier, UInt32 trackIndex, const DecodeInfo& decodeInfo,
							CVideoCodec::DecodeFrameInfo::Compatibility compatibility, const RenderInfo& renderInfo);
					~CVideoDecoder();

					// CVideoProcessor methods
		OI<SError>	reset();

					// Instance methods
		void		handleFrameReady();
		void		notePositionUpdated(UniversalTimeInterval position);

	// Properties
	private:
		CVideoDecoderInternals*	mInternals;
};
