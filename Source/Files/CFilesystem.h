//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Application Object (representing an Application on the target system)

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
typedef	CFolder	CApplicationObject;
#endif

#if TARGET_OS_LINUX || TARGET_OS_WINDOWS
typedef	CFile	CApplicationObject;
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

class CFilesystem {
	// Methods
	public:
							// Class methods
		static	OI<SError>	getFolders(const CFolder& folder, TArray<CFolder>& outFolders);
		static	OI<SError>	getFiles(const CFolder& folder, TArray<CFile>& outFiles);
		static	OI<SError>	getFoldersFiles(const CFolder& folder, TArray<CFolder>& outFolders,
									TArray<CFile>& outFiles);
								/*
									Will copy sourceFolder *into* destinationFolder
								*/
		static	OI<SError>	copy(const CFolder& sourceFolder, const CFolder& destinationFolder);
								/*
									Will copy file *into* destinationFolder
								*/
		static	OI<SError>	copy(const CFile& file, const CFolder& destinationFolder);
								/*
									Will copy files *into* destinationFolder
								*/
		static	OI<SError>	copy(const TArray<CFile> files, const CFolder& destinationFolder);
								/*
									Will replace destinationFile with sourceFile and remove sourceFile
								*/
		static	OI<SError>	replace(const CFile& sourceFile, const CFile& destinationFile);

#if TARGET_OS_MACOS
		static	OI<SError>	open(const TArray<CFile> files, const CApplicationObject& applicationObject);
		static	void		moveToTrash(const TArray<CFile> files, TArray<CFile>& outUntrashedFiles);
		static	OI<SError>	moveToTrash(const TArray<CFile> files);

		static	OI<SError>	revealInFinder(const CFolder& folder);
		static	OI<SError>	revealInFinder(const TArray<CFile> files);
#endif
};
