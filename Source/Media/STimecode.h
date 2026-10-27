//----------------------------------------------------------------------------------------------------------------------
//	STimecode.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TimeAndDate.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode

struct STimecode {
	// FrameRate
	public:
		struct FrameRate {
			// Methods
			public:
													// Lifecycle methods
													FrameRate(UInt32 numerator, UInt32 denominator, bool isDropFrame);
													FrameRate(Float64 frameRate, bool isDropFrame = false) :
														mFrameRate(frameRate), mIsDropFrame(isDropFrame)
														{}
													FrameRate(const CDictionary& storageInfo);

													// Instance methods
								Float64				getFrameRate() const
														{ return mFrameRate; }
								UInt32				getBase() const
														{ return (UInt32) ::round(mFrameRate); }
								bool				isDropFrame() const
														{ return mIsDropFrame; }

								CString				getDisplayString() const;

								CDictionary			getStorageInfo() const;

								bool				operator==(const FrameRate& other) const
														{ return (mFrameRate == other.mFrameRate) &&
																(mIsDropFrame == other.mIsDropFrame); }
								bool				operator!=(const FrameRate& other) const
														{ return (mFrameRate != other.mFrameRate) ||
																(mIsDropFrame != other.mIsDropFrame); }

													// Class methods
				static	const	TArray<FrameRate>&	getStandard();

			// Properties
			public:
				static	const	FrameRate	_23_976;
				static	const	FrameRate	_24;
				static	const	FrameRate	_25;
				static	const	FrameRate	_29_97;
				static	const	FrameRate	_29_97DropFrame;
				static	const	FrameRate	_30;
				static	const	FrameRate	_30DropFrame;
				static	const	FrameRate	_47_952;
				static	const	FrameRate	_48;
				static	const	FrameRate	_50;
				static	const	FrameRate	_59_94;
				static	const	FrameRate	_59_94DropFrame;
				static	const	FrameRate	_60;

			private:
								Float64		mFrameRate;
								bool		mIsDropFrame;
		};

	// HMSF
	public:
		struct HMSF : public TV5<SInt32, SInt32, SInt32, SInt32, bool> {
			// Methods
			public:
						// Lifecycle methods
						HMSF(SInt32 hours, SInt32 minutes, SInt32 seconds, SInt32 frames, bool isDropFrame) :
							TV5(hours, minutes, seconds, frames, isDropFrame)
							{}

						// Instance methods
				SInt32	getHours() const
							{ return getA(); }
				SInt32	getMinutes() const
							{ return getB(); }
				SInt32	getSeconds() const
							{ return getC(); }
				SInt32	getFrames() const
							{ return getD(); }
				bool	isDropFrame() const
							{ return getE(); }

				CString	getDisplayString() const;

				bool	operator==(const HMSF& other) const
							{ return (getHours() == other.getHours()) && (getMinutes() == other.getMinutes()) &&
									(getSeconds() == other.getSeconds()) && (getFrames() == other.getFrames()) &&
									(isDropFrame() == other.isDropFrame()); }
				bool	operator!=(const HMSF& other) const
							{ return !(*this == other); }
		};

	// Methods
	public:
										// Class methods
		static	HMSF					getHMSF(UniversalTimeInterval timeInterval, const FrameRate& frameRate);
		static	HMSF					getEquivalentHMSF(const HMSF& hmsf, const FrameRate& frameRate);
		static	UniversalTimeInterval	getTimeInterval(const HMSF& hmsf, const FrameRate& frameRate);
		static	CString					getDisplayString(UniversalTimeInterval timeInterval,
												const FrameRate& frameRate);
};
