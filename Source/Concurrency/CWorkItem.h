//----------------------------------------------------------------------------------------------------------------------
//	CWorkItem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItem

class CWorkItemInternals;
class CWorkItem : public CHashable {
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

	// Types
	typedef	void	(*CompletedProc)(void* userData);
	typedef	void	(*CancelledProc)(void* userData);

	// Methods
	public:
										// Lifecycle methods
										CWorkItem(const CString& id = CUUID().getBase64String(),
												const OV<CString>& reference = OV<CString>(),
												CompletedProc completedProc = nil, CancelledProc cancelledProc = nil,
												void* userData = nil);
		virtual							~CWorkItem();

										// CEquatable methods
						bool			operator==(const CEquatable& other) const
											{ return getID() == ((const CWorkItem&) other).getID(); }

										// CHashable methods
						void			hashInto(CHasher& hasher) const
											{ getID().hashInto(hasher); }

										// Instance methods
				const	CString&		getID() const;
				const	OV<CString>&	getReference() const;

						State			getState() const;
						bool			isWaiting() const
											{ return getState() == kStateWaiting; }
						bool			isActive() const
											{ return getState() == kStateActive; }
						bool			isCompleted() const
											{ return getState() == kStateCompleted; }
						bool			isCancelled() const
											{ return getState() == kStateCancelled; }

										// Subclass methods
		virtual			void			perform() = 0;

										// Internal-use only methods
						void			transitionTo(State state);

	protected:
										// Subclass methods
		virtual			void			completed() const {}
		virtual			void			cancelled() const {}

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
