//----------------------------------------------------------------------------------------------------------------------
//	CFilesystemPath.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystemPath.h"

#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sPathSeparatorPOSIX(OSSTR("/"));
static	CString	sPathSeparatorWindows(OSSTR("\\"));
#if TARGET_OS_MACOS
static	CString	sPathSeparatorHFS(OSSTR(":"));
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	const	CString&	sPathSeparator(EFilesystemPathStyle pathStyle);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystemPathInternals

class CFilesystemPathInternals : public TReferenceCountable<CFilesystemPathInternals> {
	public:
		CFilesystemPathInternals(const CString& string, EFilesystemPathStyle pathStyle) :
			TReferenceCountable(), mString(string), mPathStyle(pathStyle)
			{}

		CString					mString;
		EFilesystemPathStyle	mPathStyle;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystemPath

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath::CFilesystemPath(const CString& string, EFilesystemPathStyle pathStyle) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFilesystemPathInternals(string, pathStyle);
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
CString CFilesystemPath::getString(EFilesystemPathStyle pathStyle) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if same path style
	if (pathStyle == mInternals->mPathStyle)
		// Same
		return mInternals->mString;

	// Must translate path separator
	return mInternals->mString.replacingSubStrings(sPathSeparator(mInternals->mPathStyle), sPathSeparator(pathStyle));
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
	TArray<CString>	parts = getLastComponent().breakUp(CString(OSSTR(".")));

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

	return mInternals->mString.breakUp(sPathSeparator(mInternals->mPathStyle));
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
	TArray<CString>	fields = component.breakUp(CString(OSSTR(".")));

	return (fields.getCount() > 1) ?
			component.getSubString(0, component.getLength() - fields.getLast().getLength() - 1) : component;
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystemPath CFilesystemPath::appendingComponent(const CString& component) const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFilesystemPath(mInternals->mString + sPathSeparator(mInternals->mPathStyle) + component);
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
		return CFilesystemPath(mInternals->mString + sPathSeparator(mInternals->mPathStyle) +
				CString(OSSTR("..namedfork")) + sPathSeparator(mInternals->mPathStyle) + CString(OSSTR("rsrc")));
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString CFilesystemPath::makeLegalFilename(const CString& string, EFilesystemPathMakeLegalFilenameOptions options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get length of string
	CStringLength	length = string.getLength();

	// Extract characters
	TBuffer<UTF16Char>	buffer(length);
	string.get(*buffer, length);

	// Replace "illegal" ones with '_'
	UTF16Char*	p = *buffer;
	for (CStringLength i = 0; i < length; i++, p++) {
		if ((*p < 0x20) || (*p == ':') || (*p == 0x7F))
			*p = '_';

		if ((options & kFilesystemPathMakeLegalFilenameOptionsDisallowSpaces) && (*p == ' '))
			*p = '_';
	}

	return CString(*buffer, length);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
const CString& sPathSeparator(EFilesystemPathStyle pathStyle)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check path style
	switch (pathStyle) {
		case kFilesystemPathStylePOSIX:		return sPathSeparatorPOSIX;
		case kFilesystemPathStyleWindows:	return sPathSeparatorWindows;
#if TARGET_OS_MACOS
		case kFilesystemPathStyleHFS:		return sPathSeparatorHFS;
#endif

#if TARGET_OS_WINDOWS
		default:
			// Just to make compiler happy.  Will never get here.
			return sPathSeparatorWindows;
#endif
	}
}
