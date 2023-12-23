//----------------------------------------------------------------------------------------------------------------------
//	CVideoProcessor.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoFrame.h"
#include "SMedia.h"
#include "TimeAndDate.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoProcessor

class CVideoProcessor {
	// Format
	public:
		struct Format {

								// Lifecycle methods
								Format(const S2DSizeU16& frameSize, Float32 framerate) :
									mFrameSize(frameSize), mFramerate(framerate)
									{}

								// Instance methods
			const	S2DSizeU16&	getFrameSize() const
									{ return mFrameSize; }
					Float32		getFramerate() const
									{ return mFramerate; }

			// Properties
			private:
				S2DSizeU16	mFrameSize;
				Float32		mFramerate;
		};

	// PerformResult
	public:
		struct PerformResult {
															// Lifecycle methods
															PerformResult(UniversalTimeInterval timeInterval,
																	const CVideoFrame& videoFrame) :
																mResult(timeInterval), mVideoFrame(videoFrame)
																{}
															PerformResult(const SError& error) : mResult(error) {}
															PerformResult(const PerformResult& other) :
																mResult(other.mResult), mVideoFrame(other.mVideoFrame)
															{}

															// Instance methods
				const	TVResult<UniversalTimeInterval>&	getResult() const
																{ return mResult; }
				const	CVideoFrame&						getVideoFrame() const
																{ return *mVideoFrame; }

			private:
				TVResult<UniversalTimeInterval>	mResult;
				OI<CVideoFrame>					mVideoFrame;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
									// Lifecycle methods
									CVideoProcessor();
		virtual						~CVideoProcessor();

									// Instance methods
		virtual	OV<SError>			connectInput(const I<CVideoProcessor>& videoProcessor);
		virtual	TArray<CString>		getSetupDescription(const CString& indent);

		virtual	void				setMediaSegment(const OV<SMedia::Segment>& mediaSegment);
		virtual	void				seek(UniversalTimeInterval timeInterval);

		virtual	PerformResult		perform();
		virtual	void				reset();

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoSource

class CVideoSource : public CVideoProcessor {
	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CVideoSource();
								~CVideoSource();

								// CVideoProcessor methods
		TArray<CString>			getSetupDescription(const CString& indent)
									{ return TNArray<CString>(); }

		void					setMediaSegment(const OV<SMedia::Segment>& mediaSegment);
		void					seek(UniversalTimeInterval timeInterval);

		void					reset();

	protected:
								// Instance methods
		UniversalTimeInterval	getCurrentTimeInterval() const;
		void					setCurrentTimeInterval(UniversalTimeInterval timeInterval);

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoDestination

class CVideoDestination : public CVideoProcessor {
	// Methods
	public:
						// Lifecycle methods
						CVideoDestination() : CVideoProcessor() {}

						// Instance methods
		virtual	void	setupComplete() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoDestinationNone

class CVideoDestinationNone : public CVideoDestination {
	// Methods
	public:
				// Lifecycle methods
				CVideoDestinationNone() : CVideoDestination() {}

				// Instance methods
		void	setupComplete() {}
};
