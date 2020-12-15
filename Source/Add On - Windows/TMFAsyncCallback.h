//----------------------------------------------------------------------------------------------------------------------
//	TMFAsyncCallback.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <mfobjects.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: TMFAsyncCallback

template <typename T> struct TMFAsyncCallback : public IMFAsyncCallback {
	// Types
	public:
		typedef	HRESULT	(*InvokeProc)(__RPC__in_opt IMFAsyncResult& asyncResult, T& instance);

	// Methods
	public:
									// Lifecycle methods
									TMFAsyncCallback(T& t, InvokeProc invokeProc) :
										mT(t), mQueueID(MFASYNC_CALLBACK_QUEUE_MULTITHREADED),
												mInvokeProc(invokeProc)
										{}

									// IUnknown methods
		HRESULT	STDMETHODCALLTYPE	QueryInterface(REFIID refIID, void** object)
										{
											// Check interface
											if ((refIID == IID_IUnknown) || (refIID == IID_IMFAsyncCallback)) {
												// We comply
												*object = this;
												AddRef();

												return S_OK;
											} else {
												// Sorry
												*object = nullptr;

												return E_NOINTERFACE;
											}
										}
		ULONG	STDMETHODCALLTYPE	AddRef()
										{ return mT.AddRef(); }
		ULONG	STDMETHODCALLTYPE	Release()
										{ return mT.Release(); }

									// IMFAsyncCallback methods
		HRESULT	STDMETHODCALLTYPE	GetParameters(__RPC__out DWORD* flags, __RPC__out DWORD* queueID)
										{
											// Set values
											*flags = 0;
											*queueID = mQueueID;

											return S_OK;
										}
		HRESULT	STDMETHODCALLTYPE	Invoke(__RPC__in_opt IMFAsyncResult* asyncResult)
										{ return mInvokeProc(*asyncResult, mT); }

									// Instance methods
		void						setQueueID(DWORD queueID)
										{ mQueueID = queueID; }

	// Properties
	private:
		T&			mT;
		DWORD		mQueueID;
		InvokeProc	mInvokeProc;
};
