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
				CErrorRegistry::registerError(kFileDoesNotExistError, CString(OSSTR("File Does Not Exist")));
				CErrorRegistry::registerError(kFileIsOpenError, CString(OSSTR("File Is Open")));
				CErrorRegistry::registerError(kFileNotOpenError, CString(OSSTR("Is Not Open")));
				CErrorRegistry::registerError(kFileNotFoundError, CString(OSSTR("Is Not Found")));
				CErrorRegistry::registerError(kFileUnableToRevealInFinderError,
						CString(OSSTR("Unable to reveal in Finder")));
				CErrorRegistry::registerError(kFileUnableToReadError, CString(OSSTR("Unable to read")));
				CErrorRegistry::registerError(kFileUnableToWriteError, CString(OSSTR("Unable to write")));
			}
};

static	CFileSetup	sFileSetup;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileInternals

class CFileInternals : public TCopyOnWriteReferenceCountable<CFileInternals> {
	public:
		CFileInternals(const CFilesystemPath& filesystemPath) :
			TCopyOnWriteReferenceCountable(),
					mFilesystemPath(filesystemPath)
			{}
		CFileInternals(const CFileInternals& other) :
			TCopyOnWriteReferenceCountable(),
					mFilesystemPath(other.mFilesystemPath)
			{}

		CFilesystemPath	mFilesystemPath;
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
	// Check if assignment to self
	if (this == &other)
		return *this;

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
