//----------------------------------------------------------------------------------------------------------------------
//	CApplication.h			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"
#include "TResult.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	#include "CCoreFoundation.h"
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CApplication

class CApplication {
	// Classes
	private:
		class Internals;

	// Methods
	public:
									// Lifecycle methods
									CApplication(const CApplication& other);
									~CApplication();

									// Instance methods
				CString				getDisplayName() const;
				CString				getVersion() const;
				CString				getDisplayNameAndVersion() const;
				OV<CString>			getCopyright() const;
				CFolder				getFolder() const;

#if defined(TARGET_OS_MACOS) || defined(TARGET_OS_WINDOWS)
				OV<SError>			open(const TArray<CFile>& files) const;
#endif

				TVResult<CData>		getStorageData() const;

									// Class methods
		static	CApplication		getCurrent();

		static	OV<CApplication>	getFrom(const CData& storageData);
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		static	OV<CApplication>	getFor(CFURLRef urlRef);
#endif
#if defined(TARGET_OS_WINDOWS)
		static	OV<CApplication>	getFor(const CFilesystemPath& filesystemPath);
#endif

	private:
									// Lifecycle methods
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
									CApplication(CFURLRef urlRef, const CString& displayName, const CString& version,
											const OV<CString>& copyright);
#endif
#if defined(TARGET_OS_WINDOWS)
									CApplication(const CFilesystemPath& filesystemPath, const CString& displayName,
											const CString& version, const OV<CString>& copyright);
#endif

	// Properties
	private:
		Internals*	mInternals;
};
