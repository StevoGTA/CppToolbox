//----------------------------------------------------------------------------------------------------------------------
//	CFile.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileSetup

class CFileSetup {
	public:
		CFileSetup()
			{
				CErrorRegistry::registerError(kFileDoesNotExistError, CString("File Does Not Exist"));
				CErrorRegistry::registerError(kFileIsOpenError, CString("File Is Open"));
				CErrorRegistry::registerError(kFileNotOpenError, CString("Is Not Open"));
				CErrorRegistry::registerError(kFileNotFoundError, CString("Is Not Found"));
				CErrorRegistry::registerError(kFileUnableToRevealInFinderError, CString("Unable to reveal in Finder"));
				CErrorRegistry::registerError(kFileUnableToReadError, CString("Unable to read"));
				CErrorRegistry::registerError(kFileUnableToWriteError, CString("Unable to write"));
			}
};

static	CFileSetup	sFileSetup;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileInternals

class CFileInternals {
	public:
						CFileInternals(const CFilesystemPath& filesystemPath) :
							mFilesystemPath(filesystemPath), mReferenceCount(1)
							{}
						~CFileInternals() {}

		CFileInternals*	addReference()
							{ mReferenceCount++; return this; }
		void			removeReference()
							{
								// Remove reference and see if we are the last one
								if (--mReferenceCount == 0) {
									// Last one
									CFileInternals*	THIS = this;
									DisposeOf(THIS);
								}
							}
		CFileInternals*	prepareForWrite()
							{
								// Check reference count.  If there is more than 1 reference, we
								//	implement a "copy on write".  So we will clone ourselves so we
								//	have a personal buffer that can be changed while leaving the
								//	exiting buffer as-is for the other references.
								if (mReferenceCount > 1) {
									// Multiple references, copy
									CFileInternals*	fileInternals = new CFileInternals(mFilesystemPath);

									// One less reference
									mReferenceCount--;

									return fileInternals;
								} else
									// Only a single reference
									return this;
							}

		CFilesystemPath	mFilesystemPath;
		UInt32			mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFile::CFile(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFileInternals(filesystemPath);
}

//----------------------------------------------------------------------------------------------------------------------
CFile::CFile(const CFile& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFile::~CFile()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CFilesystemPath& CFile::getFilesystemPath() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath;
}

//----------------------------------------------------------------------------------------------------------------------
CString CFile::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath.getLastComponent();
}

//----------------------------------------------------------------------------------------------------------------------
CString CFile::getNameDeletingExtension() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath.getLastComponentDeletingExtension();
}

//----------------------------------------------------------------------------------------------------------------------
CFolder CFile::getFolder() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFolder(mInternals->mFilesystemPath.deletingLastComponent());
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::equals(const CFile& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath == other.mInternals->mFilesystemPath;
}

//----------------------------------------------------------------------------------------------------------------------
CFile& CFile::operator=(const CFile& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
void CFile::update(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Update
	mInternals->mFilesystemPath = filesystemPath;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
ECompareResult CFile::compareName(CFile* const file1, CFile* const file2, void* context)
//----------------------------------------------------------------------------------------------------------------------
{
	return file1->getName().compareTo(file2->getName());
}
