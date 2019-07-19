//----------------------------------------------------------------------------------------------------------------------
//	CFile.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFolder.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Error Codes

const	UErrorDomain	kCFileErrorDomain	= MAKE_OSTYPE('F', 'I', 'L', 'E');

const	UError	kCFileDoesNotExistError				= MAKE_UError(kCFileErrorDomain, 1);
const	UError	kCFileIsOpenError					= MAKE_UError(kCFileErrorDomain, 2);
const	UError	kCFileNotOpenError					= MAKE_UError(kCFileErrorDomain, 3);
const	UError	kCFileNotFoundError					= MAKE_UError(kCFileErrorDomain, 4);
const	UError	kCFileUnableToRevealInFinderError	= MAKE_UError(kCFileErrorDomain, 5);
const	UError	kCFileUnableToReadError				= MAKE_UError(kCFileErrorDomain, 6);
const	UError	kCFileUnableToWriteError			= MAKE_UError(kCFileErrorDomain, 7);

#if TARGET_OS_MACOS
const	UError	kCFileEOFError						= MAKE_UErrorFromOSStatus(eofErr);
#else
const	UError	kCFileEOFError						= MAKE_UError(kCFileErrorDomain, 8);
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

class CFileInternals;
class CFile : CHashable {
	// Methods
	public:
											// Lifecycle methods
											CFile(const CFilesystemPath& filesystemPath);
											CFile(const CFile& other);
											~CFile();

											// CEquatable methods
				bool						operator==(const CEquatable& other) const
												{ return equals((const CFile&) other); }

											// CHashable methods
				void						hashInto(CHasher& hasher) const
												{ hasher.add(getFilesystemPath().getString().getCString()); }

											// Instance methods
				const	CFilesystemPath&	getFilesystemPath() const;

						CString				getName() const;
						CString				getNameDeletingExtension() const;
						UError				rename(const CString& string);

						UInt64				getSize() const;

						UError				remove() const;
						bool				doesExist() const;

						CFolder				getFolder() const;
//						CImage				getImage() const;
						bool				isHidden() const;

						bool				getLocked() const;
						UError				setLocked(bool lockFile) const;

						UniversalTime		getCreationDate() const;
						UniversalTime		getModificationDate() const;

						bool				equals(const CFile& other) const;

						void				logAsMessage(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logMessage(
															prefix + CString("File: ") +
																	getFilesystemPath().getString());
												}
						void				logAsWarning(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logWarning(
															prefix + CString("File: ") +
																	getFilesystemPath().getString());
												}
						void				logAsError(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logError(
															prefix + CString("File: ") +
																	getFilesystemPath().getString());
												}

						CFile&				operator=(const CFile& other);

#if TARGET_OS_MACOS || TARGET_OS_LINUX
						UInt16				getPermissions() const;
						UError				setPermissions(UInt16 permissions) const;
#endif

#if TARGET_OS_MACOS
						bool				isAlias() const;

						CString				getComment() const;
						UError				setComment(const CString& string) const;
#endif
											// Class methods
		static			ECompareResult		compareName(CFile* const file1, CFile* const file2, void* context);

	private:
											// Instance methods
						void				update(const CFilesystemPath& filesystemPath);

	// Properties
	private:
		CFileInternals*	mInternals;
};
