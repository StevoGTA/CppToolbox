//----------------------------------------------------------------------------------------------------------------------
//	CDynamicLibrary-Windows.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDynamicLibrary.h"

#include "CReferenceCountable.h"
#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDynamicLibrary::Internals

class CDynamicLibrary::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const CFile& file, CDynamicLibrary::Handle handle) :
			TReferenceCountableAutoDelete(),
					mFile(file), mHandle(handle)
			{}
		~Internals()
			{ ::FreeLibrary(mHandle); }

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
	FARPROC	address = ::GetProcAddress(mInternals->mHandle, *name.getUTF8String());

	return (address != NULL) ? OV<void*>((void*) address) : OV<void*>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDynamicLibrary> CDynamicLibrary::load(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Load, resolving the library's own dependencies from its folder first
	Handle	handle =
					::LoadLibraryEx(file.getFilesystemPath().getString().getOSString(), NULL,
							LOAD_WITH_ALTERED_SEARCH_PATH);
	if (handle == NULL)
		// Error
		return TVResult<CDynamicLibrary>(SErrorFromWindowsGetLastError());

	return TVResult<CDynamicLibrary>(CDynamicLibrary(file, handle));
}
