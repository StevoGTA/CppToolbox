//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CNotificationCenter

class CNotificationCenterInternals;
class CNotificationCenter {
	// Sender
	public:
		class Sender {
			public:
								// Lifecycle methods
								Sender() {}
				virtual			~Sender() {}

								// Instance methods
						bool	operator!=(const Sender& other) const
									{ return !operator==(other); }

			protected:
								// Subclass methods
				virtual	bool	operator==(const Sender& other) const
									{ return false; }
		};

	// TSender
	public:
		template <typename T> class TSender : public Sender {
			public:
								// Lifecycle methods
								TSender(const T& t) : mT(t) {}

								// Instance methods
				const	T&		operator*() const
									{ return mT; }

			protected:
								// Sender methods
						bool	operator==(const Sender& other) const
									{ return mT == ((const TSender<T>&) other).mT; }

			// Properties
			private:
				T	mT;
		};

	// Observer
	public:
		struct Observer {
			// Procs
			typedef	void	(*Proc)(const CString& notificationName, const OI<Sender>& sender, const CDictionary& info,
									void* userData);

					// Lifecycle methods
					Observer(const void* observerRef, Proc proc, void* userData) :
						mObserverRef(observerRef), mProc(proc), mUserData(userData)
						{}
					Observer(const Observer& other) :
						mObserverRef(other.mObserverRef), mProc(other.mProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	callProc(const CString& notificationName, const OI<Sender>& sender, const CDictionary& info) const
						{ mProc(notificationName, sender, info, mUserData); }

			// Properties
			const	void*	mObserverRef;
					Proc	mProc;
					void*	mUserData;
		};

	// Methods
	public:
						// Lifcycle methods
		virtual			~CNotificationCenter();

						// Instance methods
				void	registerObserver(const CString& notificationName, const OI<Sender>& sender,
								const Observer& observer);
				void	registerObserver(const CString& notificationName, const Observer& observer);
				void	unregisterObserver(const CString& notificationName, const void* observerRef);
				void	unregisterObserver(const void* observerRef);

		virtual	void	queue(const CString& notificationName, const OI<Sender>& sender = OI<Sender>(),
								const CDictionary& info = CDictionary::mEmpty) = 0;

	protected:
						// Lifcycle methods
						CNotificationCenter();

						// Instance methods
				void	send(const CString& notificationName, const OI<Sender>& sender, const CDictionary& info) const;

	// Properties
	private:
		CNotificationCenterInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CImmediateNotificationCenter

class CImmediateNotificationCenter : public CNotificationCenter {
	// Methods
	public:
				// Lifecycle methods
				CImmediateNotificationCenter() {}

				// CNotificationCenter methods
		void	queue(const CString& notificationName, const OI<Sender>& sender = OI<Sender>(),
						const CDictionary& info = CDictionary::mEmpty)
					{ send(notificationName, sender, info); }
};
