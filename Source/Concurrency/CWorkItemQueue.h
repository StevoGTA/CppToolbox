//----------------------------------------------------------------------------------------------------------------------
//	CWorkItemQueue.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CWorkItem.h"
#include "PlatformDefinitions.h"

/*!
	The Basics:

	Work Item Queue, along with its companion Work Item, is the fundamental mechanism to perform multitasking.

	A Work Item Queue allows for queuing of Work Items and manages their state and execution.
	To add a task (called a Work Item) to a Work Item Queue, call one of the add() methods.  This Work Item will be
		executed on a background thread as soon as one is available.  Additional Work Items can be queued as needed.
	When adding Work Items to a Work Item Queue, it is possible to specify a priority.  Higher priority Work Items will
		be performed before lower priority Work Items.
	Work Item Queues are thread-safe meaning that any method can be called from any thread at any time.

	Work Items are the fundamental object for performing work.
	Work Items have state and are created in the waiting state.
	When a Work Item is being performed, it transition to the active state.
	When a Work Item has completed, it will transition to the completed state and the completed() method will be called.
	When a Work Item has been cancelled, at some time in the future, it will transition to the cancelled state and the
		cancelled() method will be called.
	Subclass must override the perform() method and may also choose to override the completed() and cancelled() methods
		if it is desired to be informed of those state changes.
	Work Items can be configured to perform their own cleanup if desired.


	The Main Work Item Queue

	The Main Work Item Queue is the "master" Work Item Queue and is the Work Item Queue that actually controls and
		performs the Work Items.  Additional Work Item Queues (see below) will ultimately target the Main Work Item
		Queue.
	The Main Work Item Queue has a built-in maximum concurrent items limit of the number of processor cores minus one
		(to try to leave the main thread available for UI and other minor work).


	The Advanced:

	Additional Work Item Queues can be created in order to group Work Items together and be able to perform actions on
		the group and receive callbacks when the group has been processed.  Additional Work Item Queues do not perform
		any action on their own, but become feeder queues for the Main Work Item Queue.
	Additional Work Item Queues have a maximum concurrent items parameter that allows for finer control over how many
		Work Items in this Work Item Queue can be active at any given time.  Note that this parameter has an inherent
		maximum of the number of processor cores minus one as that is the overall maximum number of Work Items that can
		be active at any given time.  This parameter does not perform magic if set higher.  If this parameter is 1,
		this Work Item Queue can be considered a serial Work Item Queue
	Additional Work Item Queues can have a target Work Item Queue if desired.  The additional Work Item Queue is subject
		to the target Work Item Queue maximum concurrent items limit, meaning that no Work Item Queue can have more
		active Work Items by combining itself plus all "child" Work Item Queues than its own maximum concurrent items
		limit.  This allows for creating rather complex hierarchies of Work Item Queues and imposing limits throughout
		the hierarchy.  For example, 6 serial Work Item Queues targeting a single Work Item Queue with a maximum
		concurrent item limit of 4 means that only 4 Work Items will ever be active at any given time.
	All Work Items in a Work Item Queue can be paused by calling the pause() method.  This will mark all Work Items in
		that Work Item Queue as paused.  This only means that these Work Items will not be made active in the future and
		does not affect Work Items already processing.  Subsequently, Work Items can be un-paused by calling the
		resume() method on the Work Item Queue.

	The Work Item Queue system also tracks the order in which Work Items are created and, everything else being equal,
		will perform the Work Item created first.
 */

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItemQueue

class CWorkItemQueueInternals;
class CWorkItemQueue {
	// Methods
	public:
								// Lifecycle methods
								CWorkItemQueue(UInt32 maximumConcurrentWorkItems = ~0);
								CWorkItemQueue(CWorkItemQueue& targetWorkItemQueue,
										UInt32 maximumConcurrentWorkItems = ~0);
		virtual					~CWorkItemQueue();
		
								// Instance methods
				void			add(CWorkItem& workItem, EWorkItemPriority priority = kWorkItemPriorityNormal);
				CWorkItem&		add(CWorkItemProc proc, void* userData,
										EWorkItemPriority priority = kWorkItemPriorityNormal);

				void			cancel(CWorkItem& workItem);

				void			pause();
				void			resume();

								// Class methods
		static	CWorkItemQueue&	main();

	// Properties
	private:
		CWorkItemQueueInternals*	mInternals;
};
