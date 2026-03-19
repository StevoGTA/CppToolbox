//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Apple.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CFilesystem.h"

#import "CLogServices.h"
#import "NSURL+C++.h"
#import "SError-Apple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	const	CString	sErrorDomain(OSSTR("Filesystem-Apple"));
static			SError	sSecurityScopedResourceAccessCouldNotResolveStorageDataError(sErrorDomain, 1,
								CString(OSSTR("Could not resolve storage data")));
static			SError	sSecurityScopedResourceAccessFailedToStartError(sErrorDomain, 2,
								CString(OSSTR("Failed to start")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define	CFilesystemReportFolderError(error, message, folder)										\
				{																					\
					CLogServices::logError(error, message,											\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),			\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);	\
					CLogServices::logError(															\
							CString::mSpaceX4 + CString(OSSTR("Folder: ")) +						\
									folder.getFilesystemPath().getString());						\
				}
#define	CFilesystemReportFilesErrorAndReturnError(error, message, file1, file2)										\
				{																									\
					CLogServices::logError(error, message,															\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),							\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);					\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + file1.getFilesystemPath().getString());	\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + file2.getFilesystemPath().getString());	\
																													\
					return OV<SError>(error);																		\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem::SecurityScopedResourceAccess

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFilesystem::SecurityScopedResourceAccess::SecurityScopedResourceAccess(CFURLRef urlRef) :
		mIsActive(false), mURLRef((CFURLRef) ::CFRetain(urlRef))
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CFilesystem::SecurityScopedResourceAccess::~SecurityScopedResourceAccess()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop if necessary
	stop();

	// Cleanup
	::CFRelease(mURLRef);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::SecurityScopedResourceAccess::start()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if active
	if (!mIsActive) {
		// Start
		Boolean	result = ::CFURLStartAccessingSecurityScopedResource(mURLRef);
		if (!result)
			// Error
			return OV<SError>(sSecurityScopedResourceAccessFailedToStartError);

		// Now active
		mIsActive = true;

		return OV<SError>();
	} else
		// Already active
		return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CFilesystem::SecurityScopedResourceAccess::stop()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if active
	if (mIsActive) {
		// Stop
		::CFURLStopAccessingSecurityScopedResource(mURLRef);

		// No longer active
		mIsActive = false;
	}
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<I<CFilesystem::SecurityScopedResourceAccess> > CFilesystem::SecurityScopedResourceAccess::fromStorageData(
		const CData& storageData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URLRef
	CCoreFoundation::OO<CFURLRef>	urlRef = getURLRefFrom(storageData);
	if (!urlRef.hasObject())
		// Could not resolve
		return TVResult<I<CFilesystem::SecurityScopedResourceAccess> >(
				sSecurityScopedResourceAccessCouldNotResolveStorageDataError);

	return TVResult<I<SecurityScopedResourceAccess> >(
			I<SecurityScopedResourceAccess>(new SecurityScopedResourceAccess(*urlRef)));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<SFoldersFiles> CFilesystem::getFoldersFiles(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CCoreFoundation::O<CFURLRef>	folderURLRef = getURLRefFor(folder);
	NSDirectoryEnumerationOptions	directoryEnumerationOptions =
											deep ? 0 : NSDirectoryEnumerationSkipsSubdirectoryDescendants;

	// Retrieve urls
	__block	NSError*						nsError = nil;
			NSDirectoryEnumerator<NSURL*>*	directoryEnumerator =
													[NSFileManager.defaultManager
															enumeratorAtURL:(__bridge NSURL*) *folderURLRef
															includingPropertiesForKeys:@[]
															options:directoryEnumerationOptions
															errorHandler:^BOOL(NSURL* url, NSError* error) {
																// Store
																nsError = error;

																// Stop
																return NO;
															}];
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (NSURL* url in directoryEnumerator)
		// Add URL
		[urls addObject:url];

	// Check results
	if (nsError == nil)
		// Have urls
		return TVResult<SFoldersFiles>([NSURL foldersFilesFor:urls]);
	else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportFolderError(sError, CString(OSSTR("getting folders and files")), folder);

		return TVResult<SFoldersFiles>(sError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CFolder> > CFilesystem::getFolders(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CCoreFoundation::O<CFURLRef>	folderURLRef = getURLRefFor(folder);
	NSDirectoryEnumerationOptions	directoryEnumerationOptions =
											deep ? 0 : NSDirectoryEnumerationSkipsSubdirectoryDescendants;

	// Retrieve urls
	__block	NSError*						nsError = nil;
			NSDirectoryEnumerator<NSURL*>*	directoryEnumerator =
													[NSFileManager.defaultManager
															enumeratorAtURL:(__bridge NSURL*) *folderURLRef
															includingPropertiesForKeys:[NSArray array]
															options:directoryEnumerationOptions
															errorHandler:^BOOL(NSURL* url, NSError* error) {
																// Store
																nsError = error;

																// Stop
																return NO;
															}];
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (NSURL* url in directoryEnumerator)
		// Add URL
		[urls addObject:url];

	// Check results
	if (nsError == nil)
		// Have urls
		return TVResult<TArray<CFolder> >([NSURL foldersFor:urls]);
	else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportFolderError(sError, CString(OSSTR("getting folders")), folder);

		return TVResult<TArray<CFolder> >(sError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CFile> > CFilesystem::getFiles(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CCoreFoundation::O<CFURLRef>	folderURLRef = getURLRefFor(folder);
	NSDirectoryEnumerationOptions	directoryEnumerationOptions =
											deep ? 0 : NSDirectoryEnumerationSkipsSubdirectoryDescendants;

	// Retrieve urls
	__block	NSError*						nsError = nil;
			NSDirectoryEnumerator<NSURL*>*	directoryEnumerator =
													[NSFileManager.defaultManager
															enumeratorAtURL:(__bridge NSURL*) *folderURLRef
															includingPropertiesForKeys:[NSArray array]
															options:directoryEnumerationOptions
															errorHandler:^BOOL(NSURL* url, NSError* error) {
																// Store
																nsError = error;

																// Stop
																return NO;
															}];
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (NSURL* url in directoryEnumerator)
		// Add URL
		[urls addObject:url];

	// Check results
	if (nsError == nil)
		// Have urls
		return TVResult<TArray<CFile> >([NSURL filesFor:urls]);
	else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportFolderError(sError, CString(OSSTR("getting files")), folder);

		return TVResult<TArray<CFile> >(sError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::replace(const CFile& sourceFile, const CFile& destinationFile)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CCoreFoundation::O<CFURLRef>	sourceURLRef = getURLRefFor(sourceFile);
	CCoreFoundation::O<CFURLRef>	destinationURLRef = getURLRefFor(destinationFile);

	// Replace contents
	NSError*	error;
	if ([[NSFileManager defaultManager] replaceItemAtURL:(__bridge NSURL*) *destinationURLRef
			withItemAtURL:(__bridge NSURL*) *sourceURLRef backupItemName:nil options:0 resultingItemURL:nil
			error:&error])
		// Success
		return OV<SError>();
	else
		// Error
		CFilesystemReportFilesErrorAndReturnError(SErrorFromNSError(error), CString(OSSTR("replacing file")),
				sourceFile, destinationFile);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFURLRef> CFilesystem::getURLRefFor(const CFolder& folder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create URLRef
	return CCoreFoundation::O<CFURLRef>(
			::CFURLCreateWithFileSystemPath(kCFAllocatorDefault, folder.getFilesystemPath().getString().getOSString(),
					kCFURLPOSIXPathStyle, true));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFURLRef> CFilesystem::getURLRefFor(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create URLRef
	return CCoreFoundation::O<CFURLRef>(
			::CFURLCreateWithFileSystemPath(kCFAllocatorDefault, file.getFilesystemPath().getString().getOSString(),
					kCFURLPOSIXPathStyle, false));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::OO<CFURLRef> CFilesystem::getURLRefFrom(const CData& storageData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CCoreFoundation::O<CFDataRef>	storageDataRef = CCoreFoundation::dataRefFrom(storageData);

	// Try to resolve as security-scoped bookmark
	Boolean		cppIsStale;
	CFErrorRef	errorRef;
	CFURLRef	urlRef =
						::CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, *storageDataRef,
								kCFURLBookmarkResolutionWithoutUIMask | kCFURLBookmarkCreationWithSecurityScope, nil,
								nil, &cppIsStale, &errorRef);
	if (urlRef != nil)
		// Success
		return CCoreFoundation::OO<CFURLRef>(urlRef);

	// Try to resolve as bookmark
	urlRef =
			::CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, *storageDataRef,
					kCFURLBookmarkResolutionWithoutUIMask, nil, nil, &cppIsStale, &errorRef);
	if (urlRef != nil)
		// Success
		return CCoreFoundation::OO<CFURLRef>(urlRef);

	// Try to resolve as alias
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	CFDataRef	bookmarkDataRef = ::CFURLCreateBookmarkDataFromAliasRecord(kCFAllocatorDefault, *storageDataRef);
#pragma clang diagnostic pop
	if (bookmarkDataRef != nil) {
		// Try to resolve as security-scoped bookmark
		urlRef =
				::CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, bookmarkDataRef,
						kCFURLBookmarkResolutionWithoutUIMask | kCFURLBookmarkResolutionWithSecurityScope, nil, nil,
						&cppIsStale, &errorRef);
		if (urlRef != nil) {
			// Success
			::CFRelease(bookmarkDataRef);

			return CCoreFoundation::OO<CFURLRef>(urlRef);
		}

		// Try to resolve as bookmark
		urlRef =
				::CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, bookmarkDataRef,
						kCFURLBookmarkResolutionWithoutUIMask, nil, nil, &cppIsStale, &errorRef);
		if (urlRef != nil) {
			// Success
			::CFRelease(bookmarkDataRef);

			return CCoreFoundation::OO<CFURLRef>(urlRef);
		}
	}
	::CFRelease(bookmarkDataRef);

	return CCoreFoundation::OO<CFURLRef>();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CFilesystem::getStorageDataFor(CFURLRef urlRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get storage data
	CFErrorRef	errorRef;
	CFDataRef	dataRef =
						::CFURLCreateBookmarkData(kCFAllocatorDefault, urlRef, kCFURLBookmarkCreationWithSecurityScope,
								nil, nil, &errorRef);
	if (dataRef != nil) {
		// Convert to data
		CData	data = CCoreFoundation::dataFrom(dataRef);
		::CFRelease(dataRef);

		return TVResult<CData>(data);
	} else
		// Error
		return TVResult<CData>(SErrorFromCFError(errorRef));
}
