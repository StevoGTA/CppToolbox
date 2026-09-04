//----------------------------------------------------------------------------------------------------------------------
//	CDynamicLibrary.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"
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

	// Classes
	private:
		class Internals;

	// Methods
	public:
																			// Lifecycle methods
																			CDynamicLibrary(
																					const CDynamicLibrary& other);
																			~CDynamicLibrary();

																			// Instance methods
				const							CFile&						getFile() const;

												OV<void*>					getSymbolAddress(const CString& name) const;
						template <typename T>	OV<T>						getSymbol(const CString& name) const
																				{
																					// Get address
																					OV<void*>	address =
																										getSymbolAddress(
																												name);

																					return address.hasValue() ?
																							OV<T>((T) *address) :
																							OV<T>();
																				}

																			// Class methods
		static									TVResult<CDynamicLibrary>	load(const CFile& file);

	private:
																			// Lifecycle methods
																			CDynamicLibrary(const CFile& file,
																					Handle handle);

	// Properties
	private:
		Internals*	mInternals;
};
