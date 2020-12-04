//----------------------------------------------------------------------------------------------------------------------
//	CWorkItem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItem

class CWorkItemInternals;
class CWorkItem {
	// Enums
	public:
		enum Priority {
			kPriorityHigh,
			kPriorityNormal,
			kPriorityBackground,
		};

		enum State {
			kStateWaiting,
			kStateActive,
			kStateCompleted,
			kStateCancelled,
		};

	// Methods
	public:
						// Lifecycle methods
						CWorkItem();
		virtual			~CWorkItem();

						// Instance methods
				State	getState() const;
				bool	isWaiting() const
							{ return getState() == kStateWaiting; }
				bool	isActive() const
							{ return getState() == kStateActive; }
				bool	isCompleted() const
							{ return getState() == kStateCompleted; }
				bool	isCancelled() const
							{ return getState() == kStateCancelled; }

						// Subclass methods
		virtual	void	perform() = 0;

		virtual	void	completed() const {}
		virtual	void	cancelled() const {}

						// Internal-use only methods
				void	transitionTo(State state);

	// Properties
	private:
		CWorkItemInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProcWorkItem

class CProcWorkItem : public CWorkItem {
	// Procs
	public:
		typedef	void	(*Proc)(CWorkItem& workItem, void* userData);

	// Methods
	public:
				// Lifecycle methods
				CProcWorkItem(Proc proc, void* userData) : CWorkItem(), mProc(proc), mUserData(userData) {}

				// CWorkItem methods
		void	perform()
					{ mProc(*this, mUserData); }

	// Properties
	private:
		Proc	mProc;
		void*	mUserData;
};
