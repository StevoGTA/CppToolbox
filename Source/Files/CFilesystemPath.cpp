//----------------------------------------------------------------------------------------------------------------------
//	CFilesystemPath.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystemPath.h"

#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

static	const	CString&	sPathSeparator(CFilesystemPath::Style style);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystemPath::Internals

class CFilesystemPath::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const CString& string, CFilesystemPath::Style style) :
			TReferenceCountableAutoDelete(),
					mString(string), mStyle(style)
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
	mInternals = new Internals(string, style);
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
OV<CString> CFilesystemPath::getExtension() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get last component
	OV<CString>	component = getLastComponent();
	if (!component.hasValue())
		// We're empty
		return OV<CString>();

	// Get last component and break into fields based on "."
	TArray<CString>	parts = component->components(CString(OSSTR(".")));
	if (parts.getCount() == 1)
		// No extension
		return OV<CString>();

	return OV<CString>(parts.getLast());
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CFilesystemPath::getComponents() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if empty
	if (mInternals->mString.isEmpty())
		// We're empty
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
OV<CString> CFilesystemPath::getLastComponent() const
//----------------------------------------------------------------------------------------------------------------------
{
	return !mInternals->mString.isEmpty() ? OV<CString>(getComponents().getLast()) : OV<CString>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CFilesystemPath::getLastComponentDeletingExtension() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get last component
	OV<CString>	component = getLastComponent();
	if (!component.hasValue())
		// We're empty
		return OV<CString>();

	// Get fields
	TArray<CString>	fields = component->components(CString(OSSTR(".")));

	return (fields.getCount() > 1) ?
			OV<CString>(component->getSubString(0, component->getLength() - fields.getLast().getLength() - 1)) :
			component;
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
		return CFilesystemPath(mInternals->mString + sPathSeparator(mInternals->mStyle) + component,
				mInternals->mStyle);
	else if ((mInternals->mStyle == kStylePOSIX) && (style == kStyleWindows))
		// Convert Windows => POSIX
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(kStylePOSIX) +
						component.replacingSubStrings(CString(OSSTR("\\")), CString(OSSTR("/"))),
				kStylePOSIX);
	else if ((mInternals->mStyle == kStyleWindows) && (style == kStylePOSIX))
		// Convert POSIX => Windows
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(kStyleWindows) +
						component.replacingSubStrings(CString(OSSTR("/")), CString(OSSTR("\\"))),
				kStyleWindows);
#if defined(TARGET_OS_MACOS)
	else if ((mInternals->mStyle == kStylePOSIX) && (style == kStyleHFS))
		// Convert HFS => POSIX
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(kStylePOSIX) +
						component
								.replacingSubStrings(CString(OSSTR(":")), CString(OSSTR("{COLON}")))
								.replacingSubStrings(CString(OSSTR("/")), CString(OSSTR(":")))
								.replacingSubStrings(CString(OSSTR("{COLON}")), CString(OSSTR("/"))),
				kStylePOSIX);
	else if ((mInternals->mStyle == kStyleWindows) && (style == kStyleHFS))
		// Convert HFS => Windows
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(kStyleWindows) +
						component.replacingSubStrings(CString(OSSTR(":")), CString(OSSTR("\\"))),
				kStyleWindows);
	else if ((mInternals->mStyle == kStyleHFS) && (style == kStylePOSIX))
		// Convert POSIX => HFS
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(kStyleHFS) +
						component
								.replacingSubStrings(CString(OSSTR("/")), CString(OSSTR("{SLASH}")))
								.replacingSubStrings(CString(OSSTR(":")), CString(OSSTR("/")))
								.replacingSubStrings(CString(OSSTR("{SLASH}")), CString(OSSTR(":"))),
				kStyleHFS);
	else
		// Convert Windows => HFS
		return CFilesystemPath(
				mInternals->mString + sPathSeparator(kStyleHFS) +
						component.replacingSubStrings(CString(OSSTR("\\")), CString(OSSTR(":"))),
				kStyleHFS);
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
		return CFilesystemPath(CString::mEmpty, mInternals->mStyle);

	return CFilesystemPath(
			mInternals->mString.getSubString(
					0, mInternals->mString.getLength() - components.getLast().getLength() - 1),
			mInternals->mStyle);
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath CFilesystemPath::appendingExtension(const CString& extension) const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFilesystemPath(mInternals->mString + CString::mPeriod + extension, mInternals->mStyle);
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath& CFilesystemPath::operator=(const CFilesystemPath& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Change
	mInternals->removeReference();
	mInternals = other.mInternals->addReference();

	return *this;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString CFilesystemPath::makeLegalFilename(const CString& string, MakeLegalFilenameOptions makeLegalFilenameOptions)
//----------------------------------------------------------------------------------------------------------------------
{

	// Extract characters
	TBuffer<UTF32Char>	buffer = string.getUTF32Chars();

	// Replace "illegal" ones with '_'
	UTF32Char*	p = *buffer;
	for (CString::Length i = 0; i < buffer.getCount(); i++, p++) {
		if ((*p < 0x20) || (*p == '/') || (*p == ':') || (*p == 0x7F))
			// Replace with _
			*p = '_';
#if defined(TARGET_OS_WINDOWS)
		else if ((*p == '\\') || (*p == '*') || (*p == '?') || (*p == '"') || (*p == '<') || (*p == '>') ||
				(*p == '|'))
			// Replace with _
			*p = '_';
#endif
		else if ((makeLegalFilenameOptions & kMakeLegalFilenameOptionsDisallowSpaces) && (*p == ' '))
			// Replace with _
			*p = '_';
	}

#if defined(TARGET_OS_WINDOWS)
	// Windows drops trailing spaces and periods, so replace them with '_'
	for (CString::Length i = buffer.getCount(); (i > 0) && (((*buffer)[i - 1] == ' ') || ((*buffer)[i - 1] == '.'));
			i--)
		// Replace with _
		(*buffer)[i - 1] = '_';

	// Windows reserves the device names, with or without an extension, so follow one with '_'
	static	const	CString	sReservedNames[] =
								{
									CString(OSSTR("AUX")),
									CString(OSSTR("COM1")), CString(OSSTR("COM2")), CString(OSSTR("COM3")),
									CString(OSSTR("COM4")), CString(OSSTR("COM5")), CString(OSSTR("COM6")),
									CString(OSSTR("COM7")), CString(OSSTR("COM8")), CString(OSSTR("COM9")),
									CString(OSSTR("CON")),
									CString(OSSTR("LPT1")), CString(OSSTR("LPT2")), CString(OSSTR("LPT3")),
									CString(OSSTR("LPT4")), CString(OSSTR("LPT5")), CString(OSSTR("LPT6")),
									CString(OSSTR("LPT7")), CString(OSSTR("LPT8")), CString(OSSTR("LPT9")),
									CString(OSSTR("NUL")),
									CString(OSSTR("PRN")),
								};
	CString	filename(buffer);
	CString	baseName = filename.components(CString::mPeriod).getFirst();
	CString	baseNameUppercased = baseName.uppercased();
	for (size_t i = 0; i < sizeof(sReservedNames) / sizeof(sReservedNames[0]); i++) {
		// Check this name
		if (baseNameUppercased == sReservedNames[i])
			// Reserved
			return baseName + CString::mUnderscore + filename.getSubString(baseName.getLength());
	}

	return filename;
#else
	return CString(buffer);
#endif
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
