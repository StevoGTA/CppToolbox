//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Application Object (representing an Application on the target system)

#if TARGET_OS_MACOS || TARGET_OS_IOS
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
		static	UError	getFolders(const CFolder& folder, TArray<CFolder>& outFolders);
		static	UError	getFiles(const CFolder& folder, TArray<CFile>& outFiles);
		static	UError	getFoldersFiles(const CFolder& folder, TArray<CFolder>& outFolders, TArray<CFile>& outFiles);

						/*
							Will copy sourceFolder *into* destinationFolder
						*/
		static	UError	copy(const CFolder& sourceFolder, const CFolder& destinationFolder);
						/*
							Will copy file *into* destinationFolder
						*/
		static	UError	copy(const CFile& file, const CFolder& destinationFolder);
						/*
							Will copy files *into* destinationFolder
						*/
		static	UError	copy(const TArray<CFile> files, const CFolder& destinationFolder);
						/*
							Will replace destinationFile with sourceFile and remove sourceFile
						*/
		static	UError	replace(const CFile& sourceFile, const CFile& destinationFile);

#if TARGET_OS_MACOS
		static	UError	open(const TArray<CFile> files, const CApplicationObject& applicationObject);
		static	void	moveToTrash(const TArray<CFile> files, TArray<CFile>& outUntrashedFiles);
		static	UError	moveToTrash(const TArray<CFile> files);

		static	UError	revealInFinder(const CFolder& folder);
		static	UError	revealInFinder(const TArray<CFile> files);
#endif
};
