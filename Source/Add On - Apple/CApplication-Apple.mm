//----------------------------------------------------------------------------------------------------------------------
//	CApplication-Apple.mm			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CApplication.h"

#if defined(TARGET_OS_MACOS)
	#import "CLogServices.h"
	#import "SError-Apple.h"

	#import <AppKit/AppKit.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CApplication::Internals

class CApplication::Internals {
	public:
						Internals(CFURLRef urlRef, const CString& displayName, const CString& version,
								const OV<CString>& copyright) :
							mURLRef((CFURLRef) ::CFRetain(urlRef)), mDisplayName(displayName), mVersion(version),
									mCopyright(copyright)
							{}
						~Internals()
							{ ::CFRelease(mURLRef); }

		static	CString	getBundleString(CFBundleRef bundleRef, CFStringRef key)
							{
								// Read
								CFStringRef	stringRef =
													(CFStringRef) ::CFBundleGetValueForInfoDictionaryKey(bundleRef,
															key);

								return (stringRef != nil) ? CString(stringRef) : CString::mEmpty;
							}

		CFURLRef	mURLRef;
		CString		mDisplayName;
		CString		mVersion;
		OV<CString>	mCopyright;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CApplication

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CApplication::CApplication(CFURLRef urlRef, const CString& displayName, const CString& version,
		const OV<CString>& copyright)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(urlRef, displayName, version, copyright);
}

//----------------------------------------------------------------------------------------------------------------------
CApplication::CApplication(const CApplication& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(other.mInternals->mURLRef, other.mInternals->mDisplayName, other.mInternals->mVersion,
					other.mInternals->mCopyright);
}

//----------------------------------------------------------------------------------------------------------------------
CApplication::~CApplication()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString CApplication::getDisplayName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDisplayName;
}

//----------------------------------------------------------------------------------------------------------------------
CString CApplication::getVersion() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVersion;
}

//----------------------------------------------------------------------------------------------------------------------
CString CApplication::getDisplayNameAndVersion() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVersion.isEmpty() ?
			mInternals->mDisplayName : (mInternals->mDisplayName + CString::mSpace + mInternals->mVersion);
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CApplication::getCopyright() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCopyright;
}

//----------------------------------------------------------------------------------------------------------------------
CFolder CApplication::getFolder() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Derive from the application URL
	CFStringRef		pathStringRef = ::CFURLCopyFileSystemPath(mInternals->mURLRef, kCFURLPOSIXPathStyle);
	CFilesystemPath	filesystemPath(CString(pathStringRef), CFilesystemPath::kStylePOSIX);
	::CFRelease(pathStringRef);

	return CFolder(filesystemPath.deletingLastComponent());
}

#if defined(TARGET_OS_MACOS)
//----------------------------------------------------------------------------------------------------------------------
OV<SError> CApplication::open(const TArray<CFile>& files) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (TArray<CFile>::Iterator iterator = files.getIterator(); iterator; iterator++)
		// Add URL
		[urls
				addObject:
						::CFBridgingRelease(
								::CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
										iterator->getFilesystemPath().getString().getOSString(), kCFURLPOSIXPathStyle,
										false))];

	// Open
			dispatch_semaphore_t	semaphore = dispatch_semaphore_create(0);
	__block	NSError*				openURLsError = nil;
	[[NSWorkspace sharedWorkspace] openURLs:urls withApplicationAtURL:(__bridge NSURL*) mInternals->mURLRef
			configuration:[NSWorkspaceOpenConfiguration configuration]
			completionHandler:^(NSRunningApplication* runningApplication, NSError* error){
		// Store error
		openURLsError = error;

		// Done
		dispatch_semaphore_signal(semaphore);
	}];
	while (dispatch_semaphore_wait(semaphore, DISPATCH_TIME_NOW))
		// Run current RunLoop
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0]];

	// Handle results
	if (openURLsError == nil)
		// Success
		return OV<SError>();
	else {
		// Error
		OV<SError>	error(SErrorFromNSError(openURLsError));
		LogError(*error, CString(OSSTR("opening files with")));

		return error;
	}
}
#endif

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CApplication::getStorageData() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get storage data
	CFErrorRef	errorRef;
	CFDataRef	dataRef =
						::CFURLCreateBookmarkData(kCFAllocatorDefault, mInternals->mURLRef,
								kCFURLBookmarkCreationWithSecurityScope, nil, nil, &errorRef);
	if (dataRef != nil) {
		// Convert to data
		CData	data = CCoreFoundation::dataFrom(dataRef);
		::CFRelease(dataRef);

		return TVResult<CData>(data);
	} else
		// Error
		return TVResult<CData>(SErrorFromCFError(errorRef));
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CApplication CApplication::getCurrent()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFBundleRef	mainBundleRef = ::CFBundleGetMainBundle();

	// Compose display name, falling back to the bundle name
	CString	displayName = Internals::getBundleString(mainBundleRef, CFSTR("CFBundleDisplayName"));
	if (displayName.isEmpty())
		displayName = Internals::getBundleString(mainBundleRef, CFSTR("CFBundleName"));

	// Compose version and copyright
	CString	version = Internals::getBundleString(mainBundleRef, CFSTR("CFBundleShortVersionString"));
	CString	copyright = Internals::getBundleString(mainBundleRef, CFSTR("CFBundleShortCopyrightString"));

	// Compose
	CFURLRef		urlRef = ::CFBundleCopyBundleURL(mainBundleRef);
	CApplication	application(urlRef, displayName, version,
							!copyright.isEmpty() ? OV<CString>(copyright) : OV<CString>());
	::CFRelease(urlRef);

	return application;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CApplication> CApplication::getFrom(const CData& storageData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Resolve the security-scoped bookmark to a URL
	CCoreFoundation::O<CFDataRef>	storageDataRef = CCoreFoundation::dataRefFrom(storageData);
	Boolean							isStale;
	CFURLRef						urlRef =
											::CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, *storageDataRef,
													kCFURLBookmarkResolutionWithoutUIMask |
															kCFURLBookmarkResolutionWithSecurityScope,
													nil, nil, &isStale, nil);
	if (urlRef == nil)
		// The application is no longer present
		return OV<CApplication>();

	// Compose
	OV<CApplication>	application = getFor(urlRef);
	::CFRelease(urlRef);

	return application;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CApplication> CApplication::getFor(CFURLRef urlRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose the display name from the last path component
	CFStringRef	lastComponentStringRef = ::CFURLCopyLastPathComponent(urlRef);
	CString		displayName = CString(lastComponentStringRef).replacingSubStrings(CString(OSSTR(".app")));
	::CFRelease(lastComponentStringRef);

	return OV<CApplication>(CApplication(urlRef, displayName, CString::mEmpty, OV<CString>()));
}
