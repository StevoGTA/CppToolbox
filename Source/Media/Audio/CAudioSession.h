//----------------------------------------------------------------------------------------------------------------------
//	CAudioSession-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CNotificationCenter.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioSession

class CAudioSession {
	// Notifications
	public:
		/*
			Sent when the the audio session is interrupted
				sender is RSender<CAudioSession>
		*/
		static	const	CString	mInterruptionDidBeginNotificationName;

		/*
			Sent when the audio session interruption is complete
				sender is RSender<CAudioSession>
				info contains if playback should continue
		*/
		static	const	CString	mInterruptionDidEndNotificationName;

		// Notification methods
		static	CAudioSession&	notificatnGetMediaDocument(const OR<CNotificationCenter::Sender>& sender);

		static	bool			notificationGetPlaybackShouldContinue(const CDictionary& info);

//		/*
//			Sent when the requested property has changed
//				senderRef is CAudioSession
//				info has the following keys:
//					mPropertyDidChangePropertyName
//					mPropertyDidChangePropertyValue
//		*/
//		static	const	CString			mPropertyDidChangeNotificationName;
//		static	const	CString			mPropertyDidChangePropertyName;		// OSType
//		static	const	CString			mPropertyDidChangePropertyValue;	// CData

	// Category
	public:
		enum Category {
			kPlaybackMixWithOthers,
			kPlaybackDoNotMixWithOthers,
			kPlaybackDoNotMixWithOthersBackground,
			kRecordAndPlayback,
		};

	// OverrideRoute
	public:
		enum OverrideRoute {
			kNone,
			kSpeaker,
		};

	// Methods
	public:
								// Instance methods
		void					activate();
		void					deactivate();
		bool					isActivated() const;

		bool					inputIsAvailable() const;
		void					set(Category category);
		void					set(OverrideRoute overrideRoute);

		UniversalTimeInterval	getCurrentHardwareIOBufferDuration() const;
		void					setPreferredHardwareIOBufferDuration(UniversalTimeInterval timeInterval);

		Float32					getCurrentHardwareOutputVolume() const;

//		void					addPropertyChangedListener(OSType propertyID, CEventHandler& eventHandler);
//		void					removePropertyChangedListener(CEventHandler& eventHandler);

#if defined(TARGET_OS_MACOS) || defined(TARGET_OS_WINDOWS)
		void					logInfo();
#endif

		bool					operator==(const CAudioSession& other) const
									{ return &other == this; }

	private:
								// Lifecycle methods
								CAudioSession();
								~CAudioSession();

	// Properties
	public:
		static	CAudioSession					mShared;
		static	CImmediateNotificationCenter	mNotificationCenter;
};
