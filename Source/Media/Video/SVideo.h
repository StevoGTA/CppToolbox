//----------------------------------------------------------------------------------------------------------------------
//	SVideo.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVideo

struct SVideo {
	// FrameRate
	struct FrameRate {
		// Methods
		public:
							// Class methods
			static	CString	getDisplayString(Float64 frameRate);
	};

	// Format
	struct Format {
		// Methods
		public:
								// Lifecycle methods
								Format(OSType codecID, const S2DSizeU16& frameSize, UInt32 frameCount,
										UniversalTimeInterval duration) :
									mCodecID(codecID), mFrameSize(frameSize),
											mFrameRate((Float64) frameCount / duration)
									{}
								Format(OSType codecID, const S2DSizeU16& frameSize, UInt32 frameRateNumerator,
										UInt32 frameRateDenominator) :
									mCodecID(codecID), mFrameSize(frameSize),
											mFrameRate((Float64) frameRateNumerator / (Float64) frameRateDenominator)
									{}
								Format(OSType codecID, const S2DSizeU16& frameSize, Float64 frameRate) :
									mCodecID(codecID), mFrameSize(frameSize), mFrameRate(frameRate)
									{}

								// Instance methods
					OSType		getCodecID() const
									{ return mCodecID; }
			const	S2DSizeU16&	getFrameSize() const
									{ return mFrameSize; }
					Float64		getFrameRate() const
									{ return mFrameRate; }

					CString		getDescription() const;

		// Properties
		private:
			OSType		mCodecID;
			S2DSizeU16	mFrameSize;
			Float64		mFrameRate;
	};
};
