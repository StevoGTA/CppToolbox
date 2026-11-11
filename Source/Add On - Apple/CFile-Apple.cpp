//----------------------------------------------------------------------------------------------------------------------
//	CFile-Apple.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "CLogServices.h"
#include "SError-POSIX.h"

#include <sys/stat.h>
#include <sys/xattr.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReportErrorAndReturnError(error, message)															\
				{																								\
					CLogServices::logError(error, message,														\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),						\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);				\
					CLogServices::logError(																		\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + getFilesystemPath().getString());	\
																												\
					return OV<SError>(error);																	\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CFile::getLocked() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	struct	stat	statInfo;

	return (::stat(*getFilesystemPath().getString().getUTF8String(), &statInfo) == 0) &&
			((statInfo.st_flags & UF_IMMUTABLE) != 0);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setLocked(bool lockFile) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get flags
	struct	stat	statInfo;
	if (::stat(*getFilesystemPath().getString().getUTF8String(), &statInfo) != 0)
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno),
				CString(OSSTR("getting flags when setting locked")));

	// Update flags
	statInfo.st_flags = lockFile ? (statInfo.st_flags | UF_IMMUTABLE) : (statInfo.st_flags & ~UF_IMMUTABLE);
	if (::chflags(*getFilesystemPath().getString().getUTF8String(), statInfo.st_flags) != 0)
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("setting locked")));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CFile::getExtendedAttributeAsData(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString::C	path = getFilesystemPath().getString().getUTF8String();
	CString::C	nameC = name.getUTF8String();

	// Get size
	ssize_t	byteCount = ::getxattr(*path, *nameC, nil, 0, 0, 0);
	if (byteCount < 0)
		// Not present or can't be read
		return OV<CData>();

	// Read
	CData	data((CData::ByteCount) byteCount, 0);

	return (::getxattr(*path, *nameC, *data.getMutableBuffer(0, byteCount), byteCount, 0, 0) == byteCount) ?
			OV<CData>(data) : OV<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setExtendedAttribute(const CString& name, const CData& data) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Write
	if (::setxattr(*getFilesystemPath().getString().getUTF8String(), *name.getUTF8String(), *data.getUInt8Buffer(),
			data.getByteCount(), 0, 0) != 0)
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("setting extended attribute")));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::removeExtendedAttribute(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	if ((::removexattr(*getFilesystemPath().getString().getUTF8String(), *name.getUTF8String(), 0) != 0) &&
			(errno != ENOATTR))
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("removing extended attribute")));

	return OV<SError>();
}
