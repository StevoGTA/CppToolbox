//----------------------------------------------------------------------------------------------------------------------
//	STimecode.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode

struct STimecode {
	// Base
	public:
		enum Base {
			kBase24FPSNonDropFrame	= 24,	// film, ATSC, 2k, 4k, 6k
//			kBase25FPSNonDropFrame	= 25,	// PAL (used in Europe, Uruguay, Argentina, Australia), SECAM, DVB, ATSC
//			kBase29_97FPSDropFrame	= 2997,	// 30 ÷ 1.001 fps - NTSC American System (US, Canada, Mexico, Colombia, etc.), ATSC, PAL-M (Brazil)
//			kBase30FPSNonDropFrame	= 30,	// ATSC
		};

	// Methods
	public:
					// Lifecycle methods
					STimecode(SInt32 frameIndex, Base base) : mFrameIndex(frameIndex), mBase(base) {}
					STimecode(SInt32 hours, SInt32 minutes, SInt32 seconds, SInt32 frames, Base base);
					STimecode(const CString& string, Base base);
					STimecode(const STimecode& other) : mFrameIndex(other.mFrameIndex), mBase(other.mBase) {}

					// Instance methods
		UInt32		getFrameIndex() const
						{ return mFrameIndex; }
		Base		getBase() const
						{ return mBase; }
		CString		getDisplayString() const;

		STimecode	addingFrames(SInt32 frameCount) const
						{ return STimecode(mFrameIndex + frameCount, mBase); }

		bool		operator==(const STimecode& other) const
						{ return (mFrameIndex == other.mFrameIndex) && (mBase == other.mBase); }
		bool		operator!=(const STimecode& other) const
						{ return (mFrameIndex != other.mFrameIndex) || (mBase != other.mBase); }

	// Properties
	private:
		SInt32	mFrameIndex;
		Base	mBase;
};
