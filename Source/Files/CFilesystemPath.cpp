//----------------------------------------------------------------------------------------------------------------------
//	CFilesystemPath.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystemPath.h"

#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

static	const	CString&	sPathSeparator(CFilesystemPath::Style style);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystemPathInternals

class CFilesystemPathInternals : public TReferenceCountable<CFilesystemPathInternals> {
	public:
		CFilesystemPathInternals(const CString& string, CFilesystemPath::Style style) :
			TReferenceCountable(), mString(string), mStyle(style)
			{}

		CString					mString;
		CFilesystemPath::Style	mStyle;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystemPath

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath::CFilesystemPath(const CString& string, Style style) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFilesystemPathInternals(string, style);
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath::CFilesystemPath(const CFilesystemPath& other) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath::~CFilesystemPath()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString CFilesystemPath::getString(Style style, const CString& wrapper) const
//----------------------------------------------------------------------------------------------------------------------
{
	return wrapper +
			((style == mInternals->mStyle) ?
					mInternals->mString :
					mInternals->mString.replacingSubStrings(sPathSeparator(mInternals->mStyle),
							sPathSeparator(style))) +
			wrapper;
}

//----------------------------------------------------------------------------------------------------------------------
CString CFilesystemPath::getExtension() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if empty
	if (mInternals->mString.isEmpty())
		// No extension
		return CString::mEmpty;

	// Get last component and break into fields based on "."
	TArray<CString>	parts = getLastComponent().components(CString(OSSTR(".")));

	return (parts.getCount() > 1) ? parts.getLast() : CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CFilesystemPath::getComponents() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if empty
	if (mInternals->mString.isEmpty())
		// No components
		return TNArray<CString>();

	// Setup
	const	CString&	pathSeparator = sPathSeparator(mInternals->mStyle);

	// Check style
	if (mInternals->mStyle == kStylePOSIX) {
		// POSIX converts "/" in path components to ":"
		if (mInternals->mString.hasPrefix(CString(OSSTR("/"))))
			// Process as full path
			return TNArray<CString>(mInternals->mString.getSubString(1).components(pathSeparator));
		else
			// Process as subPath
			return TNArray<CString>(mInternals->mString.components(pathSeparator));
	} else
		// All others get passed through
		return mInternals->mString.components(pathSeparator);
}

//----------------------------------------------------------------------------------------------------------------------
CString CFilesystemPath::getLastComponent() const
//----------------------------------------------------------------------------------------------------------------------
{
	return !mInternals->mString.isEmpty() ? getComponents().getLast() : CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CFilesystemPath::getLastComponentDeletingExtension() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get last component
	CString	component = getLastComponent();

	// Check if empty
	if (component.isEmpty())
		// Empty
		return CString::mEmpty;

	// Get fields
	TArray<CString>	fields = component.components(CString(OSSTR(".")));

	return (fields.getCount() > 1) ?
			component.getSubString(0, component.getLength() - fields.getLast().getLength() - 1) : component;
}

//----------------------------------------------------------------------------------------------------------------------
CString CFilesystemPath::getLastComponentForDisplay() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have path
	if (!mInternals->mString.isEmpty()) {
		// Have path
		CString	lastComponent = getComponents().getLast();
		switch (mInternals->mStyle) {
			case kStylePOSIX:
				// POSIX - ":" => "/"
				return lastComponent.replacingSubStrings(CString(OSSTR(":")), CString(OSSTR("/")));

#if defined(TARGET_OS_MACOS)
			case kStyleHFS:
				// HFS - "/" => ":"
				return lastComponent.replacingSubStrings(CString(OSSTR("/")), CString(OSSTR(":")));
#endif

			default:
				// Everything else
				return lastComponent;
		}
	} else
		// No path
		return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath CFilesystemPath::appendingComponent(const CString& component, Style style) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if styles match
	if (mInternals->mStyle == style)
		// Add
		return CFilesystemPath(mInternals->mString + sPathSeparator(mInternals->mStyle) + component);
	else if ((mInternals->mStyle == kStylePOSIX) && (style == kStyleWindows))
		// Convert Windows => POSIX
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(mInternals->mStyle) +
						component.replacingSubStrings(CString(OSSTR("\\")), CString(OSSTR("/"))));
	else if ((mInternals->mStyle == kStyleWindows) && (style == kStylePOSIX))
		// Convert POSIX => Windows
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(mInternals->mStyle) +
						component.replacingSubStrings(CString(OSSTR("/")), CString(OSSTR("\\"))));
