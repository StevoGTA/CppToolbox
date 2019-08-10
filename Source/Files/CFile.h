//----------------------------------------------------------------------------------------------------------------------
//	CFile.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFolder.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Errors

const	UErrorDomain	kFileErrorDomain	= MAKE_OSTYPE('F', 'I', 'L', 'E');

const	UError	kFileDoesNotExistError				= MAKE_UError(kFileErrorDomain, 1);
const	UError	kFileIsOpenError					= MAKE_UError(kFileErrorDomain, 2);
const	UError	kFileNotOpenError					= MAKE_UError(kFileErrorDomain, 3);
const	UError	kFileNotFoundError					= MAKE_UError(kFileErrorDomain, 4);
const	UError	kFileUnableToRevealInFinderError	= MAKE_UError(kFileErrorDomain, 5);
const	UError	kFileUnableToReadError				= MAKE_UError(kFileErrorDomain, 6);
const	UError	kFileUnableToWriteError				= MAKE_UError(kFileErrorDomain, 7);

#if TARGET_OS_MACOS
const	UError	kFileEOFError						= MAKE_UErrorFromOSStatus(eofErr);
#else
const	UError	kFileEOFError						= MAKE_UError(kFileErrorDomain, 8);
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
