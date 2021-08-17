//----------------------------------------------------------------------------------------------------------------------
//	CFile.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CFile"));

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

// MARK: Properties

SError	CFile::mDoesNotExistError(sErrorDomain, 1, CString(OSSTR("Does Not Exist")));
SError	CFile::mIsOpenError(sErrorDomain, 2, CString(OSSTR("Is Open")));
SError	CFile::mNotOpenError(sErrorDomain, 3, CString(OSSTR("Is Not Open")));
SError	CFile::mNotFoundError(sErrorDomain, 4, CString(OSSTR("Is Not Found")));
SError	CFile::mUnableToRevealInFinderError(sErrorDomain, 5, CString(OSSTR("Unable to reveal in Finder")));
SError	CFile::mUnableToReadError(sErrorDomain, 6, CString(OSSTR("Unable to read")));
SError	CFile::mUnableToWriteError(sErrorDomain, 7, CString(OSSTR("Unable to write")));

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
CString CFile::getNameForDisplay() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath.getLastComponentForDisplay();
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
