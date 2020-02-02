//----------------------------------------------------------------------------------------------------------------------
//	CFilesystemPath.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystemPath.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sPathSeparatorPOSIX("/");
static	CString	sPathSeparatorWindows("\\");
#if TARGET_OS_MACOS
static	CString	sPathSeparatorHFS(":");
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
	TArray<CString>	parts = getLastComponent().breakUp(CString("."));

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
	TArray<CString>	fields = component.breakUp(CString("."));

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
CFilesystemPath CFilesystemPath::getForResourceFork() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get components
	TArray<CString>	components = getComponents();
	CArrayItemCount	componentsCount = components.getCount();

	// Check components count
	if (componentsCount == 0)
		// Empty
		return CFilesystemPath(CString::mEmpty);

	// Check if already have resource fork info
	if ((components.getCount() > 2) && (components[componentsCount - 2] == CString("..namedfork")) &&
			(components[componentsCount - 1] == CString("rsrc")))
		// Already setup
		return *this;
	else
		// Update
		return CFilesystemPath(mInternals->mString + sPathSeparator(mInternals->mPathStyle) + CString("..namedfork") +
								sPathSeparator(mInternals->mPathStyle) + CString("rsrc"));
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
	}
}
