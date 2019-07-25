//----------------------------------------------------------------------------------------------------------------------
//	CFolder.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFolderSetup

class CFolderSetup {
	public:
		CFolderSetup()
			{
				CErrorRegistry::registerError(kFolderDoesNotExistError, CString("Folder Does Not Exist"));
				CErrorRegistry::registerError(kFolderAlreadyExistsError, CString("Folder Already Exists"));
			}
};

static	CFolderSetup	sFolderSetup;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFolderInternals

class CFolderInternals {
	public:
							CFolderInternals(const CFilesystemPath& filesystemPath) :
								mFilesystemPath(filesystemPath), mReferenceCount(1)
								{}
							~CFolderInternals() {}

		CFolderInternals*	addReference()
								{ mReferenceCount++; return this; }
		void				removeReference()
								{
									// Remove reference and see if we are the last one
									if (--mReferenceCount == 0) {
										// Last one
										CFolderInternals*	THIS = this;
										DisposeOf(THIS);
									}
								}
		CFolderInternals*	prepareForWrite()
								{
									// Check reference count.  If there is more than 1 reference, we
									//	implement a "copy on write".  So we will clone ourselves so we
									//	have a personal buffer that can be changed while leaving the
									//	exiting buffer as-is for the other references.
									if (mReferenceCount > 1) {
										// Multiple references, copy
										CFolderInternals*	folderInternals = new CFolderInternals(mFilesystemPath);

										// One less reference
										mReferenceCount--;

										return folderInternals;
									} else
										// Only a single reference
										return this;
								}

		CFilesystemPath	mFilesystemPath;
		UInt32			mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFolder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFolder::CFolder(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFolderInternals(filesystemPath);
}

//----------------------------------------------------------------------------------------------------------------------
CFolder::CFolder(const CFolder& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFolder::~CFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CFilesystemPath& CFolder::getFilesystemPath() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath;
}

//----------------------------------------------------------------------------------------------------------------------
CString CFolder::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath.getLastComponent();
}

//----------------------------------------------------------------------------------------------------------------------
CFolder CFolder::getFolder() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFolder(mInternals->mFilesystemPath.deletingLastComponent());
}

//----------------------------------------------------------------------------------------------------------------------
bool CFolder::equals(const CFolder& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath == other.mInternals->mFilesystemPath;
}

//----------------------------------------------------------------------------------------------------------------------
CFolder& CFolder::operator=(const CFolder& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
void CFolder::update(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Update
	mInternals->mFilesystemPath = filesystemPath;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
ECompareResult CFolder::compareName(CFolder* const folder1, CFolder* const folder2, void* context)
//----------------------------------------------------------------------------------------------------------------------
{
	return folder1->getName().compareTo(folder2->getName());
}
