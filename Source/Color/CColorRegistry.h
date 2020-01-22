//----------------------------------------------------------------------------------------------------------------------
//	CColorRegistry.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "CPreferences.h"
#include "CString.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Notifications

/*
	Sent when a color has been changed
		senderRef is CColorRegistry
		info contains the following keys:
			eColorRegistryGroupIDKey
			eColorRegistryColorIDKey
			eColorRegistryColorKey
*/
extern	CString eColorRegistryColorChangedNotificationName;

extern	CString	eColorRegistryGroupIDKey;	// OSType
extern	CString	eColorRegistryColorIDKey;	// OSType
extern	CString	eColorRegistryColorKey;		// CColor*

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorGroup

class CColorGroupInternals;
class CColorGroup {
	// Methods
	public:
												// Lifecycle methods
												CColorGroup(OSType id, UInt32 displayIndex = 0);
												~CColorGroup();

												// Instance methods
						OSType					getID() const;
						UInt32					getDisplayIndex() const;

						void					registerColor(OSType id);
				const	TNumericArray<OSType>&	getColorIDs() const;

												// Class methods
		static	ECompareResult					compareDisplayIndexes(CColorGroup* const colorGroup1,
														CColorGroup* const colorGroup2, void* userData);

	// Properties
	private:
		CColorGroupInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorSet

const	OSType	kColorSetNonPresetID = 0;

class CColorSetInternals;
class CColorSet {
	// Methods
	public:
									// Lifecycle methods
									CColorSet(const CString& name);	// Can modify
									CColorSet(OSType id);			// Cannot modify
									CColorSet(const CDictionary& info);
		virtual						~CColorSet();

									// Instance methods
				const	CString&	getName() const;
						OSType		getID() const;
						bool		getCanModify() const
										{ return getID() == kColorSetNonPresetID; }

				const	CColor&		getColor(OSType colorGroupID, OSType colorID) const;
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
	// Methods
	public:
										// Lifecycle methods
										CColorRegistry();
										CColorRegistry(const SPref& pref);
										~CColorRegistry();

										// Instance methods
				CColorGroup&			registerColorGroup(OSType id, UInt32 displayIndex = 0);
		const	TPtrArray<CColorGroup*>	getColorGroups() const;

				CColorSet&				registerColorSetPreset(OSType id);
		const	TPtrArray<CColorSet*>	getColorSets(bool includeColorSetPresets) const;
				void					removeColorSet(const CColorSet& colorSet);

		const	CColorSet&				getCurrentColorSet() const;
				void					setColorSetAsCurrent(const CColorSet& colorSet);
				void					setCurrentColorSetColor(OSType colorGroupID, OSType colorID,
												const CColor& color);
				void					createNewColorSetFromCurrentColorSet(const CString& name);
				void					updateColorSetFromCurrentColorSet(CColorSet& colorSet) const;
				OR<CColorSet>			getFirstColorSetMatchingColorsOfCurrentColorSet() const;

	// Properties
	private:
		CColorRegistryInternals*	mInternals;
};
