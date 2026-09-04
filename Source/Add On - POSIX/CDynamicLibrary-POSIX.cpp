//----------------------------------------------------------------------------------------------------------------------
//	CDynamicLibrary-POSIX.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDynamicLibrary.h"

#include "CReferenceCountable.h"

#include <cstring>
#include <dlfcn.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDynamicLibrary::Internals

class CDynamicLibrary::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const CFile& file, CDynamicLibrary::Handle handle) :
			TReferenceCountableAutoDelete(),
					mFile(file), mHandle(handle)
			{}
		~Internals()
			{ ::dlclose(mHandle); }

		CFile					mFile;
		CDynamicLibrary::Handle	mHandle;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDynamicLibrary

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDynamicLibrary::CDynamicLibrary(const CDynamicLibrary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CDynamicLibrary::CDynamicLibrary(const CFile& file, Handle handle)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(file, handle);
}

//----------------------------------------------------------------------------------------------------------------------
CDynamicLibrary::~CDynamicLibrary()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CFile& CDynamicLibrary::getFile() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFile;
}

//----------------------------------------------------------------------------------------------------------------------
OV<void*> CDynamicLibrary::getSymbolAddress(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Look up
	void*	address = ::dlsym(mInternals->mHandle, *name.getUTF8String());

	return (address != nil) ? OV<void*>(address) : OV<void*>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDynamicLibrary> CDynamicLibrary::load(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Load
	Handle	handle = ::dlopen(*file.getFilesystemPath().getString().getUTF8String(), RTLD_NOW | RTLD_LOCAL);
	if (handle == nil) {
		// Error
		const	char*	message = ::dlerror();

		return TVResult<CDynamicLibrary>(
				SError(CString(OSSTR("CDynamicLibrary")), 1,
						CString(message, (CString::Length) ::strlen(message), CString::kEncodingUTF8)));
	}

	return TVResult<CDynamicLibrary>(CDynamicLibrary(file, handle));
}
