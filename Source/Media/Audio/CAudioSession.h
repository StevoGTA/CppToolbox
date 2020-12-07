//----------------------------------------------------------------------------------------------------------------------
//	CAudioSession-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioSession

class CAudioSession {
	// Enums
	public:
		enum Category {
			kPlaybackMixWithOthers,
			kPlaybackDoNotMixWithOthers,
			kPlaybackDoNotMixWithOthersBackground,
			kRecordAndPlayback,
		};

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

#if TARGET_OS_MACOS || TARGET_OS_WINDOWS
		void					logInfo();
#endif

	private:
								// Lifecycle methods
								CAudioSession();
								~CAudioSession();

	// Properties
	public:
		static	CAudioSession	mShared;

		/*
			Sent when the the audio session is interrupted
				senderRef is CAudioSession
		*/
		static	CString			mInterruptionDidBeginNotificationName;

		/*
			Sent when the audio session interruption is complete
				senderRef is CAudioSession
				info has the following keys;
					mInterruptionDidEndPlaybackShouldContinue	// bool
		*/
		static	CString			mInterruptionDidEndNotificationName;
		static	CString			mInterruptionDidEndPlaybackShouldContinue;

//		/*
//			Sent when the requested property has changed
//				senderRef is CAudioSession
//				info has the following keys:
//					mPropertyDidChangePropertyName
//					mPropertyDidChangePropertyValue
//		*/
//		static	CString			mPropertyDidChangeNotificationName;
//		static	CString			mPropertyDidChangePropertyName;		// OSType
//		static	CString			mPropertyDidChangePropertyValue;	// CData
};
