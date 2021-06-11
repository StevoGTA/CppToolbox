//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SFoldersFiles.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFilesystem

class CFilesystem {
	// Types
	public:
		// Application Object (representing an Application on the target system)
#if TARGET_OS_MACOS
		typedef	CFolder	Application;
#endif

	public:
		struct GetFoldersFilesResult {

										// Methods
										GetFoldersFilesResult(const SFoldersFiles& foldersFiles) :
											mFoldersFiles(OI<SFoldersFiles>(foldersFiles))
											{}
										GetFoldersFilesResult(const SError& error) :
											mError(OI<SError>(error))
											{}

			const	OI<SFoldersFiles>&	getFoldersFiles() const
											{ return mFoldersFiles; }
			const	OI<SError>&			getError() const
											{ return mError; }

			private:
				OI<SFoldersFiles>	mFoldersFiles;
				OI<SError>			mError;
		};

		struct GetFoldersResult {

											// Methods
											GetFoldersResult(const TArray<CFolder>& folders) :
												mFolders(OI<TArray<CFolder> >(folders))
												{}
											GetFoldersResult(const SError& error) :
												mError(OI<SError>(error))
												{}

			const	OI<TArray<CFolder> >&	getFolders() const
												{ return mFolders; }
			const	OI<SError>&				getError() const
												{ return mError; }

			private:
				OI<TArray<CFolder> >	mFolders;
				OI<SError>				mError;
		};

		struct GetFilesResult {

										// Methods
										GetFilesResult(const TArray<CFile>& files) :
											mFiles(OI<TArray<CFile> >(files))
											{}
										GetFilesResult(const SError& error) :
											mError(OI<SError>(error))
											{}

			const	OI<TArray<CFile> >&	getFiles() const
											{ return mFiles; }
			const	OI<SError>&			getError() const
											{ return mError; }

			private:
				OI<TArray<CFile> >	mFiles;
				OI<SError>			mError;
		};

	// Methods
	public:
										// Class methods
		static	GetFoldersFilesResult	getFoldersFiles(const CFolder& folder, bool deep = false);
		static	GetFoldersResult		getFolders(const CFolder& folder, bool deep = false);
		static	GetFilesResult			getFiles(const CFolder& folder, bool deep = false);

											//	Will copy sourceFolder *into* destinationFolder
		static	OI<SError>				copy(const CFolder& sourceFolder, const CFolder& destinationFolder);

											//	Will copy file *into* destinationFolder
		static	OI<SError>				copy(const CFile& file, const CFolder& destinationFolder);

											//	Will copy files *into* destinationFolder
		static	OI<SError>				copy(const TArray<CFile> files, const CFolder& destinationFolder);

											//	Will replace destinationFile with sourceFile and remove sourceFile
		static	OI<SError>				replace(const CFile& sourceFile, const CFile& destinationFile);

#if TARGET_OS_MACOS
		static	OI<SError>				open(const TArray<CFile> files, const Application& application);
		static	void					moveToTrash(const TArray<CFile> files, TArray<CFile>& outUntrashedFiles);
		static	OI<SError>				moveToTrash(const TArray<CFile> files);

		static	OI<SError>				revealInFinder(const CFolder& folder);
		static	OI<SError>				revealInFinder(const TArray<CFile> files);
#endif
};
