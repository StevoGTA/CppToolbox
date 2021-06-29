//----------------------------------------------------------------------------------------------------------------------
//	SVideoFormats.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"

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

	// Properties
	private:
		OSType		mCodecID;
		S2DSizeU16	mFrameSize;
		Float32		mFramerate;
};
