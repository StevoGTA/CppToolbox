//----------------------------------------------------------------------------------------------------------------------
//	CClipboard-macOS.mm			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CClipboard.h"

#import "CCoreFoundation.h"

#import <AppKit/AppKit.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: ClipboardOwner

@interface ClipboardOwner : NSObject <NSPasteboardTypeOwner>

// MARK: Properties

@property (nonatomic, assign)	CClipboard::ContentsReplacedProc	contentsReplacedProc;
@property (nonatomic, assign)	void*								userData;

@end

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local data

static	NSMutableSet<ClipboardOwner*>*	sClipboardOwners = [NSMutableSet set];

static	SError							sUnableToSetDataError(CString(OSSTR("CClipboard")), 1,
												CString(OSSTR("Unable to set clipboard data")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - ClipboardOwner

@implementation ClipboardOwner

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
- (instancetype) initWithContentsReplacedProc:(CClipboard::ContentsReplacedProc) contentsReplacedProc
		userData:(void*) userData
{
	// Do super
	if (self = [super init]) {
		// Store
		self.contentsReplacedProc = contentsReplacedProc;
		self.userData = userData;
	}

	return self;
}

// MARK: NSPasteboardTypeOwner methods

//----------------------------------------------------------------------------------------------------------------------
- (void) pasteboard:(NSPasteboard*) pasteboard provideDataForType:(NSPasteboardType) type
{
}

//----------------------------------------------------------------------------------------------------------------------
- (void) pasteboardChangedOwner:(NSPasteboard*) pasteboard
{
	// Call proc
	self.contentsReplacedProc(self.userData);

	// No longer needed
	[sClipboardOwners removeObject:self];
}

@end

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CClipboard

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CClipboard::set(const CString& type, const CData& data, ContentsReplacedProc contentsReplacedProc,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSPasteboard*		pasteboard = NSPasteboard.generalPasteboard;
	NSString*			typeString = (__bridge NSString*) type.getOSString();
	ClipboardOwner*		owner =
								[[ClipboardOwner alloc] initWithContentsReplacedProc:contentsReplacedProc
										userData:userData];

	// Set
	[pasteboard declareTypes:@[typeString] owner:owner];

	// Add owner
	[sClipboardOwners addObject:owner];

	return [pasteboard setData:(__bridge NSData*) *CCoreFoundation::dataRefFrom(data) forType:typeString] ?
			OV<SError>() : OV<SError>(sUnableToSetDataError);
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CClipboard::getData(const CString& type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get data
	NSData*	data = [NSPasteboard.generalPasteboard dataForType:(__bridge NSString*) type.getOSString()];

	return (data != nil) ? OV<CData>(CCoreFoundation::dataFrom((__bridge CFDataRef) data)) : OV<CData>();
}
