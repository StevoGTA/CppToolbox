//----------------------------------------------------------------------------------------------------------------------
//	STimecode.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode

struct STimecode {
	// Mode
	public:
		enum Mode {
			kMode24FPSNonDropFrame	= 24,	// film, ATSC, 2k, 4k, 6k
//			kMode25FPSNonDropFrame	= 25,	// PAL (used in Europe, Uruguay, Argentina, Australia), SECAM, DVB, ATSC
//			kMode29_97FPSDropFrame	= 2997,	// 30 ÷ 1.001 fps - NTSC American System (US, Canada, Mexico, Colombia, etc.), ATSC, PAL-M (Brazil)
//			kMode30FPSNonDropFrame	= 30,	// ATSC
		};

	// Methods
	public:
					// Lifecycle methods
					STimecode(SInt32 frameIndex, Mode mode) : mFrameIndex(frameIndex), mMode(mode) {}
					STimecode(SInt32 hours, SInt32 minutes, SInt32 seconds, SInt32 frames, Mode mode);
					STimecode(const CString& string, Mode mode);
					STimecode(const STimecode& other) : mFrameIndex(other.mFrameIndex), mMode(other.mMode) {}

					// Instance methods
		UInt32		getFrameIndex() const
						{ return mFrameIndex; }
		Mode		getMode() const
						{ return mMode; }
		CString		getDisplayString() const;

		STimecode	addingFrames(SInt32 frameCount) const
						{ return STimecode(mFrameIndex + frameCount, mMode); }

		bool		operator==(const STimecode& other) const
						{ return (mFrameIndex == other.mFrameIndex) && (mMode == other.mMode); }
		bool		operator!=(const STimecode& other) const
						{ return (mFrameIndex != other.mFrameIndex) || (mMode != other.mMode); }

	// Properties
	private:
		SInt32	mFrameIndex;
		Mode	mMode;
};
