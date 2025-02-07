//----------------------------------------------------------------------------------------------------------------------
//	STimecode.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode

struct STimecode {
	// Framerate
	public:
		struct Framerate {
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
										Framerate(const Framerate& other) :
											mKind(other.mKind), mNonDropFrameBase(other.mNonDropFrameBase)
											{}

										// Instance methods
						Kind			getKind() const
											{ return mKind; }
						bool			isDropFrame() const
											{ return mKind != kKindNonDropFrame; }
						UInt32			getBase() const
											{
												// Check Kind
												switch (mKind) {
													case kKindNonDropFrame:		return *mNonDropFrameBase;
													case kKindDropFrame2997:	return 30;
													case kKindDropFrame5994:	return 60;
												}
											}
						Float32			getFramerate() const
											{
												// Check kind
												switch (mKind) {
													case kKindNonDropFrame:		return *mNonDropFrameBase;
													case kKindDropFrame2997:	return 30000.0f / 1001.0f;
													case kKindDropFrame5994:	return 60000.0f / 1001.0f;
												}
											}

						CDictionary		getInfo() const;

						bool			operator==(const Framerate& other) const
											{ return (mKind == other.mKind) &&
													(mNonDropFrameBase == other.mNonDropFrameBase); }
						bool			operator!=(const Framerate& other) const
											{ return (mKind != other.mKind) ||
													(mNonDropFrameBase != other.mNonDropFrameBase); }

										// Class methods
				static	Framerate		forNonDropFrame(UInt32 base)
											{ return Framerate(kKindNonDropFrame, base); }
				static	Framerate		forDropFrame2997()
											{ return Framerate(kKindDropFrame2997); }
				static	Framerate		forDropFrame5994()
											{ return Framerate(kKindDropFrame5994); }

				static	OV<Framerate>	fromInfo(const CDictionary& info);

			private:
										// Lifecycle methods
										Framerate(Kind kind, UInt32 nonDropFrameBase) :
											mKind(kind), mNonDropFrameBase(nonDropFrameBase)
											{}
										Framerate(Kind kind) : mKind(kind) {}

			// Properties
			public:
				static	Framerate	mDefault;

			private:
						Kind		mKind;
						OV<UInt32>	mNonDropFrameBase;
		};

	// Methods
	public:
								// Lifecycle methods
								STimecode(SInt32 frameIndex,
										const Framerate& framerate = Framerate::forNonDropFrame(24)) :
									mFrameIndex(frameIndex), mFramerate(framerate)
									{}
								STimecode(SInt32 hours, SInt32 minutes, SInt32 seconds, SInt32 frames,
										const Framerate& framerate = Framerate::forNonDropFrame(24));
								STimecode(const STimecode& other) :
									mFrameIndex(other.mFrameIndex), mFramerate(other.mFramerate)
									{}

								// Instance methods
				UInt32			getFrameIndex() const
									{ return mFrameIndex; }
				CString			getDisplayString() const;

				STimecode		addingFrames(SInt32 frameCount) const
									{ return STimecode(mFrameIndex + frameCount, mFramerate); }

				Float64			getSeconds() const
									{ return (Float64) mFrameIndex / (Float64) mFramerate.getFramerate(); }

				CDictionary		getInfo() const;

				bool			operator==(const STimecode& other) const
									{ return (mFrameIndex == other.mFrameIndex) && (mFramerate == other.mFramerate); }
				bool			operator!=(const STimecode& other) const
									{ return (mFrameIndex != other.mFrameIndex) || (mFramerate != other.mFramerate); }

								// Class methods
		static	OV<STimecode>	fromInfo(const CDictionary& info);
		static	OV<STimecode>	fromString(const CString& string,
										const Framerate& framerate = Framerate::forNonDropFrame(24));

	// Properties
	private:
		SInt32		mFrameIndex;
		Framerate	mFramerate;
};
