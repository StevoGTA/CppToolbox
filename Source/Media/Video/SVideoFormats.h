//----------------------------------------------------------------------------------------------------------------------
//	SVideoFormats.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CVideoFrame.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVideoStorageFormat

struct SVideoStorageFormat {
						// Lifecycle methods
						SVideoStorageFormat(OSType codecID, const S2DSizeU16& frameSize, Float32 framerate) :
							mCodecID(codecID), mFrameSize(frameSize), mFramerate(framerate)
							{}

						// Instance methods
			OSType		getCodecID() const
							{ return mCodecID; }
	const	S2DSizeU16&	getFrameSize() const
							{ return mFrameSize; }
			Float32		getFramerate() const
							{ return mFramerate; }
			CString		getDescription() const
							{
								// Compose description
								CString	description;

								description +=
										CString(mFrameSize.mWidth) + CString(OSSTR("x")) + CString(mFrameSize.mHeight);
								description +=
										CString(OSSTR(" at ")) + CString(mFramerate, 0, 3) + CString(OSSTR("fps"));

								return description;
							}

	// Properties
	private:
		OSType		mCodecID;
		S2DSizeU16	mFrameSize;
		Float32		mFramerate;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SVideoProcessingFormat

struct SVideoProcessingFormat {

										// Lifecycle methods
										SVideoProcessingFormat(const S2DSizeU16& frameSize, Float32 framerate,
												CVideoFrame::Compatibility compatibility) :
											mFrameSize(frameSize), mFramerate(framerate), mCompatibility(compatibility)
											{}

										// Instance methods
	const	S2DSizeU16&					getFrameSize() const
											{ return mFrameSize; }
			Float32						getFramerate() const
											{ return mFramerate; }
			CVideoFrame::Compatibility	getCompatibility() const
											{ return mCompatibility; }

	// Properties
	private:
		S2DSizeU16					mFrameSize;
		Float32						mFramerate;
		CVideoFrame::Compatibility	mCompatibility;
};
