//----------------------------------------------------------------------------------------------------------------------
//	CProgress.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProgress

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

	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CProgress(const UpdateInfo& updateInfo);
		virtual							~CProgress();

										// Instance methods
				const	CString&		getMessage() const;
						void			setMessage(const CString& message);

				const	OV<Float32>&	getValue() const;
						void			setValue(Float32 value);
						void			removeValue();

	protected:
										// Lifecycle methods
										CProgress(const UpdateInfo& updateInfo, Float32 initialValue);

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CItemsProgress
/*
	Always has a completed items count.
	Once there is a total items count, the CProgress value will be set to the ratio.

	If you want to indicate an indeterminate state, delay setting total items count until you are ready for actual,
		determinate progress.
 */

class CItemsProgress : public CProgress {
	// Classes
	private:
		class Internals;

	// Methods
	public:
					// Lifecycle methods
					CItemsProgress(const UpdateInfo& updateInfo, UInt32 initialTotalItemsCount);
					CItemsProgress(const UpdateInfo& updateInfo);
					~CItemsProgress();

					// Instance methods
		void		addTotalItemsCount(UInt32 itemsCount);
		OV<UInt32>	getTotalItemsCount() const;

		void		addCompletedItemsCount(UInt32 itemsCount);
		UInt32		getCompletedItemsCount() const;

		void		reset();

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTimeIntervalProgress

class CTimeIntervalProgress : public CProgress {
	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CTimeIntervalProgress(const UpdateInfo& updateInfo,
										UniversalTimeInterval totalTimeInterval);
								~CTimeIntervalProgress();

								// Instance methods
		UniversalTimeInterval	getTotalTimeInterval() const;

		UniversalTimeInterval	getTimeInterval() const;
		void					setTimeInterval(UniversalTimeInterval timeInterval);
		void					complete();

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAggregateTimeIntervalProgress

class CAggregateTimeIntervalProgress : public CProgress {
	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CAggregateTimeIntervalProgress(const UpdateInfo& updateInfo);
								~CAggregateTimeIntervalProgress();

								// Instance methods
		CTimeIntervalProgress&	addTimeIntervalProgress(UniversalTimeInterval totalTimeInterval);
		void					reset();

	// Properties
	private:
		Internals*	mInternals;
};
