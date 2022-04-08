//----------------------------------------------------------------------------------------------------------------------
//	NSString+C++.mm			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "NSString+C++.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

static	NSString*	sStringFor(const CString& string);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CString

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CString& localizationGroup, const CString& localizationKey)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mStringRef =
			(CFStringRef) ::CFBridgingRetain([[NSBundle mainBundle] localizedStringForKey:sStringFor(localizationKey)
					value:sStringFor(localizationKey) table:sStringFor(localizationGroup)]);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - NSString extension

@implementation NSString (Cpp)

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
+ (NSString*) stringForCString:(const CString&) string
{
	return sStringFor(string);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
- (NSString*) stringByReplacingOccurrencesOfCString:(const CString&) target withCString:(const CString&) replacement
{
	return [self stringByReplacingOccurrencesOfString:sStringFor(target) withString:sStringFor(replacement)];
}

@end

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
NSString* sStringFor(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	return CFBridgingRelease(
			::CFStringCreateWithCString(kCFAllocatorDefault, *string.getCString(CString::kEncodingUTF8),
			kCFStringEncodingUTF8));
}
