//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "SFoldersFiles.h"
#include "TResult.h"
#include "Tuple.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	#include "CCoreFoundation.h"
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFilesystem

class CFilesystem {
	// FileResult
	public:
		struct FileResult : public TV2<CFile, SError> {
			// Methods
			public:
								// Lifecycle methods
								FileResult(const CFile& file, const SError& error) : TV2(file, error) {}
								FileResult(const FileResult& other) : TV2(other) {}

								// Instamce methods
				const	CFile&	getFile() const { return getA(); }
				const	SError&	getError() const { return getB(); }
		};

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	// SecurityScopedResourceAccess
	public:
		class SecurityScopedResourceAccess {
			// Methods
			public:
															// Lifecycle methods
															SecurityScopedResourceAccess(CFURLRef urlRef);
															~SecurityScopedResourceAccess();

															// Instance methods
				OV<SError>									start();
				void										stop();

															// Class methods
		static	TVResult<I<SecurityScopedResourceAccess> >	fromStorageData(const CData& storageData);

			// Properties
			private:
				bool		mIsActive;
				CFURLRef	mURLRef;
		};
#endif

	// Methods
	public:
												// Class methods
		static	TVResult<SFoldersFiles>			getFoldersFiles(const CFolder& folder, bool deep = false);
		static	TVResult<TArray<CFolder> >		getFolders(const CFolder& folder, bool deep = false);
		static	TVResult<TArray<CFile> >		getFiles(const CFolder& folder, bool deep = false);
		static	CFile							getDotUnderscoreFile(const CFile& file);
#if defined(TARGET_OS_MACOS)
		static	CFile							getResourceFork(const CFile& file);
#endif
		static	OI<I<CRandomAccessDataSource> >	getResourceDataSource(const CFile& file);

												// Will copy sourceFolder *into* destinationFolder
		static	OV<SError>						copy(const CFolder& sourceFolder, const CFolder& destinationFolder);

												// Will copy file *into* destinationFolder
		static	OV<SError>						copy(const CFile& file, const CFolder& destinationFolder);

												// Will copy files *into* destinationFolder
		static	OV<SError>						copy(const TArray<CFile> files, const CFolder& destinationFolder);

												// Will replace destinationFile with sourceFile and remove sourceFile
		static	OV<SError>						replace(const CFile& sourceFile, const CFile& destinationFile);

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		static	CCoreFoundation::O<CFURLRef>	getURLRefFor(const CFolder& folder);
		static	CCoreFoundation::O<CFURLRef>	getURLRefFor(const CFile& file);
		static	CCoreFoundation::OO<CFURLRef>	getURLRefFrom(const CData& storageData);
		static	TVResult<CData>					getStorageDataFor(CFURLRef urlRef);
#endif
#if defined(TARGET_OS_MACOS)
		static	OV<SError>						open(const TArray<CFile>& files, CFURLRef applicationURLRef);
		static	TArray<FileResult>				moveToTrash(const TArray<CFile>& files);

		static	OV<SError>						revealInFinder(const CFolder& folder);
		static	OV<SError>						revealInFinder(const TArray<CFile>& files);
#endif
#if defined(TARGET_OS_WINDOWS)
	#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		static	OV<SError>						revealInFileExplorer(const TArray<CFile>& files);
	#endif
#endif

	// Properties
	public:
		static	const	CString	mErrorDomain;

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		static			SError	mSecurityScopedResourceAccessCouldNotResolveStorageDataError;
		static			SError	mSecurityScopedResourceAccessCouldNotComposeStorageDataError;
		static			SError	mSecurityScopedResourceAccessFailedToStartError;
#endif
};