#if defined(TARGET_OS_MACOS)
	else if ((mInternals->mStyle == kStylePOSIX) && (style == kStyleHFS))
		// Convert HFS => POSIX
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(mInternals->mStyle) +
						component
								.replacingSubStrings(CString(OSSTR(":")), CString(OSSTR("{COLON}")))
								.replacingSubStrings(CString(OSSTR("/")), CString(OSSTR(":")))
								.replacingSubStrings(CString(OSSTR("{COLON}")), CString(OSSTR("/"))));
	else if ((mInternals->mStyle == kStyleWindows) && (style == kStyleHFS))
		// Convert HFS => Windows
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(mInternals->mStyle) +
						component.replacingSubStrings(CString(OSSTR(":")), CString(OSSTR("\\"))));
	else if ((mInternals->mStyle == kStyleHFS) && (style == kStylePOSIX))
		// Convert POSIX => HFS
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(mInternals->mStyle) +
						component
								.replacingSubStrings(CString(OSSTR("/")), CString(OSSTR("{SLASH}")))
								.replacingSubStrings(CString(OSSTR(":")), CString(OSSTR("/")))
								.replacingSubStrings(CString(OSSTR("{SLASH}")), CString(OSSTR(":"))));
	else
		// Convert Windows => HFS
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(mInternals->mStyle) +
						component.replacingSubStrings(CString(OSSTR("\\")), CString(OSSTR(":"))));
#else
	// For non-macOS platforms - need to make the compiler happy
	else
		return *this;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath CFilesystemPath::deletingLastComponent() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get components
	TArray<CString>	components = getComponents();

	// Check if empty
	if (components.isEmpty())
		// No extension
		return CFilesystemPath(CString::mEmpty);

	return CFilesystemPath(
			mInternals->mString.getSubString(
					0, mInternals->mString.getLength() - components.getLast().getLength() - 1));
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath CFilesystemPath::appendingExtension(const CString& extension) const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFilesystemPath(mInternals->mString + CString::mPeriod + extension);
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath CFilesystemPath::getForResourceFork() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get components
	TArray<CString>		components = getComponents();
	CArray::ItemCount	componentsCount = components.getCount();

	// Check components count
	if (componentsCount == 0)
		// Empty
		return CFilesystemPath(CString::mEmpty);

	// Check if already have resource fork info
	if ((components.getCount() > 2) && (components[componentsCount - 2] == CString(OSSTR("..namedfork"))) &&
			(components[componentsCount - 1] == CString(OSSTR("rsrc"))))
		// Already setup
		return *this;
	else
		// Update
		return CFilesystemPath(mInternals->mString + sPathSeparator(mInternals->mStyle) +
				CString(OSSTR("..namedfork")) + sPathSeparator(mInternals->mStyle) + CString(OSSTR("rsrc")));
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString CFilesystemPath::makeLegalFilename(const CString& string, MakeLegalFilenameOptions makeLegalFilenameOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get length of string
	CString::Length	length = string.getLength();

	// Extract characters
	TBuffer<UTF16Char>	buffer(length);
	string.get(*buffer, length);

	// Replace "illegal" ones with '_'
	UTF16Char*	p = *buffer;
	for (CString::Length i = 0; i < length; i++, p++) {
		if ((*p < 0x20) || (*p == ':') || (*p == 0x7F))
			*p = '_';

		if ((makeLegalFilenameOptions & kMakeLegalFilenameOptionsDisallowSpaces) && (*p == ' '))
			*p = '_';
	}

	return CString(*buffer, length);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
const CString& sPathSeparator(CFilesystemPath::Style style)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CString	sPathSeparatorPOSIX(OSSTR("/"));
	static	CString	sPathSeparatorWindows(OSSTR("\\"));
#if defined(TARGET_OS_MACOS)
	static	CString	sPathSeparatorHFS(OSSTR(":"));
#endif

	// Check path style
	switch (style) {
		case CFilesystemPath::kStylePOSIX:		return sPathSeparatorPOSIX;
		case CFilesystemPath::kStyleWindows:	return sPathSeparatorWindows;
#if defined(TARGET_OS_MACOS)
		case CFilesystemPath::kStyleHFS:		return sPathSeparatorHFS;
#endif

#if defined(TARGET_OS_WINDOWS)
		default:
			// Just to make compiler happy.  Will never get here.
			return sPathSeparatorWindows;
#endif
	}
}
