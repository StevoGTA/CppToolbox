//----------------------------------------------------------------------------------------------------------------------
//	CFolder.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFilesystemPath.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFolder

class CFile;

class CFolder : public CHashable {
	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											CFolder(const CFilesystemPath& filesystemPath);
											CFolder(const CFolder& other);
											~CFolder();

											// CEquatable methods
								bool		operator==(const CEquatable& other) const
												{ return equals((const CFolder&) other); }

											// CHashable methods
								void		hashInto(CHasher& hasher) const
												{ getFilesystemPath().hashInto(hasher); }

											// Instance methods
				const	CFilesystemPath&	getFilesystemPath() const;

						CString				getName() const;
						OV<SError>			rename(const CString& string);

						OV<SError>			create(bool createIntermediateFolders = false) const;
						OV<SError>			remove() const;
						bool				doesExist() const;

						CFolder				getParentFolder() const;

						CFolder				getChildFolder(const CString& name) const;
						CFile				getFile(const CString& name) const;

#if defined(TARGET_OS_MACOS)
						bool				isPackage() const;
#endif

						bool				equals(const CFolder& other) const;

						void				logAsMessage(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logMessage(
															prefix + CString(OSSTR("Folder: ")) +
																	getFilesystemPath().getString());
												}
						void				logAsWarning(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logWarning(
															prefix + CString(OSSTR("Folder: ")) +
																	getFilesystemPath().getString());
												}
						void				logAsError(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logError(
															prefix + CString(OSSTR("Folder: ")) +
																	getFilesystemPath().getString());
												}

						CFolder&			operator=(const CFolder& other);

											// Class methods
		static	const	CFolder&			temp();

#if defined(TARGET_OS_MACOS)
		static	const	CFolder&			systemApplicationSupport();
		static	const	CFolder&			systemAudioPlugins();
		static	const	CFolder&			systemAudioPresets();
		static	const	CFolder&			systemFrameworks();
		static	const	CFolder&			systemLibrary();
		static	const	CFolder&			userApplicationSupport();
		static	const	CFolder&			userAudioPlugins();
		static	const	CFolder&			userAudioPresets();
		static	const	CFolder&			userDesktop();
		static	const	CFolder&			userHome();
		static	const	CFolder&			userLibrary();
		static	const	CFolder&			userLogs();
		static	const	CFolder&			userMusic();
#elif defined(TARGET_OS_WINDOWS)
		static	const	CFolder&			localApplicationData();
#endif

	private:
											// Instance methods
						void				update(const CFilesystemPath& filesystemPath);

	// Properties
	public:
		static	const	SError		mDoesNotExistError;
		static	const	SError		mAlreadyExistsError;

	private:
						Internals*	mInternals;
};
