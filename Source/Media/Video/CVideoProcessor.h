//----------------------------------------------------------------------------------------------------------------------
//	CVideoProcessor.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoFrame.h"
#include "SVideoSourceStatus.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoProcessor

class CVideoProcessorInternals;
class CVideoProcessor {
	// PerformResult
	public:
		struct PerformResult {
											// Lifecycle methods
											PerformResult(UniversalTimeInterval timeInterval,
													const CVideoFrame& videoFrame) :
												mVideoSourceStatus(timeInterval), mVideoFrame(videoFrame)
												{}
											PerformResult(const SError& error) : mVideoSourceStatus(error) {}
											PerformResult(const PerformResult& other) :
												mVideoSourceStatus(other.mVideoSourceStatus),
														mVideoFrame(other.mVideoFrame)
											{}

											// Instance methods
				const	SVideoSourceStatus&	getVideoSourceStatus() const
												{ return mVideoSourceStatus; }
				const	CVideoFrame&		getVideoFrame() const
												{ return *mVideoFrame; }

			private:
				SVideoSourceStatus	mVideoSourceStatus;
				OI<CVideoFrame>		mVideoFrame;
		};

	// Methods
	public:
								// Lifecycle methods
								CVideoProcessor();
		virtual					~CVideoProcessor();

								// Instance methods
		virtual	OI<SError>		connectInput(const I<CVideoProcessor>& videoProcessor);

		virtual	void			setSourceWindow(UniversalTimeInterval startTimeInterval,
										const OV<UniversalTimeInterval>& durationTimeInterval);
		virtual	void			seek(UniversalTimeInterval timeInterval);

		virtual	PerformResult	perform();
		virtual	void			reset();

	// Properties
	private:
		CVideoProcessorInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoSource

class CVideoSource : public CVideoProcessor {
	// Methods
	public:
		// Lifecycle methods
		CVideoSource() : CVideoProcessor() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoDestination

class CVideoDestination : public CVideoProcessor {
	// Methods
	public:
		// Lifecycle methods
		CVideoDestination() : CVideoProcessor() {}
};
