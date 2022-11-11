//----------------------------------------------------------------------------------------------------------------------
//	CFilesystemPath.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFilesystemPath

class CFilesystemPathInternals;
class CFilesystemPath : public CHashable {
	// Enums
	public:
		enum Style {
			// Paths in the style "Volume/Folder/Folder/"
			kStylePOSIX,

			// Paths in the style "C:\Volume\Folder\Folder\"
			kStyleWindows,

#if defined(TARGET_OS_MACOS)
			// Deprecated paths in the style "Volume:Folder:Folder:"
			kStyleHFS,
#endif

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS) || \
		defined(TARGET_OS_LINUX)
			kStylePlatformDefault = kStylePOSIX,
#endif

#if defined(TARGET_OS_WINDOWS)
			kStylePlatformDefault = kStyleWindows,
#endif
		};

		enum MakeLegalFilenameOptions {
			kMakeLegalFilenameOptionsNone				= 0,
			kMakeLegalFilenameOptionsDisallowSpaces	= 1 << 0,
		};

	// Methods
	public:
								// Lifecycle methods
								CFilesystemPath(const CString& string, Style style = kStylePlatformDefault);
								CFilesystemPath(const CFilesystemPath& other);
								~CFilesystemPath();

								// CEquatable methods
				bool			operator==(const CEquatable& other) const
									{ return equals((const CFilesystemPath&) other); }

								// CHashable methods
				void			hashInto(CHasher& hasher) const
									{ getString().hashInto(hasher); }

								// Instance methods
				CString			getString(Style style = kStylePlatformDefault, const CString& wrapper = CString::mEmpty)
										const;

				OV<CString>		getExtension() const;

				TArray<CString>	getComponents() const;
				OV<CString>		getLastComponent() const;
				OV<CString>		getLastComponentDeletingExtension() const;
				CString			getLastComponentForDisplay() const;

				CFilesystemPath	appendingComponent(const CString& component, Style style = kStylePlatformDefault) const;
				CFilesystemPath	deletingLastComponent() const;
				CFilesystemPath	appendingExtension(const CString& extension) const;

				bool			equals(const CFilesystemPath& other) const
									{ return getString() == other.getString(); }

				CFilesystemPath&	operator=(const CFilesystemPath& other);

								// Class methods
		static	CString			makeLegalFilename(const CString& string,
										MakeLegalFilenameOptions makeLegalFilenameOptions =
												kMakeLegalFilenameOptionsNone);

	// Properties
	private:
		CFilesystemPathInternals*	mInternals;
};
