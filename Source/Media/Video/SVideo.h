//----------------------------------------------------------------------------------------------------------------------
//	SVideo.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVideo

struct SVideo {
	// Format
	struct Format {
							// Lifecycle methods
							Format(OSType codecID, const S2DSizeU16& frameSize, Float32 framerate) :
								mCodecID(codecID), mFrameSize(frameSize), mFramerate(framerate)
								{}
							Format(const Format& other) :
								mCodecID(other.mCodecID), mFrameSize(other.mFrameSize), mFramerate(other.mFramerate)
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
											CString(mFrameSize.mWidth) + CString(OSSTR("x")) +
													CString(mFrameSize.mHeight);
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
};
