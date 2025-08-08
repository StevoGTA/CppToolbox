//----------------------------------------------------------------------------------------------------------------------
//	STimecode.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode

struct STimecode {
	// FrameRate
	public:
		struct FrameRate {
			// Kind
			public:
				enum Kind {
					kKindNonDropFrame	= 0,
					kKindDropFrame2997	= 2997,
					kKindDropFrame5994	= 5994,
				};

			// Methods
			public:
													// Lifecycle methods
													FrameRate(const FrameRate& other) :
														mKind(other.mKind), mNonDropFrameBase(other.mNonDropFrameBase)
														{}

													// Instance methods
								Kind				getKind() const
														{ return mKind; }
								bool				isDropFrame() const
														{ return mKind != kKindNonDropFrame; }
								UInt32				getBase() const
														{
															// Check Kind
															switch (mKind) {
																case kKindNonDropFrame:		return *mNonDropFrameBase;
																case kKindDropFrame2997:	return 30;
																case kKindDropFrame5994:	return 60;
#if defined(TARGET_OS_WINDOWS)
																default:					return 0;
#endif
															}
														}
								Float32				getFrameRate() const
														{
															// Check kind
															switch (mKind) {
																case kKindNonDropFrame:		return (Float32) *mNonDropFrameBase;
																case kKindDropFrame2997:	return 30000.0f / 1001.0f;
																case kKindDropFrame5994:	return 60000.0f / 1001.0f;
#if defined(TARGET_OS_WINDOWS)
																default:					return 0.0;
#endif
															}
														}

								CDictionary			getInfo() const;

								CString				getDisplayString() const;

								bool				operator==(const FrameRate& other) const
														{ return (mKind == other.mKind) &&
																(mNonDropFrameBase == other.mNonDropFrameBase); }
								bool				operator!=(const FrameRate& other) const
														{ return (mKind != other.mKind) ||
																(mNonDropFrameBase != other.mNonDropFrameBase); }

													// Class methods
				static			FrameRate			forNonDropFrame(UInt32 base)
														{ return FrameRate(kKindNonDropFrame, base); }
				static			FrameRate			forDropFrame2997()
														{ return FrameRate(kKindDropFrame2997); }
				static			FrameRate			forDropFrame5994()
														{ return FrameRate(kKindDropFrame5994); }

				static			OV<FrameRate>		fromInfo(const CDictionary& info);

				static	const	TArray<FrameRate>&	getStandard();

			private:
													// Lifecycle methods
													FrameRate(Kind kind, UInt32 nonDropFrameBase) :
														mKind(kind), mNonDropFrameBase(nonDropFrameBase)
														{}
													FrameRate(Kind kind) : mKind(kind) {}

			// Properties
			public:
				static	FrameRate	mDefault;

			private:
						Kind		mKind;
						OV<UInt32>	mNonDropFrameBase;
		};

	// HMSF
	public:
		struct HMSF {
			// Methods
			public:
						// Lifecycle methods
						HMSF(SInt32 hours, SInt32 minutes, SInt32 seconds, SInt32 frames) :
							mHours(hours), mMinutes(minutes), mSeconds(seconds), mFrames(frames)
							{}
						HMSF(const HMSF& other) :
							mHours(other.mHours), mMinutes(other.mMinutes), mSeconds(other.mSeconds),
									mFrames(other.mFrames)
							{}

						// Instance methods
				SInt32	getHours() const
							{ return mHours; }
				SInt32	getMinutes() const
							{ return mMinutes; }
				SInt32	getSeconds() const
							{ return mSeconds; }
				SInt32	getFrames() const
							{ return mFrames; }

			// Properties
			private:
				SInt32	mHours;
				SInt32	mMinutes;
				SInt32	mSeconds;
				SInt32	mFrames;
		};

	// Methods
	public:
										// Lifecycle methods
										STimecode(SInt32 frameIndex,
												const FrameRate& frameRate = FrameRate::forNonDropFrame(24)) :
											mFrameIndex(frameIndex), mFrameRate(frameRate)
											{}
										STimecode(SInt32 hours, SInt32 minutes, SInt32 seconds, SInt32 frames,
												const FrameRate& frameRate = FrameRate::forNonDropFrame(24));
										STimecode(SInt32 hours, SInt32 minutes, Float32 seconds,
												const FrameRate& frameRate = FrameRate::forNonDropFrame(24));
										STimecode(const STimecode& other) :
											mFrameIndex(other.mFrameIndex), mFrameRate(other.mFrameRate)
											{}

										// Instance methods
						UInt32			getFrameIndex() const
											{ return mFrameIndex; }
						HMSF			getHMSF() const;
				const	FrameRate&		getFrameRate() const
											{ return mFrameRate; }

						CString			getDisplayString() const;

						STimecode		addingFrames(SInt32 frameCount) const
											{ return STimecode(mFrameIndex + frameCount, mFrameRate); }
						STimecode		addingSeconds(Float64 seconds) const
											{ return STimecode(
													mFrameIndex + (SInt32) (seconds * mFrameRate.getFrameRate()),
													mFrameRate); }

						Float64			getSeconds() const
											{ return (Float64) mFrameIndex / (Float64) mFrameRate.getFrameRate(); }

						CDictionary		getInfo() const;

						bool			operator==(const STimecode& other) const
											{ return (mFrameIndex == other.mFrameIndex) &&
													(mFrameRate == other.mFrameRate); }
						bool			operator!=(const STimecode& other) const
											{ return (mFrameIndex != other.mFrameIndex) ||
													(mFrameRate != other.mFrameRate); }
						STimecode&		operator=(const STimecode& other)
											{ mFrameIndex = other.mFrameIndex; mFrameRate = other.mFrameRate;
													return *this; }

										// Class methods
		static			OV<STimecode>	fromInfo(const CDictionary& info);
		static			OV<STimecode>	fromString(const CString& string,
												const FrameRate& frameRate = FrameRate::forNonDropFrame(24));

	// Properties
	private:
		SInt32		mFrameIndex;
		FrameRate	mFrameRate;
};
