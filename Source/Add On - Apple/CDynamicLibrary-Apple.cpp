//----------------------------------------------------------------------------------------------------------------------
//	CDynamicLibrary-Apple.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDynamicLibrary.h"

#include <Security/Security.h>
#include <sys/sysctl.h>
#include <sys/xattr.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDynamicLibrary

// MARK: Class methods

#if defined(TARGET_OS_MACOS)
//----------------------------------------------------------------------------------------------------------------------
OV<CFile> CDynamicLibrary::getExecutableFile(const CFolder& bundleFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFURLRef	urlRef =
						::CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
								bundleFolder.getFilesystemPath().getString().getOSString(), kCFURLPOSIXPathStyle,
								true);
	CFBundleRef	bundleRef = ::CFBundleCreate(kCFAllocatorDefault, urlRef);
	::CFRelease(urlRef);
	if (bundleRef == nil)
		// Not a bundle
		return OV<CFile>();

	// Get executable
	CFURLRef	executableURLRef = ::CFBundleCopyExecutableURL(bundleRef);
	::CFRelease(bundleRef);
	if (executableURLRef == nil)
		// No executable
		return OV<CFile>();

	// Compose file
	CFURLRef		absoluteExecutableURLRef = ::CFURLCopyAbsoluteURL(executableURLRef);
	CFStringRef		pathStringRef = ::CFURLCopyFileSystemPath(absoluteExecutableURLRef, kCFURLPOSIXPathStyle);
	CFilesystemPath	filesystemPath(CString(pathStringRef), CFilesystemPath::kStylePOSIX);
	::CFRelease(pathStringRef);
	::CFRelease(absoluteExecutableURLRef);
	::CFRelease(executableURLRef);

	return OV<CFile>(CFile(filesystemPath));
}
#endif

//----------------------------------------------------------------------------------------------------------------------
OV<SVersionInfo> CDynamicLibrary::getVersionInfo(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFURLRef	urlRef =
						::CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
								filesystemPath.getString().getOSString(), kCFURLPOSIXPathStyle, true);
	CFBundleRef	bundleRef = ::CFBundleCreate(kCFAllocatorDefault, urlRef);
	::CFRelease(urlRef);
	if (bundleRef == nil)
		// Not a bundle
		return OV<SVersionInfo>();

	// Get version
	CFTypeRef			typeRef =
								::CFBundleGetValueForInfoDictionaryKey(bundleRef, CFSTR("CFBundleShortVersionString"));
	OV<SVersionInfo>	versionInfo =
								((typeRef != nil) && (::CFGetTypeID(typeRef) == ::CFStringGetTypeID())) ?
										OV<SVersionInfo>(SVersionInfo::getFrom(CString((CFStringRef) typeRef))) :
										OV<SVersionInfo>();
	::CFRelease(bundleRef);

	return versionInfo;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CDynamicLibrary::getManufacturer(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFURLRef			urlRef =
								::CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
										filesystemPath.getString().getOSString(), kCFURLPOSIXPathStyle, true);
	SecStaticCodeRef	staticCodeRef;
	OSStatus			status = ::SecStaticCodeCreateWithPath(urlRef, kSecCSDefaultFlags, &staticCodeRef);
	::CFRelease(urlRef);
	if (status != errSecSuccess)
		// Not readable
		return OV<CString>();

	// Get signing information
	CFDictionaryRef	informationRef;
	status = ::SecCodeCopySigningInformation(staticCodeRef, kSecCSSigningInformation, &informationRef);
	::CFRelease(staticCodeRef);
	if (status != errSecSuccess)
		// Not signed
		return OV<CString>();

	// Get the signer's name from the first certificate
	CFArrayRef	certificatesRef = (CFArrayRef) ::CFDictionaryGetValue(informationRef, kSecCodeInfoCertificates);
	CFStringRef	commonNameRef = nil;
	if ((certificatesRef != nil) && (::CFArrayGetCount(certificatesRef) > 0))
		// Copy common name
		::SecCertificateCopyCommonName((SecCertificateRef) ::CFArrayGetValueAtIndex(certificatesRef, 0),
				&commonNameRef);
	::CFRelease(informationRef);
	if (commonNameRef == nil)
		// Signed without a certificate
		return OV<CString>();

	// Compose manufacturer, dropping the certificate kind ("Developer ID Application: ") and the team ID (" (...)")
	CString	manufacturer(commonNameRef);
	::CFRelease(commonNameRef);

	OV<SRange32>	kindRange = manufacturer.findSubString(CString(OSSTR(": ")));
	if (kindRange.hasValue())
		// Drop kind
		manufacturer = manufacturer.getSubString(kindRange->getStart() + kindRange->getLength());

	OV<SRange32>	teamIDRange = manufacturer.findSubString(CString(OSSTR(" (")));
	if (teamIDRange.hasValue() && manufacturer.hasSuffix(CString(OSSTR(")"))))
		// Drop team ID
		manufacturer = manufacturer.getSubString(0, teamIDRange->getStart());

	return OV<CString>(manufacturer);
}

