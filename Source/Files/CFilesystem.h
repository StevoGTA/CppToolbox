//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAppleResourceManager.h"
#include "SFoldersFiles.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFilesystem

class CFilesystem {
	// Types
	public:
		// Application Object (representing an Application on the target system)
#if defined(TARGET_OS_MACOS)
		typedef	CFolder	Application;
#endif

	// Methods
	public:
											// Class methods
		static	TVResult<SFoldersFiles>		getFoldersFiles(const CFolder& folder, bool deep = false);
		static	TVResult<TArray<CFolder> >	getFolders(const CFolder& folder, bool deep = false);
		static	TVResult<TArray<CFile> >	getFiles(const CFolder& folder, bool deep = false);
		static	OV<CFile>					getDotUnderscoreFile(const CFile& file);
		static	OV<CFile>					getResourceFork(const CFile& file);
		static	OI<CAppleResourceManager>	getAppleResourceManager(const CFile& file);

											// Will copy sourceFolder *into* destinationFolder
		static	OV<SError>					copy(const CFolder& sourceFolder, const CFolder& destinationFolder);

											// Will copy file *into* destinationFolder
		static	OV<SError>					copy(const CFile& file, const CFolder& destinationFolder);

											// Will copy files *into* destinationFolder
		static	OV<SError>					copy(const TArray<CFile> files, const CFolder& destinationFolder);

											// Will replace destinationFile with sourceFile and remove sourceFile
		static	OV<SError>					replace(const CFile& sourceFile, const CFile& destinationFile);

#if defined(TARGET_OS_MACOS)
		static	OV<SError>					open(const TArray<CFile> files, const Application& application);
		static	void						moveToTrash(const TArray<CFile> files, TMArray<CFile>& outUntrashedFiles);
		static	OV<SError>					moveToTrash(const TArray<CFile> files);

		static	OV<SError>					revealInFinder(const CFolder& folder);
		static	OV<SError>					revealInFinder(const TArray<CFile> files);
#endif
};
