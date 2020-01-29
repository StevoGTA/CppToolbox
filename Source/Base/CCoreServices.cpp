//----------------------------------------------------------------------------------------------------------------------
//	CCoreServices.cpp			©2017 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreServices.h"

#if TARGET_OS_MACOS
	#include "CFolder.h"
	#include "CFUtilities.h"

	#include <sys/sysctl.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreServices methods

//----------------------------------------------------------------------------------------------------------------------
const SSystemVersionInfo& CCoreServices::getSystemVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	static	SSystemVersionInfo*	sVersionInfo = nil;

	if (sVersionInfo == nil) {
		// Get info
#if TARGET_OS_MACOS
		// Setup
		FILE*	file = ::popen("sw_vers", "r");
		char	line[256];

		::fgets(line, sizeof(line), file);
		CString	productName = CString(line).breakUp(CString::mTabCharacter)[1].removeLeadingAndTrailingWhitespace();

		::fgets(line, sizeof(line), file);
		TArray<CString>	components =
								CString(line).breakUp(CString::mTabCharacter)[1].breakUp(CString::mPeriodCharacter);

		::fgets(line, sizeof(line), file);
		CString	buildVersion = CString(line).breakUp(CString::mTabCharacter)[1].removeLeadingAndTrailingWhitespace();

		sVersionInfo =
				new SSystemVersionInfo(productName, components[0].getUInt32(), components[1].getUInt32(),
						(components.getCount() == 3) ? components[2].getUInt32() : 0, buildVersion);

		// Cleanup
		pclose(file);
#endif
	}

	return *sVersionInfo;
}

#if TARGET_OS_MACOS
//----------------------------------------------------------------------------------------------------------------------
const SVersionInfo& CCoreServices::getCoreAudioVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	static	SVersionInfo*	sVersionInfo = nil;

	if (sVersionInfo == nil) {
		// Get info
		CFURLRef	urlRef =
							eFilesystemPathCopyURLRef(
									CFolder::systemFrameworksFolder().getFilesystemPath()
											.appendingComponent(CString("CoreAudio.framework")), true);
		CFBundleRef	bundleRef = ::CFBundleCreate(kCFAllocatorDefault, urlRef);
		::CFRelease(urlRef);
		CFStringRef	stringRef =
							(CFStringRef) ::CFBundleGetValueForInfoDictionaryKey(bundleRef,
									CFSTR("CFBundleShortVersionString"));
		::CFRelease(bundleRef);
		TArray<CString>	array = CString(stringRef).breakUp(CString("."));
		UInt8			majorVersion = (array.getCount() > 0) ? array[0].getUInt8() : 0;
		UInt8			minorVersion = (array.getCount() > 1) ? array[1].getUInt8() : 0;
		UInt8			patchVersion = (array.getCount() > 2) ? array[2].getUInt8() : 0;

		sVersionInfo =
				(majorVersion != 0) ?
						new SVersionInfo(majorVersion, minorVersion, patchVersion) :
						new SVersionInfo(CString(stringRef));
	}

	return *sVersionInfo;
}
#endif

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreServices::getTotalProcessorCoresCount()
//----------------------------------------------------------------------------------------------------------------------
{
	static	int	sTotalProcessorCoresCount = 0;

	if (sTotalProcessorCoresCount == 0) {
		// Get info
#if TARGET_OS_MACOS
		int		request[2] = {CTL_HW, HW_NCPU};
		size_t	size = sizeof(int);
		int		status = ::sysctl(request, 2, &sTotalProcessorCoresCount, &size, nil, 0);
		if (status != 0)
			sTotalProcessorCoresCount = 1;
#endif
	}

	return sTotalProcessorCoresCount;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CCoreServices::getProcessorInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CString*	sProcessorInfoString = nil;

	if (sProcessorInfoString == nil) {
		// Get info
#if TARGET_OS_MACOS
		char	buffer[1024];
		size_t	size = sizeof(buffer);
		int		status = ::sysctlbyname("machdep.cpu.brand_string", &buffer, &size, nil, 0);
		if (status == 0)
			sProcessorInfoString = new CString(buffer);
#endif
	}

    return (sProcessorInfoString != nil) ? *sProcessorInfoString : CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CCoreServices::getPhysicalMemoryByteCount()
//----------------------------------------------------------------------------------------------------------------------
{
	static	int64_t	sPhysicalMemoryByteCount = 0;

	if (sPhysicalMemoryByteCount == 0) {
		// Get info
#if TARGET_OS_MACOS
		int		request[2] = {CTL_HW, HW_MEMSIZE};
		size_t	size = sizeof(int64_t);
		::sysctl(request, 2, &sPhysicalMemoryByteCount, &size, nil, 0);
#endif
	}

	return sPhysicalMemoryByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreServices::getPhysicalMemoryPageSize()
//----------------------------------------------------------------------------------------------------------------------
{
	static	UInt32	sPhysicalMemoryPageSize = 0;

#if TARGET_OS_MACOS
	if (sPhysicalMemoryPageSize == 0)
		// Get info
		sPhysicalMemoryPageSize = (UInt32) ::sysconf(_SC_PAGE_SIZE);
#endif

	return sPhysicalMemoryPageSize;
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreServices::stopInDebugger()
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_MACOS
	kill(getpid(), SIGINT);
#endif
}
