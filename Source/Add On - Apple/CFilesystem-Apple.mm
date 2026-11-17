//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Apple.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CFilesystem.h"

#import "CCoreFoundation.h"
#import "CLogServices.h"
#import "SError-Apple.h"
#import "SError-POSIX.h"

#import <Foundation/Foundation.h>
#import <sys/attr.h>
#import <sys/mount.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

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
#define	CFilesystemReportFolderErrorAndReturnValue(error, message, folder, value)					\
				{																					\
					CLogServices::logError(error, message,											\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),			\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);	\
					CLogServices::logError(															\
							CString::mSpaceX4 + CString(OSSTR("Folder: ")) +						\
									folder.getFilesystemPath().getString());						\
																									\
					return value;																	\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs

//----------------------------------------------------------------------------------------------------------------------
static TVResult<CFilesystem::VolumeInfo> sGetVolumeInfo(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get volume info
	CString::C		path = filesystemPath.getString().getUTF8String();
	struct statfs	statfsInfo;
	if (::statfs(*path, &statfsInfo) != 0)
		// Error
		return TVResult<CFilesystem::VolumeInfo>(SErrorFromPOSIXerror(errno));

	// Get volume capabilities.  Volume attributes must be requested from the volume's mount point.  A volume that
	//	can't report its capabilities is treated as supporting neither hard links nor extended attributes.
	struct attrlist	attributeList = {0};
	attributeList.bitmapcount = ATTR_BIT_MAP_COUNT;
	attributeList.volattr = ATTR_VOL_INFO | ATTR_VOL_CAPABILITIES;

	struct {
		UInt32					mByteCount;
		vol_capabilities_attr_t	mCapabilities;
	} __attribute__((aligned(4), packed))	capabilitiesInfo;

	bool	hasCapabilities =
					::getattrlist(statfsInfo.f_mntonname, &attributeList, &capabilitiesInfo, sizeof(capabilitiesInfo),
							0) == 0;
	bool	supportsHardLinks =
					hasCapabilities &&
							((capabilitiesInfo.mCapabilities.capabilities[VOL_CAPABILITIES_FORMAT] &
									capabilitiesInfo.mCapabilities.valid[VOL_CAPABILITIES_FORMAT] &
											VOL_CAP_FMT_HARDLINKS) != 0);
	bool	supportsExtendedAttributes =
					hasCapabilities &&
							((capabilitiesInfo.mCapabilities.capabilities[VOL_CAPABILITIES_INTERFACES] &
									capabilitiesInfo.mCapabilities.valid[VOL_CAPABILITIES_INTERFACES] &
											VOL_CAP_INT_EXTENDED_ATTR) != 0);

	return TVResult<CFilesystem::VolumeInfo>(
			CFilesystem::VolumeInfo(
					CString(statfsInfo.f_fstypename, MFSTYPENAMELEN, CString::kEncodingUTF8),
					((UInt64) (UInt32) statfsInfo.f_fsid.val[0] << 32) | (UInt32) statfsInfo.f_fsid.val[1],
					CFolder(CFilesystemPath(CString(statfsInfo.f_mntonname, MAXPATHLEN, CString::kEncodingUTF8))),
					statfsInfo.f_bsize, (statfsInfo.f_flags & MNT_LOCAL) != 0, (statfsInfo.f_flags & MNT_RDONLY) != 0,
					supportsHardLinks, supportsExtendedAttributes));
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
			return OV<SError>(mSecurityScopedResourceAccessFailedToStartError);

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
				mSecurityScopedResourceAccessCouldNotResolveStorageDataError);

	return TVResult<I<SecurityScopedResourceAccess> >(
			I<SecurityScopedResourceAccess>(new SecurityScopedResourceAccess(*urlRef)));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

// MARK :Properties

