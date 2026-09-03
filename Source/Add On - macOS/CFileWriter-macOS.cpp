//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter-macOS.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

#include "CFilesystem.h"
#include "SError-POSIX.h"

#include <cerrno>
#include <sys/xattr.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileWriter

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::write(const CFile& file, const CFile::AppleMetadata& appleMetadata)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString::C	path = file.getFilesystemPath().getString().getUTF8String();

	// Remove any existing resource fork.  The fork cannot be unlinked - the named fork path is not a directory entry -
	//	so it is removed as the extended attribute it actually is.
	if ((::removexattr(*path, XATTR_RESOURCEFORK_NAME, 0) != 0) && (errno != ENOATTR))
		// Error
		return OV<SError>(SErrorFromPOSIXerror(errno));

	// Write the resource fork data, if any
	if (appleMetadata.getResourceForkData().hasValue()) {
		// Write the fork
		OV<SError>	error = write(CFilesystem::getResourceForkFile(file), *appleMetadata.getResourceForkData());
		ReturnErrorIfError(error);
	}

	// Read any existing Finder Info
	CFile::FinderInfo	finderInfo;
	bool				haveExisting =
								::getxattr(*path, XATTR_FINDERINFO_NAME, &finderInfo, sizeof(finderInfo), 0, 0) ==
										(ssize_t) sizeof(finderInfo);

	// Write the Finder Info, if any
	if (appleMetadata.getIdentity().hasValue()) {
		// Write our type and creator
		const	CFile::AppleMetadata::Identity&	identity = *appleMetadata.getIdentity();
		finderInfo.setFileType(identity.getFileType());
		finderInfo.setFileCreator(identity.getFileCreator().getValue(0));

		if (::setxattr(*path, XATTR_FINDERINFO_NAME, &finderInfo, sizeof(finderInfo), 0, 0) != 0)
			// Error
			return OV<SError>(SErrorFromPOSIXerror(errno));
	} else if (haveExisting) {
		// Clear our type and creator, then write the block back
		finderInfo.setFileType(0);
		finderInfo.setFileCreator(0);

		if (::setxattr(*path, XATTR_FINDERINFO_NAME, &finderInfo, sizeof(finderInfo), 0, 0) != 0)
			// Error
			return OV<SError>(SErrorFromPOSIXerror(errno));
	}

	return OV<SError>();
}