//----------------------------------------------------------------------------------------------------------------------
CDynamicLibrary::Loadability CDynamicLibrary::getLoadability(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFURLRef	urlRef =
						::CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
								filesystemPath.getString().getOSString(), kCFURLPOSIXPathStyle, true);
	CFBundleRef	bundleRef = ::CFBundleCreate(kCFAllocatorDefault, urlRef);
	::CFRelease(urlRef);
	if (bundleRef == nil)
		// Not a bundle
		return Loadability(Loadability::kStateUnreadable);

	// Get architectures
	CFArrayRef	architecturesRef = ::CFBundleCopyExecutableArchitectures(bundleRef);
	if (architecturesRef == nil) {
		// No readable executable
		::CFRelease(bundleRef);

		return Loadability(Loadability::kStateUnreadable);
	}

	// Check for this process's architecture
#if defined(__arm64__)
	SInt32		architecture = kCFBundleExecutableArchitectureARM64;
#elif defined(__x86_64__)
	SInt32		architecture = kCFBundleExecutableArchitectureX86_64;
#endif
	CFNumberRef	architectureNumberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &architecture);
	bool		architectureFound =
						::CFArrayContainsValue(architecturesRef, ::CFRangeMake(0, ::CFArrayGetCount(architecturesRef)),
								architectureNumberRef);
	::CFRelease(architectureNumberRef);
	::CFRelease(architecturesRef);
	if (!architectureFound) {
		// Wrong architecture
		::CFRelease(bundleRef);

		return Loadability(Loadability::kStateWrongArchitecture);
	}

	// Check the minimum OS version against this one (compared numerically, so 10.15 is after 10.9)
	CFTypeRef	minimumSystemVersionTypeRef =
						::CFBundleGetValueForInfoDictionaryKey(bundleRef, CFSTR("LSMinimumSystemVersion"));
	char		currentSystemVersion[32];
	size_t		currentSystemVersionByteCount = sizeof(currentSystemVersion);
	if ((minimumSystemVersionTypeRef != nil) &&
			(::CFGetTypeID(minimumSystemVersionTypeRef) == ::CFStringGetTypeID()) &&
			(::sysctlbyname("kern.osproductversion", currentSystemVersion, &currentSystemVersionByteCount, nil, 0) ==
					0) &&
			(::CFStringCompare((CFStringRef) minimumSystemVersionTypeRef,
					CString(currentSystemVersion, (CString::Length) ::strlen(currentSystemVersion),
							CString::kEncodingUTF8).getOSString(),
					kCFCompareNumerically) == kCFCompareGreaterThan)) {
		// Newer OS required
		Loadability	loadability(SVersionInfo::getFrom(CString((CFStringRef) minimumSystemVersionTypeRef)));
		::CFRelease(bundleRef);

		return loadability;
	}
	::CFRelease(bundleRef);

	// Check quarantine.  The first field of the attribute is hex flags, where 0x0040 notes the user approved it.
	char	quarantine[256];
	ssize_t	quarantineByteCount =
					::getxattr(*filesystemPath.getString().getUTF8String(), "com.apple.quarantine",
							quarantine, sizeof(quarantine) - 1, 0, 0);
	if (quarantineByteCount > 0) {
		// Have quarantine
		quarantine[quarantineByteCount] = 0;
		if ((::strtoul(quarantine, nil, 16) & 0x0040) == 0)
			// Not yet approved
			return Loadability(Loadability::kStateNotYetApproved);
	}

	return Loadability(Loadability::kStatePossible);
}
