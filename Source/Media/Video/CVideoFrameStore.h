//----------------------------------------------------------------------------------------------------------------------
//	CVideoFrameStore.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoFrameStore

class CVideoFrameStoreInternals;
class CVideoFrameStore : public CVideoDestination {
	// Info
	public:
		struct Info {
			// Procs
			typedef	void	(*CurrentFrameUpdatedProc)(const CVideoFrameStore& videoFrameStore,
									const CVideoFrame& videoFrame, void* userData);
			typedef	void	(*ErrorProc)(const CVideoFrameStore& videoFrameStore, const SError& error, void* userData);

					// Lifecycle methods
					Info(CurrentFrameUpdatedProc currentFrameUpdatedProc, ErrorProc errorProc, void* userData) :
						mCurrentFrameUpdatedProc(currentFrameUpdatedProc), mErrorProc(errorProc), mUserData(userData)
						{}
					Info(const Info& other) :
						mCurrentFrameUpdatedProc(other.mCurrentFrameUpdatedProc), mErrorProc(other.mErrorProc),
								mUserData(other.mUserData)
						{}

					// Instance methods
			void	currentFrameUpdated(const CVideoFrameStore& videoFrameStore, const CVideoFrame& videoFrame) const
						{ mCurrentFrameUpdatedProc(videoFrameStore, videoFrame, mUserData); }
			void	error(const CVideoFrameStore& videoFrameStore, const SError& error) const
						{ mErrorProc(videoFrameStore, error, mUserData); }

			// Properties
			private:
			CurrentFrameUpdatedProc	mCurrentFrameUpdatedProc;
				ErrorProc			mErrorProc;
				void*				mUserData;
		};

	// Methods
	public:
							// Lifecycle methods
							CVideoFrameStore(const CString& identifier, const Info& info);
							~CVideoFrameStore();

							// CVideoProcessor methods
				void		seek(UniversalTimeInterval timeInterval);

				void		reset();

							// CVideoDestination methods
				void		setupComplete();

							// Instance methods
				void		notePositionUpdated(UniversalTimeInterval position);

				void		resume();
				void		pause();

				void		startSeek();
				void		finishSeek();

	// Properties
	private:
		CVideoFrameStoreInternals*	mInternals;
};
