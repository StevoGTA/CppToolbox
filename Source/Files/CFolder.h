//----------------------------------------------------------------------------------------------------------------------
//	CFolder.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFilesystemPath.h"
#include "CLogServices.h"
#include "CppToolboxError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Errors

const	UErrorDomain	kFolderErrorDomain	= MAKE_OSTYPE('F', 'o', 'l', 'd');

const	UError	kFolderDoesNotExistError	= MAKE_UError(kFolderErrorDomain, 1);
const	UError	kFolderAlreadyExistsError	= MAKE_UError(kFolderErrorDomain, 2);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFolder

class CFolderInternals;
class CFolder : CHashable {
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
												{ hasher.add(getFilesystemPath().getString().getCString()); }

											// Instance methods
				const	CFilesystemPath&	getFilesystemPath() const;

						CString				getName() const;
						UError				rename(const CString& string);

						UError				create() const;
						UError				remove() const;
						bool				doesExist() const;

						CFolder				getFolder() const;

#if TARGET_OS_MACOS
						bool				isPackage() const;
#endif

						bool				equals(const CFolder& other) const;

						void				logAsMessage(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logMessage(
															prefix + CString("Folder: ") +
																	getFilesystemPath().getString());
												}
						void				logAsWarning(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logWarning(
															prefix + CString("Folder: ") +
																	getFilesystemPath().getString());
												}
						void				logAsError(const CString& prefix = CString::mEmpty) const
												{
													// Log
													CLogServices::logError(
															prefix + CString("Folder: ") +
																	getFilesystemPath().getString());
												}

						CFolder&			operator=(const CFolder& other);

											// Class methods
		static	const	CFolder&			tempFolder();

		static			ECompareResult		compareName(CFolder* const folder1, CFolder* const folder2, void* context);

	private:
											// Instance methods
						void				update(const CFilesystemPath& filesystemPath);

	// Properties
	public:
#if TARGET_OS_MACOS
		static	const	CFolder&			systemApplicationSupportFolder();
		static	const	CFolder&			systemAudioPluginsFolder();
		static	const	CFolder&			systemAudioPresetsFolder();
		static	const	CFolder&			systemFrameworksFolder();
		static	const	CFolder&			systemLibraryFolder();
		static	const	CFolder&			userApplicationSupportFolder();
		static	const	CFolder&			userAudioPluginsFolder();
		static	const	CFolder&			userAudioPresetsFolder();
		static	const	CFolder&			userDesktopFolder();
		static	const	CFolder&			userHomeFolder();
		static	const	CFolder&			userLibraryFolder();
		static	const	CFolder&			userMusicFolder();
#endif

	private:
						CFolderInternals*	mInternals;
};
