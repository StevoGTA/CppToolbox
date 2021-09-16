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
			typedef	void	(*ItemPrepareStartedProc)(const I<Item>& item, void* userData);
			typedef	void	(*ItemPrepareCompletedProc)(const I<Item>& item,
									const TIResult<I<CMediaPlayer> >& mediaPlayer, void* userData);

					// Lifecycle methods
					Info(ItemPrepareStartedProc itemPrepareStartedProc,
							ItemPrepareCompletedProc itemPrepareCompletedProc, void* userData) :
						mItemPrepareStartedProc(itemPrepareStartedProc),
								mItemPrepareCompletedProc(itemPrepareCompletedProc), mUserData(userData)
						{}
					Info(const Info& other) :
						mItemPrepareStartedProc(other.mItemPrepareStartedProc),
								mItemPrepareCompletedProc(other.mItemPrepareCompletedProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	itemPrepareStarted(const I<Item>& item)
						{ mItemPrepareStartedProc(item, mUserData); }
			void	itemPrepareCompleted(const I<Item>& item, const TIResult<I<CMediaPlayer> >& mediaPlayer)
						{ mItemPrepareCompletedProc(item, mediaPlayer, mUserData); }

			// Properties
			private:
				ItemPrepareStartedProc		mItemPrepareStartedProc;
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
