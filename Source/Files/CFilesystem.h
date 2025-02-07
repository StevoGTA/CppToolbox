//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "SFoldersFiles.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFilesystem

class CFilesystem {
	// Types
	public:
		// Application Object (representing an Application on the target system)
		typedef	CFolder	MacOSApplication;

#if defined(TARGET_OS_MACOS)
		typedef	MacOSApplication	Application;
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

#if defined(TARGET_OS_MACOS)
		static	OV<SError>						open(const TArray<CFile>& files, const Application& application);
		static	void							moveToTrash(const TArray<CFile>& files,
														TMArray<CFile>& outUntrashedFiles);
		static	OV<SError>						moveToTrash(const TArray<CFile>& files);

		static	OV<SError>						revealInFinder(const CFolder& folder);
		static	OV<SError>						revealInFinder(const TArray<CFile>& files);
#endif
#if defined(TARGET_OS_WINDOWS)
	#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		static	OV<SError>						revealInFileExplorer(const TArray<CFile>& files);
	#endif
#endif
};
