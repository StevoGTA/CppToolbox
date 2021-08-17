//----------------------------------------------------------------------------------------------------------------------
//	CFile.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFolder.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFile

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
												{ getFilesystemPath().hashInto(hasher); }

											// Instance methods
				const	CFilesystemPath&	getFilesystemPath() const;

						CString				getName() const;
						CString				getNameDeletingExtension() const;
						CString				getNameForDisplay() const;
						OI<SError>			rename(const CString& string);

						UInt64				getSize() const;

						OI<SError>			remove() const;
						bool				doesExist() const;

						CFolder				getFolder() const;
						bool				isHidden() const;

						bool				getLocked() const;
						OI<SError>			setLocked(bool lockFile) const;

						UniversalTime		getCreationDate() const;
						UniversalTime		getModificationDate() const;

						bool				equals(const CFile& other) const;

						void				logAsMessage(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logMessage(
															prefix + CString(OSSTR("File: ")) +
																	getFilesystemPath().getString());
												}
						void				logAsWarning(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logWarning(
															prefix + CString(OSSTR("File: ")) +
																	getFilesystemPath().getString());
												}
						void				logAsError(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logError(
															prefix + CString(OSSTR("File: ")) +
																	getFilesystemPath().getString());
												}

						CFile&				operator=(const CFile& other);

#if TARGET_OS_MACOS || TARGET_OS_LINUX
						UInt16				getPermissions() const;
						OI<SError>			setPermissions(UInt16 permissions) const;
#endif

#if TARGET_OS_MACOS
						bool				isAlias() const;

						CString				getComment() const;
						OI<SError>			setComment(const CString& string) const;
#endif

	private:
											// Instance methods
						void				update(const CFilesystemPath& filesystemPath);

	// Properties
	public:
		static	SError			mDoesNotExistError;
		static	SError			mIsOpenError;
		static	SError			mNotOpenError;
		static	SError			mNotFoundError;
		static	SError			mUnableToRevealInFinderError;
		static	SError			mUnableToReadError;
		static	SError			mUnableToWriteError;

	private:
				CFileInternals*	mInternals;
};
