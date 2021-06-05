//----------------------------------------------------------------------------------------------------------------------
//	SFoldersFiles.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#import "CFolder.h"
#import "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SFoldersFiles

struct SFoldersFiles {

								// Lifecycle methods
								SFoldersFiles(const TArray<CFolder>& folders, const TArray<CFile>& files) :
									mFolders(folders), mFiles(files)
									{}
								SFoldersFiles(const SFoldersFiles& other) :
									mFolders(other.mFolders), mFiles(other.mFiles)
									{}

								// Instance methods
	const	TArray<CFolder>&	getFolders() const
									{ return mFolders; }
	const	TArray<CFile>&		getFiles() const
									{ return mFiles; }

	// Properties
	private:
		TNArray<CFolder>	mFolders;
		TNArray<CFile>		mFiles;
};
