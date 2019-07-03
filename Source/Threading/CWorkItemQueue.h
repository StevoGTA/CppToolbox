//----------------------------------------------------------------------------------------------------------------------
//	CWorkItemQueue.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: Procs

class CWorkItem;
typedef	void	(*CWorkItemProc)(void* userData, CWorkItem& workItem);

//----------------------------------------------------------------------------------------------------------------------
// MARK: Priority

enum EWorkItemPriority {
	kWorkItemPriorityHigh,
	kWorkItemPriorityNormal,
	kWorkItemPriorityBackground,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItem

class CWorkItemInternals;
class CWorkItem {
	// Methods
	public:
						// Lifecycle methods
						CWorkItem(bool disposeOnCompletion = false);
		virtual			~CWorkItem();
		
						// Instance methods
				void	cancel();
				bool	isCancelled() const;

				void	setActive(bool isActive);
				bool	isActive() const;

		virtual	void	perform() = 0;

		virtual	void	finished() const;

	// Properties
	private:
		CWorkItemInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemQueue

class CWorkItemQueueInternals;
class CWorkItemQueue {
	// Methods
	public:
							// Lifecycle methods
							CWorkItemQueue();
		virtual				~CWorkItemQueue();
		
							// Instance methods
				void		add(CWorkItem& workItem, EWorkItemPriority priority = kWorkItemPriorityNormal);
				CWorkItem&	add(CWorkItemProc proc, void* userData,
									EWorkItemPriority priority = kWorkItemPriorityNormal);

				void		pause();
				void		resume();

	// Properties
	public:
		static	CWorkItemQueue&				mDefault;

	private:
				CWorkItemQueueInternals*	mInternals;
};
