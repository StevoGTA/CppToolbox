//----------------------------------------------------------------------------------------------------------------------
//	CFile.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFolder.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFile

class CFile : public CHashable {
	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											CFile(const CFilesystemPath& filesystemPath);
											CFile(const CFile& other);
											~CFile();

											// CEquatable methods
						bool				operator==(const CEquatable& other) const
												{ return equals((const CFile&) other); }

											// CHashable methods
						void				hashInto(CHasher& hasher) const
												{ getFilesystemPath().hashInto(hasher); }

											// Instance methods
				const	CFilesystemPath&	getFilesystemPath() const;

						CString				getName() const;
						CString				getNameDeletingExtension() const;
						CString				getNameForDisplay() const;
						OV<SError>			rename(const CString& string);

						UInt64				getByteCount() const;

						OV<SError>			remove() const;
						bool				doesExist() const;

						CFolder				getFolder() const;
						bool				isHidden() const;

						bool				getLocked() const;
						OV<SError>			setLocked(bool lockFile) const;

						UniversalTime		getCreationUniversalTime() const;
						UniversalTime		getModificationUniversalTime() const;

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

#if defined(TARGET_OS_MACOS) || defined(TARGET_OS_LINUX)
						UInt16				getPermissions() const;
						OV<SError>			setPermissions(UInt16 permissions) const;
#endif

#if defined(TARGET_OS_MACOS)
						bool				isAlias() const;

						OV<CString>			getComments() const;
						OV<SError>			setComments(const CString& string) const;
#endif

		static			CString				getFilesystemPathsForDisplay(const TArray<CFile>& files,
													CFilesystemPath::Style filesystemPathStyle =
															CFilesystemPath::kStylePlatformDefault);

	private:
											// Instance methods
						void				update(const CFilesystemPath& filesystemPath);

	// Properties
	public:
		static	const	SError		mDoesNotExistError;
		static	const	SError		mIsOpenError;
		static	const	SError		mNotOpenError;
		static	const	SError		mNotFoundError;
		static	const	SError		mUnableToRevealInFinderError;
		static	const	SError		mUnableToReadError;
		static	const	SError		mUnableToWriteError;

	private:
						Internals*	mInternals;
};
