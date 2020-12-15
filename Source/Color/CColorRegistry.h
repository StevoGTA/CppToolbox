//----------------------------------------------------------------------------------------------------------------------
//	CColorRegistry.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "CNotificationCenter.h"
#include "CPreferences.h"
//#include "CString.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CColorGroup

class CColorGroupInternals;
class CColorGroup {
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
				const	TNumericArray<OSType>&	getColorIDs() const;

												// Class methods
		static	ECompareResult					compareDisplayIndexes(const CColorGroup& colorGroup1,
														const CColorGroup& colorGroup2, void* userData);

	// Properties
	private:
		CColorGroupInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorSet

class CColorSetInternals;
class CColorSet {
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

		const	CColor&		getColor(OSType colorGroupID, OSType colorID,
									const CColor& defaultColor = CColor::mClear) const;
				void		setColor(OSType colorGroupID, OSType colorID, const CColor& color);
				void		setColorsFrom(const CColorSet& other);

				CDictionary	getInfo() const;

				bool		matchesColorsOf(const CColorSet& other) const;

				bool		operator==(const CColorSet& other) const;

	// Properties
	private:
		CColorSetInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorRegistry

class CColorRegistryInternals;
class CColorRegistry {
	// Notifications
	public:
		/*
			Sent when a color has been changed
				senderRef is CColorRegistry
				info contains the following keys:
					mGroupIDKey
					mColorIDKey
					mColorKey
		*/
		static	CString mColorChangedNotificationName;

		static	CString	mGroupIDKey;	// OSType
		static	CString	mColorIDKey;	// OSType
		static	CString	mColorKey;		// CColor*

	// Methods
	public:
									// Lifecycle methods
									CColorRegistry(CNotificationCenter& notificationCenter);
									CColorRegistry(CNotificationCenter& notificationCenter,
											const CPreferences::Pref& pref);
									~CColorRegistry();

									// Instance methods
				CColorGroup&		registerColorGroup(OSType id, UInt32 displayIndex = 0);
				TArray<CColorGroup>	getColorGroups() const;

				CColorSet&			registerColorSetPreset(OSType id);
				TArray<CColorSet>	getColorSets(bool includeColorSetPresets) const;
				void				removeColorSet(const CColorSet& colorSet);

		const	CColorSet&			getCurrentColorSet() const;
				void				setAsCurrent(const CColorSet& colorSet);
				void				setCurrentColorSetColor(OSType colorGroupID, OSType colorID, const CColor& color);
		const	CColorSet&			createNewFromCurrentColorSet(const CString& name);
				void				updateFromCurrentColorSet(CColorSet& colorSet) const;
				OR<CColorSet>		getFirstMatchingColorsOfCurrentColorSet() const;

	// Properties
	private:
		CColorRegistryInternals*	mInternals;
};
