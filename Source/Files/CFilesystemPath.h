//----------------------------------------------------------------------------------------------------------------------
//	CFilesystemPath.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Path style

enum EFilesystemPathStyle {
	// Paths in the style "Volume/Folder/Folder/"
	kFilesystemPathStylePOSIX,

	// Paths in the style "Volume\Folder\Folder\"
	kFilesystemPathStyleWindows,

#if TARGET_OS_MACOS
	// Deprecated paths in the style "Volume:Folder:Folder:"
	kFilesystemPathStyleHFS,
#endif

#if TARGET_OS_MACOS || TARGET_OS_LINUX || TARGET_OS_IOS
	kFilesystemPathStylePlatformDefault = kFilesystemPathStylePOSIX,
#endif

#if TARGET_OS_WINDOWS
	kFilesystemPathStylePlatformDefault = kFilesystemPathStyleWindows,
#endif
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFilesystemPath

class CFilesystemPathInternals;
class CFilesystemPath : public CHashable {
	// Methods
	public:
						// Lifecycle methods
						CFilesystemPath(const CString& string,
								EFilesystemPathStyle pathStyle = kFilesystemPathStylePlatformDefault);
						CFilesystemPath(const CFilesystemPath& other);
						~CFilesystemPath();

						// CEquatable methods
		bool			operator==(const CEquatable& other) const
							{ return equals((const CFilesystemPath&) other); }

						// CHashable methods
		void			hashInto(CHasher& hasher) const
							{ hasher.add(getString().getCString()); }

						// Instance methods
		CString			getString(EFilesystemPathStyle pathStyle = kFilesystemPathStylePlatformDefault) const;

		CString			getExtension() const;

		TArray<CString>	getComponents() const;
		CString			getLastComponent() const;
		CString			getLastComponentDeletingExtension() const;

		CFilesystemPath	appendingComponent(const CString& component) const;
		CFilesystemPath	deletingLastComponent() const;
		CFilesystemPath	getForResourceFork() const;

		bool			equals(const CFilesystemPath& other) const
							{ return getString() == other.getString(); }

	// Properties
	private:
		CFilesystemPathInternals*	mInternals;
};
