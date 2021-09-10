//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlaybackQueue.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CMediaPlayer.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlaybackQueue

class CMediaPlaybackQueueInternals;
class CMediaPlaybackQueue {
	// Item
	public:
		class Item {
			// Methods
			public:
													Item() {}
				virtual								~Item() {}

				virtual	TIResult<I<CMediaPlayer> >	prepare() = 0;
						void						cancel() {}
		};

	// Info
	public:
		struct Info {
			// Types
			typedef	void	(*ItemPrepareCompletedProc)(const TIResult<I<CMediaPlayer> >& mediaPlayer, void* userData);

					// Lifecycle methods
					Info(ItemPrepareCompletedProc itemPrepareCompletedProc, void* userData) :
						mItemPrepareCompletedProc(itemPrepareCompletedProc), mUserData(userData)
						{}
					Info(const Info& other) :
						mItemPrepareCompletedProc(other.mItemPrepareCompletedProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	itemPrepareCompleted(const TIResult<I<CMediaPlayer> >& mediaPlayer)
						{ mItemPrepareCompletedProc(mediaPlayer, mUserData); }

			// Properties
			private:
				ItemPrepareCompletedProc	mItemPrepareCompletedProc;
				void*						mUserData;
		};

	// Methods
	public:
				// Lifecycle methods
				CMediaPlaybackQueue(CSRSWMessageQueues& messageQueues, const Info& info);
				~CMediaPlaybackQueue();

				// Instance methods
		void	set(const TArray<I<Item> >& items);

	// Properties
	private:
		CMediaPlaybackQueueInternals*	mInternals;
};
