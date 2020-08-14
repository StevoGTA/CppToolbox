//----------------------------------------------------------------------------------------------------------------------
//	CCoreServices-macOS.cpp			©2017 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreServices.h"

#include "CFolder.h"
#include "CCoreFoundation.h"

#include <sys/sysctl.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreServices

// MARK: Info methods

//----------------------------------------------------------------------------------------------------------------------
const SSystemVersionInfo& CCoreServices::getSystemVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	static	SSystemVersionInfo*	sVersionInfo = nil;

	if (sVersionInfo == nil) {
		// Get info
		// Setup
		FILE*	file = ::popen("sw_vers", "r");
		char	line[256];

		::fgets(line, sizeof(line), file);
		CString	productName = CString(line).breakUp(CString::mTab)[1].removingLeadingAndTrailingWhitespace();

		::fgets(line, sizeof(line), file);
		TArray<CString>	components =
								CString(line).breakUp(CString::mTab)[1].breakUp(CString::mPeriod);

		::fgets(line, sizeof(line), file);
		CString	buildVersion = CString(line).breakUp(CString::mTab)[1].removingLeadingAndTrailingWhitespace();

		sVersionInfo =
				new SSystemVersionInfo(productName, components[0].getUInt32(), components[1].getUInt32(),
						(components.getCount() == 3) ? components[2].getUInt32() : 0, buildVersion);

		// Cleanup
		pclose(file);
	}

	return *sVersionInfo;
}

//----------------------------------------------------------------------------------------------------------------------
const SVersionInfo& CCoreServices::getCoreAudioVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	static	SVersionInfo*	sVersionInfo = nil;

	if (sVersionInfo == nil) {
		// Get info
		CFURLRef	urlRef =
							CCoreFoundation::createURLRefFrom(
									CFolder::systemFrameworksFolder().getFilesystemPath()
											.appendingComponent(CString(CFSTR("CoreAudio.framework"))), true);
		CFBundleRef	bundleRef = ::CFBundleCreate(kCFAllocatorDefault, urlRef);
		::CFRelease(urlRef);
		CFStringRef	stringRef =
							(CFStringRef) ::CFBundleGetValueForInfoDictionaryKey(bundleRef,
									CFSTR("CFBundleShortVersionString"));
		::CFRelease(bundleRef);
		TArray<CString>	array = CString(stringRef).breakUp(CString(CFSTR(".")));
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

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreServices::getTotalProcessorCoresCount()
//----------------------------------------------------------------------------------------------------------------------
{
	static	int	sTotalProcessorCoresCount = 0;

	if (sTotalProcessorCoresCount == 0) {
		// Get info
		int		request[2] = {CTL_HW, HW_NCPU};
		size_t	size = sizeof(int);
		int		status = ::sysctl(request, 2, &sTotalProcessorCoresCount, &size, nil, 0);
		if (status != 0)
			sTotalProcessorCoresCount = 1;
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
		char	buffer[1024];
		size_t	size = sizeof(buffer);
		int		status = ::sysctlbyname("machdep.cpu.brand_string", &buffer, &size, nil, 0);
		if (status == 0)
			sProcessorInfoString = new CString(buffer);
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
		int		request[2] = {CTL_HW, HW_MEMSIZE};
		size_t	size = sizeof(int64_t);
		::sysctl(request, 2, &sPhysicalMemoryByteCount, &size, nil, 0);
	}

	return sPhysicalMemoryByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreServices::getPhysicalMemoryPageSize()
//----------------------------------------------------------------------------------------------------------------------
{
	static	UInt32	sPhysicalMemoryPageSize = 0;

	if (sPhysicalMemoryPageSize == 0)
		// Get info
		sPhysicalMemoryPageSize = (UInt32) ::sysconf(_SC_PAGE_SIZE);

	return sPhysicalMemoryPageSize;
}

// MARK: Debugger methods

//----------------------------------------------------------------------------------------------------------------------
void CCoreServices::stopInDebugger()
//----------------------------------------------------------------------------------------------------------------------
{
	kill(getpid(), SIGINT);
}
