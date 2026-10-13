//----------------------------------------------------------------------------------------------------------------------
//	CDynamicLibrary.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"
#include "SVersionInfo.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDynamicLibrary

class CDynamicLibrary {
	// Types
	public:
#if defined(TARGET_OS_MACOS)
		typedef	void*				Handle;
#elif defined(TARGET_OS_WINDOWS)
		typedef	struct HINSTANCE__*	Handle;		// HMODULE
#endif

	// Loadability
	public:
		struct Loadability {
			// State
			public:
				enum State {
					kStatePossible,

					kStateUnreadable,
					kStateWrongArchitecture,
					kStateNewerOSRequired,
					kStateNotYetApproved,
				};

			// Methods
			public:
											// Lifecycle methods
											Loadability(const SVersionInfo& requiredOSVersion) :
												mState(kStateNewerOSRequired), mRequiredOSVersion(requiredOSVersion)
												{}
											Loadability(State state) : mState(state) {}
											Loadability(const Loadability& other) :
												mState(other.mState), mRequiredOSVersion(other.mRequiredOSVersion)
												{}

											// Instance methods
								State		getState() const
												{ return mState; }
				const	OV<SVersionInfo>&	getRequiredOSVersion() const
												{ return mRequiredOSVersion; }

			// Properties
			private:
				State				mState;
				OV<SVersionInfo>	mRequiredOSVersion;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
													// Lifecycle methods
													CDynamicLibrary(const CDynamicLibrary& other);
													~CDynamicLibrary();

													// Instance methods
				const	CFile&						getFile() const;

						OV<void*>					getSymbolAddress(const CString& name) const;

						template <typename T>
						OV<T>						getSymbol(const CString& name) const
														{
															// Get address
															OV<void*>	address = getSymbolAddress(name);

															return address.hasValue() ? OV<T>((T) *address) : OV<T>();
														}

													// Class methods
		static			OV<SVersionInfo>			getVersionInfo(const CFile& file);
		static			OV<CString>					getManufacturer(const CFile& file);

		static			Loadability					getLoadability(const CFile& file);
		static			TVResult<CDynamicLibrary>	load(const CFile& file);

	private:
													// Lifecycle methods
													CDynamicLibrary(const CFile& file, Handle handle);

	// Properties
	private:
		Internals*	mInternals;
};
