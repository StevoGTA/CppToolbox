//----------------------------------------------------------------------------------------------------------------------
//	COperationQueue.h			©2012 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: Procs

class COperation;
typedef	void	(*CProcOperationProc)(void* userData, COperation& operation);

//----------------------------------------------------------------------------------------------------------------------
// MARK: Priority

enum EOperationPriority {
	kOperationPriorityHigh,
	kOperationPriorityNormal,
	kOperationPriorityBackground,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COperation

class COperationInternals;
class COperation {
	// Methods
	public:
						// Lifecycle methods
						COperation(bool disposeOnCompletion = false);
		virtual			~COperation();
		
						// Instance methods
				void	cancel();
				bool	isCancelled() const;

				void	setActive(bool isActive);
				bool	isActive() const;

		virtual	void	perform() = 0;

		virtual	void	finished() const;

	// Properties
	private:
		COperationInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COperationQueue

class COperationQueueInternals;
class COperationQueue {
	// Methods
	public:
							// Lifecycle methods
							COperationQueue();
		virtual				~COperationQueue();
		
							// Instance methods
				void		add(COperation& operation, EOperationPriority priority = kOperationPriorityNormal);
				COperation&	add(CProcOperationProc proc, void* userData,
									EOperationPriority priority = kOperationPriorityNormal);

				void		pause();
				void		resume();

	// Properties
	public:
		static	COperationQueue&			mDefault;

	private:
				COperationQueueInternals*	mInternals;
};
