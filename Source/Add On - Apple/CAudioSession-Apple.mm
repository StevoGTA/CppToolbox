//----------------------------------------------------------------------------------------------------------------------
//	CAudioSession-iOS.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioSession.h"

#include "CLogServices.h"
#include "CNotificationCenter.h"

#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	#import <AVFoundation/AVFoundation.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: NotificationObserver

#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
@interface NotificationObserver : NSObject
@end

@implementation NotificationObserver

//----------------------------------------------------------------------------------------------------------------------
- (void) handleInterruptionNotification:(NSNotification*) notification
{
	// Retrieve info
	NSDictionary<NSString*, NSObject*>*	userInfo = notification.userInfo;
	AVAudioSessionInterruptionType		interruptionType =
												((NSNumber*) userInfo[AVAudioSessionInterruptionTypeKey]).intValue;

	// Check type
	switch (interruptionType) {
		case AVAudioSessionInterruptionTypeBegan:
			// Began
			// Post notification
			CNotificationCenter::mStandard.send(CAudioSession::mInterruptionDidBeginNotificationName,
					&CAudioSession::shared);
			break;

		case AVAudioSessionInterruptionTypeEnded:
			// Ended
			// Get info
			AVAudioSessionInterruptionOptions	interruptionOptions =
														((NSNumber*) userInfo[AVAudioSessionInterruptionOptionKey])
																.intValue;

			// Post notification
			CDictionary	info;
			info.set(CAudioSession::mInterruptionDidEndPlaybackShouldContinue,
					(interruptionOptions & AVAudioSessionInterruptionOptionShouldResume) != 0);

			CNotificationCenter::mStandard.send(CAudioSession::mInterruptionDidEndNotificationName,
					&CAudioSession::shared, info);
			break;
	}
}

@end
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local data

#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
static	bool					sIsAudioSessionActive = false;
static	NotificationObserver*	sNotificationObserver = [[NotificationObserver alloc] init];
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioSession

// MARK: Properties

CAudioSession CAudioSession::shared;

CString CAudioSession::mInterruptionDidBeginNotificationName(OSSTR("CAudioSession - Interruption Did Begin"));
CString CAudioSession::mInterruptionDidEndNotificationName(OSSTR("CAudioSession - Interruption Did End"));
CString	CAudioSession::mInterruptionDidEndPlaybackShouldContinue(OSSTR("playbackShouldContinue"));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioSession::CAudioSession()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup notifications
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	NSNotificationCenter*	notificationCenter = NSNotificationCenter.defaultCenter;
	[notificationCenter addObserver:sNotificationObserver selector:@selector(handleInterruptionNotification:)
			name:AVAudioSessionInterruptionNotification object:nil];
#endif

}

//----------------------------------------------------------------------------------------------------------------------
CAudioSession::~CAudioSession()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup notifications
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::activate()
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	// Activate
	NSError*	error;
	BOOL		result = [AVAudioSession.sharedInstance setActive:YES error:&error];
	if (result)
		// Success
		sIsAudioSessionActive = true;
	else
		// Failed
		CLogServices::logMessage(CString(OSSTR("CAudioSession failed to activate with error ")) +
				CString((__bridge CFStringRef) error.localizedDescription));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::deactivate()
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	// Deactivate
	NSError*	error;
	BOOL		result = [AVAudioSession.sharedInstance setActive:NO error:&error];
	if (!result)
		// Failed
		CLogServices::logMessage(CString(OSSTR("CAudioSession failed to deactivate with error ")) +
				CString((__bridge CFStringRef) error.localizedDescription));

	// Update
	sIsAudioSessionActive = false;
#endif

}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioSession::isActivated() const
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	return sIsAudioSessionActive;
#else
	return true;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioSession::inputIsAvailable() const
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	return AVAudioSession.sharedInstance.isInputAvailable;
#else
	return true;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::set(Category category)
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	// Convert category to AVAudioSessionCategory
	NSString*	avAudioSessionCategory;
	switch (category) {
		case kPlaybackMixWithOthers:
			// Playback, mix with others
			avAudioSessionCategory = AVAudioSessionCategoryAmbient;
			break;

		case kPlaybackDoNotMixWithOthers:
			// Playback, do not mix with others
			avAudioSessionCategory = AVAudioSessionCategoryPlayback;
			break;

		case kPlaybackDoNotMixWithOthersBackground:
			// Playback, mix with others, background
			avAudioSessionCategory = AVAudioSessionCategorySoloAmbient;
			break;

		case kRecordAndPlayback:
			// Record and playback
			avAudioSessionCategory = AVAudioSessionCategoryPlayAndRecord;
			break;
	}

	// Set category
	NSError*	error;
	BOOL		result = [AVAudioSession.sharedInstance setCategory:avAudioSessionCategory error:&error];
	if (!result)
		// Failed
		CLogServices::logMessage(CString(OSSTR("CAudioSession failed to set category with error ")) +
				CString((__bridge CFStringRef) error.localizedDescription));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::set(OverrideRoute overrideRoute)
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	// Convert override route to AVAudioSessionOverrideRoute
	NSUInteger	avAudioSessionOverrideRoute;
	switch (overrideRoute) {
		case kNone:		avAudioSessionOverrideRoute = AVAudioSessionPortOverrideNone;		break;
		case kSpeaker:	avAudioSessionOverrideRoute = AVAudioSessionPortOverrideSpeaker;	break;
	}

	// Set override output audio port
	NSError*	error;
	BOOL	result = [AVAudioSession.sharedInstance overrideOutputAudioPort:avAudioSessionOverrideRoute error:&error];
	if (!result)
		// Failed
		CLogServices::logMessage(CString(OSSTR("CAudioSession failed to set override route with error ")) +
				CString((__bridge CFStringRef) error.localizedDescription));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CAudioSession::getCurrentHardwareIOBufferDuration() const
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	return AVAudioSession.sharedInstance.IOBufferDuration;
#else
	return 0.25;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::setPreferredHardwareIOBufferDuration(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	NSError*	error;
	BOOL		result = [AVAudioSession.sharedInstance setPreferredIOBufferDuration:timeInterval error:&error];
	if (!result)
		// Failed
		CLogServices::logMessage(CString(OSSTR("CAudioSession failed to set preferred IO buffer duration with error "))
				+ CString((__bridge CFStringRef) error.localizedDescription));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CAudioSession::getCurrentHardwareOutputVolume() const
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	return AVAudioSession.sharedInstance.outputVolume;
#else
	return 1.0; 
#endif
}