SError	CFilesystem::mSecurityScopedResourceAccessCouldNotResolveStorageDataError(mErrorDomain, 100);
SError	CFilesystem::mSecurityScopedResourceAccessCouldNotComposeStorageDataError(mErrorDomain, 101);
SError	CFilesystem::mSecurityScopedResourceAccessFailedToStartError(mErrorDomain, 102);

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
	if (nsError == nil) {
		// Have urls
		TNArray<CFolder>	folders;
		TNArray<CFile>		files;

		// Iterate NSURLs
		for (NSURL* url in urls) {
			// Check if directory
			if (url.hasDirectoryPath)
				// Folder
				folders += CFolder(CFilesystemPath(CString((__bridge OSStringType) url.path)));
			else
				// File
				files += CFile(CFilesystemPath(CString((__bridge OSStringType) url.path)));
		}

		return TVResult<SFoldersFiles>(SFoldersFiles(folders, files));
	} else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportFolderErrorAndReturnValue(sError, CString(OSSTR("getting folders and files")), folder,
				TVResult<SFoldersFiles>(sError));
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
	if (nsError == nil) {
		// Have urls
		TNArray<CFolder>	folders;

		// Iterate NSURLs
		for (NSURL* url in urls) {
			// Check if directory
			if (url.hasDirectoryPath)
				// Folder
				folders += CFolder(CFilesystemPath(CString((__bridge OSStringType) url.path)));
		}

		return TVResult<TArray<CFolder> >(folders);
	} else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportFolderErrorAndReturnValue(sError, CString(OSSTR("getting folders")), folder,
				TVResult<TArray<CFolder> >(sError));
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
	if (nsError == nil) {
		// Have urls
		TNArray<CFile>	files;

		// Iterate NSURLs
		for (NSURL* url in urls) {
			// Check if directory
			if (!url.hasDirectoryPath)
				// File
				files += CFile(CFilesystemPath(CString((__bridge OSStringType) url.path)));
		}

		return TVResult<TArray<CFile> >(files);
	} else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportFolderErrorAndReturnValue(sError, CString(OSSTR("getting files")), folder,
				TVResult<TArray<CFile> >(sError));
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CFilesystem::VolumeInfo> CFilesystem::getVolumeInfo(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get Volume Info
	return sGetVolumeInfo(file.getFilesystemPath());
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CFilesystem::VolumeInfo> CFilesystem::getVolumeInfo(const CFolder& folder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get Volume Info
	return sGetVolumeInfo(folder.getFilesystemPath());
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CFolder> CFilesystem::createTempFolder(const VolumeInfo& volumeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create.  macOS removes these folders when they are left behind.
	NSError*	error;
	NSURL*		url =
						[NSFileManager.defaultManager URLForDirectory:NSItemReplacementDirectory
								inDomain:NSUserDomainMask
								appropriateForURL:(__bridge NSURL*) *getURLRefFor(volumeInfo.getRootFolder())
								create:YES error:&error];

	return (url != nil) ?
			TVResult<CFolder>(CFolder(CFilesystemPath(CString((__bridge CFStringRef) url.path)))) :
			TVResult<CFolder>(SErrorFromNSError(error));
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
	if ([NSFileManager.defaultManager replaceItemAtURL:(__bridge NSURL*) *destinationURLRef
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
OV<SError> CFilesystem::createHardLink(const CFile& file, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFile	destinationFile(destinationFolder.getFilesystemPath().appendingComponent(file.getName()));

	// Create hard link
	if (::link(*file.getFilesystemPath().getString().getUTF8String(),
			*destinationFile.getFilesystemPath().getString().getUTF8String()) == 0)
		// Success
		return OV<SError>();
	else
		// Error
		CFilesystemReportFilesErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("creating hard link")),
				file, destinationFile);
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
	CFURLRef	urlRef =
						::CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, *storageDataRef,
								kCFURLBookmarkResolutionWithoutUIMask | kCFURLBookmarkCreationWithSecurityScope, nil,
								nil, &cppIsStale, nil);
	if (urlRef != nil)
		// Success
		return CCoreFoundation::OO<CFURLRef>(urlRef);

	// Try to resolve as bookmark
	urlRef =
			::CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, *storageDataRef,
					kCFURLBookmarkResolutionWithoutUIMask, nil, nil, &cppIsStale, nil);
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
						&cppIsStale, nil);
		if (urlRef != nil) {
			// Success
			::CFRelease(bookmarkDataRef);

			return CCoreFoundation::OO<CFURLRef>(urlRef);
		}

		// Try to resolve as bookmark
		urlRef =
				::CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, bookmarkDataRef,
						kCFURLBookmarkResolutionWithoutUIMask, nil, nil, &cppIsStale, nil);
		if (urlRef != nil) {
			// Success
			::CFRelease(bookmarkDataRef);

			return CCoreFoundation::OO<CFURLRef>(urlRef);
		}

		// Cleanup
		::CFRelease(bookmarkDataRef);
	}

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
	} else {
		// Error
		SError	error = SErrorFromCFError(errorRef);
		::CFRelease(errorRef);

		return TVResult<CData>(error);
	}
}
