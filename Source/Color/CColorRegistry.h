//----------------------------------------------------------------------------------------------------------------------
//	CColorRegistry.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "CNotificationCenter.h"
#include "CPreferences.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CColorRegistry

class CColorRegistry {
	// Group
	public:
		struct Group {
			// Entry
			public:
				struct Entry {
					// Methods
					public:
											// Lifecycle methods
											Entry(OSType id, const CString& name) : mID(id), mName(name) {}

											// Instance methods
								OSType		getID() const
												{ return mID; }
						const	CString&	getName() const
												{ return mName; }
					// Properties
					private:
						OSType	mID;
						CString	mName;
				};

			// Methods
			public:
									// Lifecycle methods
									Group(OSType id, const CString& name) : mID(id), mName(name) {}
									
									// Instance methods
						OSType		getID() const
										{ return mID; }
				const	CString&	getName() const
										{ return mName; }

			// Properties
			private:
				OSType	mID;
				CString	mName;
		};

	// Preset
	public:
		struct Preset {
			// Methods
			public:
									// Lifecycle methods
									Preset(OSType presetID, const CString& name) :
										mID(presetID), mUniqueID(presetID, true, false), mName(name)
										{}
									Preset(const CString& uniqueID, const CString& name) :
										mUniqueID(uniqueID), mName(name)
										{}
									Preset(const CString& name) :
										mUniqueID(CUUID().getHexString()), mName(name)
										{}

									// Instance methods
				const	OV<OSType>&	getID() const
										{ return mID; }
				const	CString&	getUniqueID() const
										{ return mUniqueID; }
				const	CString&	getName() const
										{ return mName; }

						bool		operator==(const Preset& other) const
										{ return mUniqueID == other.mUniqueID; }

			// Properties
			private:
				OV<OSType>	mID;
				CString		mUniqueID;
				CString		mName;
		};

	// Notifications
	public:
		/*
			Sent when all colors may have changed
				sender is RSender<CColorRegistry>
		*/
		static	const	CString	mColorsChangedNotificationName;

		/*
			Sent when one color has changed
				sender is RSender<CColorRegistry>
				info contains the groupID, entryID, and color
		*/
		static	const	CString	mColorChangedNotificationName;

		// Notification methods
		static			OSType	notificationGetGroupID(const CDictionary& info);
		static			OSType	notificationGetGroupEntryID(const CDictionary& info);
		static	const	CColor&	notificationGetColor(const CDictionary& info);

	// Classes
	private:
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												~CColorRegistry();

												// Instance methods
						void					noteGroup(OSType groupID, const CString& name, const bool& isEnabled);
						void					noteGroupEntry(OSType groupID, OSType entryID, const CString& name);
						TArray<Group>			getEnabledGroups() const;
						TArray<Group::Entry>	getGroupEntries(const Group& group) const;

						void					notePreset(OSType presetID, const CString& name);
						void					setColor(OSType presetID, OSType groupID, OSType entryID,
														const CColor& color);

						TArray<Preset>			getPresets() const;
						void					usePreset(const Preset& preset);

						OV<Preset>				getPresetMatchingCurrentColors() const;
						void					createPresetFromCurrentColors(const CString& name);
						void					updatePresetFromCurrentColors(const Preset& preset);
						void					removePreset(const Preset& preset);


				const	CColor&					getColor(OSType groupID, OSType entryID) const;
						void					setColor(OSType groupID, OSType entryID, const CColor& color);

						CNotificationCenter&	getNotificationCenter() const;

						void					usePersistence(const CPreferences::Pref& pref);

						bool					operator==(const CColorRegistry& other) const
													{ return &other == this; }

												// Class methods
		static			CColorRegistry&			shared();

	private:
												// Lifecycle methods
												CColorRegistry();

	// Properties
	private:
		Internals*	mInternals;
};
