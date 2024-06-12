//----------------------------------------------------------------------------------------------------------------------
//	CColorRegistry.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "CNotificationCenter.h"
#include "CPreferences.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CColorGroup

class CColorGroup {
	// Classes
	private:
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												CColorGroup(OSType id, UInt32 displayIndex = 0);
												CColorGroup(const CColorGroup& other);
												~CColorGroup();

												// Instance methods
						OSType					getID() const;
						UInt32					getDisplayIndex() const;

						void					registerColor(OSType id);
				const	TNumberArray<OSType>&	getColorIDs() const;

												// Class methods
		static			bool					compareDisplayIndexes(const CColorGroup& colorGroup1,
														const CColorGroup& colorGroup2, void* userData);

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorSet

class CColorSet {
	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CColorSet(const CString& name);	// Can modify
							CColorSet(OSType id);			// Cannot modify
							CColorSet(const CDictionary& info);
							CColorSet(const CColorSet& other);
							~CColorSet();

							// Instance methods
		const	CString&	getName() const;
				OV<OSType>	getID() const;
				bool		getCanModify() const
								{ return !getID().hasValue(); }

		const	CColor&		getColor(OSType groupID, OSType colorID, const CColor& defaultColor = CColor::mClear) const;
				void		setColor(OSType groupID, OSType colorID, const CColor& color);
				void		setColorsFrom(const CColorSet& other);

				CDictionary	getInfo() const;

				bool		matchesColorsOf(const CColorSet& other) const;

				bool		operator==(const CColorSet& other) const;

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorRegistry

class CColorRegistry {
	// Notifications
	public:
		/*
			Sent when a ColorSet has been changed
				sender is RSender<CColorRegistry>
		*/
		static	const	CString mColorSetChangedNotificationName;

		/*
			Sent when a Color has been changed
				sender is RSender<CColorRegistry>
				info contains the groupID, colorID, and color
		*/
		static	const	CString mColorChangedNotificationName;

		// Notificaton methods
		static			CColorRegistry&	notificatnGetMediaDocument(const OR<CNotificationCenter::Sender>& sender);

		static			OSType			notificationGetGroupID(const CDictionary& info);
		static			OSType			notificationGetColorID(const CDictionary& info);
		static	const	CColor&			notificationGetColor(const CDictionary& info);

	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CColorRegistry();
										CColorRegistry(const CPreferences::Pref& pref);
										~CColorRegistry();

										// Instance methods
				CNotificationCenter&	getNotificationCenter() const;
				CColorGroup&			registerColorGroup(OSType id, UInt32 displayIndex = 0);
				TArray<CColorGroup>		getColorGroups() const;

				CColorSet&				registerColorSetPreset(OSType id);
				TArray<CColorSet>		getColorSets(bool includeColorSetPresets) const;
				void					removeColorSet(const CColorSet& colorSet);

		const	CColorSet&				getCurrentColorSet() const;
				void					setAsCurrent(const CColorSet& colorSet);
				void					setCurrentColorSetColor(OSType groupID, OSType colorID, const CColor& color);
		const	CColorSet&				createNewFromCurrentColorSet(const CString& name);
				void					updateFromCurrentColorSet(CColorSet& colorSet) const;
				OR<CColorSet>			getFirstMatchingColorsOfCurrentColorSet() const;

				bool					operator==(const CColorRegistry& other) const
											{ return &other == this; }

	// Properties
	private:
		Internals*	mInternals;
};
