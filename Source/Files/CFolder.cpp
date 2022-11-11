//----------------------------------------------------------------------------------------------------------------------
//	CFolder.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CFolder"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFolderInternals

class CFolderInternals : public TCopyOnWriteReferenceCountable<CFolderInternals> {
	public:
		CFolderInternals(const CFilesystemPath& filesystemPath) :
			TCopyOnWriteReferenceCountable(),
					mFilesystemPath(filesystemPath)
			{}
		CFolderInternals(const CFolderInternals& other) :
			TCopyOnWriteReferenceCountable(),
					mFilesystemPath(other.mFilesystemPath)
			{}

		CFilesystemPath	mFilesystemPath;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFolder

// MARK: Properties

const	SError CFolder::mDoesNotExistError(sErrorDomain, 1, CString(OSSTR("Folder Does Not Exist")));
const	SError CFolder::mAlreadyExistsError(sErrorDomain, 2, CString(OSSTR("Folder Already Exists")));

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
	return *mInternals->mFilesystemPath.getLastComponent();
}

//----------------------------------------------------------------------------------------------------------------------
CFolder CFolder::getParentFolder() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFolder(mInternals->mFilesystemPath.deletingLastComponent());
}

//----------------------------------------------------------------------------------------------------------------------
CFolder CFolder::getChildFolder(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFolder(mInternals->mFilesystemPath.appendingComponent(name));
}

//----------------------------------------------------------------------------------------------------------------------
CFile CFolder::getFile(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFile(mInternals->mFilesystemPath.appendingComponent(name));
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
void CFolder::update(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Update
	mInternals->mFilesystemPath = filesystemPath;
}
