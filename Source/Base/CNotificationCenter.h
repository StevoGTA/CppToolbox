//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CNotificationCenter

class CNotificationCenter {
	// Sender
	public:
		class Sender : public CReferenceCountable {
			// Methods
			public:
								// Lifecycle methods
								Sender() : CReferenceCountable() {}
								Sender(const Sender& other) : CReferenceCountable(other) {}

								// Instance methods
				virtual	Sender*	copy() const = 0;
				virtual	bool	operator==(const Sender& other) const = 0;
		};

	// ISender - Pass pointer to sender that will have delete called at cleanup
	public:
		template <typename T> class ISender : public Sender {
			// Methods
			public:
								// Lifecycle methods
								ISender(const T* t) : Sender(), mT(t) {}
								ISender(const ISender& other) : Sender(other), mT(other.mT) {}
								~ISender()
									{
										// Check if last reference
										if (getReferenceCount() == 1)
											// Cleanup
											Delete(mT);
									}

								// Instance methods
				const	T&		operator*() const
									{ return *mT; }

			protected:
								// Sender methods
						Sender*	copy() const
									{ return new ISender(*this); }

						bool	operator==(const Sender& other) const
									{ return *mT == *((const ISender<T>&) other).mT; }

			// Properties
			private:
				const	T*	mT;
		};

	// RSender - Pass reference to sender
	public:
		template <typename T> class RSender : public Sender {
			// Methods
			public:
						// Lifecycle methods
						RSender(T& t) : Sender(), mT(t) {}
						RSender(const RSender& other) : Sender(other), mT(other.mT) {}

						// TReferenceCountable methods
				void	cleanup()
							{}

						// Instance methods
				T&		operator*() const
							{ return mT; }

			protected:
						// Sender methods
				Sender*	copy() const
							{ return new RSender(*this); }

				bool	operator==(const Sender& other) const
							{ return mT == ((const RSender<T>&) other).mT; }

			// Properties
			private:
				T&	mT;
		};

	// VSender - Pass by value to sender
	public:
		template <typename T> class VSender : public Sender {
			// Methods
			public:
								// Lifecycle methods
								VSender(const T& t) : Sender(), mT(t) {}
								VSender(const VSender& other) : Sender(other), mT(other.mT) {}

								// TReferenceCountable methods
						void	cleanup()
									{}

								// Instance methods
				const	T&		operator*() const
									{ return mT; }

			protected:
								// Sender methods
						Sender*	copy() const
									{ return new VSender(*this); }

						bool	operator==(const Sender& other) const
									{ return *mT == *((const VSender<T>&) other).mT; }

			// Properties
			private:
				T	mT;
		};

	// Observer
	public:
		struct Observer {
			// Types
			typedef	const void*	Ref;

			// Procs
			typedef	void	(*Proc)(const CString& notificationName, const OR<Sender>& sender, const CDictionary& info,
									void* userData);

					// Lifecycle methods
					Observer(Ref ref, Proc proc, void* userData) : mRef(ref), mProc(proc), mUserData(userData) {}
					Observer(const Observer& other) :
						mRef(other.mRef), mProc(other.mProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	callProc(const CString& notificationName, const OR<Sender>& sender, const CDictionary& info) const
						{ mProc(notificationName, sender, info, mUserData); }

			// Properties
			Ref		mRef;
			Proc	mProc;
			void*	mUserData;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
						// Lifcycle methods
		virtual			~CNotificationCenter();

						// Instance methods
				void	registerObserver(const CString& notificationName, const Sender& sender,
								const Observer& observer);
				void	registerObserver(const CString& notificationName, const Observer& observer);
				void	unregisterObserver(const CString& notificationName, Observer::Ref observerRef);
				void	unregisterObserver(Observer::Ref observerRef);

		virtual	void	queue(const CString& notificationName, const Sender& sender, const CDictionary& info) = 0;
				void	queue(const CString& notificationName, const Sender& sender)
							{ queue(notificationName, sender, CDictionary::mEmpty); }
		virtual	void	queue(const CString& notificationName, const CDictionary& info) = 0;
				void	queue(const CString& notificationName)
							{ queue(notificationName, CDictionary::mEmpty); }

	protected:
						// Lifcycle methods
						CNotificationCenter();

						// Instance methods
				void	send(const CString& notificationName, const OR<Sender>& sender, const CDictionary& info) const;

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CImmediateNotificationCenter

class CImmediateNotificationCenter : public CNotificationCenter {
	// Methods
	public:
				// Lifecycle methods
				CImmediateNotificationCenter() {}

				// CNotificationCenter methods
		void	queue(const CString& notificationName, const Sender& sender, const CDictionary& info)
					{ send(notificationName, OR<Sender>((Sender&) sender), info); }
		void	queue(const CString& notificationName, const Sender& sender)
					{ CNotificationCenter::queue(notificationName, sender); }
		void	queue(const CString& notificationName, const CDictionary& info)
					{ send(notificationName, OR<Sender>(), info); }
		void	queue(const CString& notificationName)
					{ CNotificationCenter::queue(notificationName); }
};
