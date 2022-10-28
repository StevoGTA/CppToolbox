//----------------------------------------------------------------------------------------------------------------------
//	CProgress.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProgress

class CProgressInternals;
class CProgress {
	// UpdateInfo
	public:
		struct UpdateInfo {
			// Types
			typedef	void	(*Proc)(const CProgress& progress, void* userData);

					// Methods
					UpdateInfo(Proc proc, void* userData) :
						mProc(proc), mUserData(userData)
						{}
					UpdateInfo(const UpdateInfo& other) :
						mProc(other.mProc), mUserData(other.mUserData)
						{}

			void	notify(const CProgress& progress)
						{ mProc(progress, mUserData); }

			// Properties
			private:
				Proc	mProc;
				void*	mUserData;
		};

	// Methods
	public:
										// Lifecycle methods
										CProgress(const UpdateInfo& updateInfo);
										CProgress(const CProgress& other);
		virtual							~CProgress();

										// Instance methods
				const	CString&		getMessage() const;
						void			setMessage(const CString& message);

				const	OV<Float32>&	getValue() const;
						void			setValue(OV<Float32> value);

	// Properties
	private:
		CProgressInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CItemsProgress
/*
	Always has a completed items count.
	Once there is a total items count, the CProgress value will be set to the ratio.

	If you want to indicate an indeterminate state, delay setting total items count until you are ready for actual,
		determinate progress.
 */

class CItemsProgressInternals;
class CItemsProgress : public CProgress {
	// Methods
	public:
				// Lifecycle methods
				CItemsProgress(const UpdateInfo& updateInfo, const OV<UInt32>& initialTotalItemsCount = OV<UInt32>());
				CItemsProgress(const CItemsProgress& other);
				~CItemsProgress();

				// Instance methods
		void	addTotalItemsCount(UInt32 itemsCount);
		void	addCompletedItemsCount(UInt32 itemsCount);

	// Properties
	private:
		CItemsProgressInternals*	mInternals;
};
