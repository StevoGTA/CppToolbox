//----------------------------------------------------------------------------------------------------------------------
//	CFolder-macOS.mm			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

#include "CCoreFoundation.h"

#include <Foundation/Foundation.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFolder

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CFolder::isPackage() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(getFilesystemPath(), false));

	// Get info
	NSNumber*	number;

	return [url getResourceValue:&number forKey:NSURLIsPackageKey error:nil] && number.boolValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::tempFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil)
		// Setup
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) NSTemporaryDirectory())));

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::systemApplicationSupportFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path =
							NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSLocalDomainMask, YES)
									[0];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::systemAudioPluginsFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path =
							[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSLocalDomainMask, YES)
									[0] stringByAppendingPathComponent:@"Audio/Plug-Ins"];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::systemAudioPresetsFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path =
							[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSLocalDomainMask, YES)
									[0] stringByAppendingPathComponent:@"Audio/Presets"];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::systemFrameworksFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		CFURLRef	urlRef =
							::CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("/System/Library/Frameworks"),
									kCFURLPOSIXPathStyle, false);
		CFStringRef	stringRef = ::CFURLCopyPath(urlRef);
		::CFRelease(urlRef);

		sFolder = new CFolder(CFilesystemPath(CString(stringRef)));
		::CFRelease(stringRef);
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::systemLibraryFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSLocalDomainMask, YES)[0];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userApplicationSupportFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES)[0];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userAudioPluginsFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path =
							[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)
									[0] stringByAppendingPathComponent:@"Audio/Plug-Ins"];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userAudioPresetsFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path =
							[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)
									[0] stringByAppendingPathComponent:@"Audio/Presets"];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userDesktopFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES)[0];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userHomeFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path = NSHomeDirectory();
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userLibraryFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userLogsFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
		sFolder =
				new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path))
						.appendingComponent(CString(OSSTR("Logs"))));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userMusicFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		NSString*	path = NSSearchPathForDirectoriesInDomains(NSMusicDirectory, NSUserDomainMask, YES)[0];
		sFolder = new CFolder(CFilesystemPath(CString((__bridge CFStringRef) path)));
	}

	return *sFolder;
}
