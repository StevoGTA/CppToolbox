//----------------------------------------------------------------------------------------------------------------------
//	CWorkItem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: Priority

enum EWorkItemPriority {
	kWorkItemPriorityHigh,
	kWorkItemPriorityNormal,
	kWorkItemPriorityBackground,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - State

enum EWorkItemState {
	kWorkItemStateWaiting,
	kWorkItemStateActive,
	kWorkItemStateCompleted,
	kWorkItemStateCancelled,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Procs

class CWorkItem;
typedef	void	(*CWorkItemProc)(void* userData, CWorkItem& workItem);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItem

class CWorkItemInternals;
class CWorkItem {
	// Methods
	public:
								// Lifecycle methods
								CWorkItem(bool disposeWhenCompletedOrCancelled = false);
		virtual					~CWorkItem();

								// Instance methods
				EWorkItemState	getState() const;
				bool			isWaiting() const
									{ return getState() == kWorkItemStateWaiting; }
				bool			isActive() const
									{ return getState() == kWorkItemStateActive; }
				bool			isCompleted() const
									{ return getState() == kWorkItemStateCompleted; }
				bool			isCancelled() const
									{ return getState() == kWorkItemStateCancelled; }

								// Subclass methods
		virtual	void			perform() = 0;

		virtual	void			completed() const;
		virtual	void			cancelled() const;

								// Internal-use only methods
				void			transitionTo(EWorkItemState state);

	// Properties
	private:
		CWorkItemInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProcWorkItem

class CProcWorkItem : public CWorkItem {
	// Methods
	public:
				// Lifecycle methods
				CProcWorkItem(CWorkItemProc proc, void* userData) : CWorkItem(true), mProc(proc), mUserData(userData) {}
				~CProcWorkItem() {}

				// CWorkItem methods
		void	perform()
					{ mProc(mUserData, *this); }

	// Properties
	private:
		CWorkItemProc	mProc;
		void*			mUserData;
};
