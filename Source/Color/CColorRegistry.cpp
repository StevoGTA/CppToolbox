//----------------------------------------------------------------------------------------------------------------------
//	CColorRegistry.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CColorRegistry.h"

#include "CNotificationCenter.h"
#include "CPreferences.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CColorGroup::Internals

class CColorGroup::Internals : public TReferenceCountable<Internals> {
	public:
		Internals(OSType id, UInt32 displayIndex) :
			TReferenceCountable(), mID(id), mDisplayIndex(displayIndex)
			{}

		OSType					mID;
		UInt32					mDisplayIndex;
		TNumberArray<OSType>	mColorIDs;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorGroup

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColorGroup::CColorGroup(OSType id, UInt32 displayIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(id, displayIndex);
}

//----------------------------------------------------------------------------------------------------------------------
CColorGroup::CColorGroup(const CColorGroup& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CColorGroup::~CColorGroup()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OSType CColorGroup::getID() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mID;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CColorGroup::getDisplayIndex() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDisplayIndex;
}

//----------------------------------------------------------------------------------------------------------------------
void CColorGroup::registerColor(OSType id)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mColorIDs += id;
}

//----------------------------------------------------------------------------------------------------------------------
const TNumberArray<OSType>& CColorGroup::getColorIDs() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mColorIDs;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CColorGroup::compareDisplayIndexes(const CColorGroup& colorGroup1, const CColorGroup& colorGroup2, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	return eCompare(colorGroup1.getDisplayIndex(), colorGroup2.getDisplayIndex());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorSet::Internals

class CColorSet::Internals : public TReferenceCountable<Internals> {
	public:
		Internals(const CString& name, OV<OSType> id = OV<OSType>()) :
			TReferenceCountable(), mName(name), mID(id), mColorsMap((SValue::OpaqueEqualsProc) CColor::areEqual)
			{}

	CString										mName;
	OV<OSType>									mID;
	TNKeyConvertibleDictionary<UInt64, CColor>	mColorsMap;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorSet

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColorSet::CColorSet(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(name);
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet::CColorSet(OSType id)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(CString::mEmpty, OV<OSType>(id));
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet::CColorSet(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Retrieve name
	CString		name = info.getString(CString(OSSTR("name")));

	// Setup
	mInternals = new Internals(name);

	// Setup colors
	TArray<CDictionary>	colorSetColorInfos = info.getArrayOfDictionaries(CString(OSSTR("colors")));
	for (CArray::ItemIndex j = 0; j < colorSetColorInfos.getCount(); j++) {
		// Get color set color info
		const	CDictionary&	colorSetColorInfo = colorSetColorInfos[j];
				OSType			colorGroupID = colorSetColorInfo.getOSType(CString(OSSTR("groupID")));
				OSType			colorID = colorSetColorInfo.getOSType(CString(OSSTR("colorID")));
				CDictionary		colorInfo = colorSetColorInfo.getDictionary(CString(OSSTR("color")));

		// Store
		UInt64	key = ((UInt64) colorGroupID << 32) | colorID;
		mInternals->mColorsMap.set(key, CColor(colorInfo));
	}
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet::CColorSet(const CColorSet& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet::~CColorSet()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CColorSet::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mName;
}

//----------------------------------------------------------------------------------------------------------------------
OV<OSType> CColorSet::getID() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mID;
}

//----------------------------------------------------------------------------------------------------------------------
const CColor& CColorSet::getColor(OSType colorGroupID, OSType colorID, const CColor& defaultColor) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	key = ((UInt64) colorGroupID << 32) | colorID;
	OR<CColor>	color = mInternals->mColorsMap[key];

	return color.hasReference() ? *color : defaultColor;
}

//----------------------------------------------------------------------------------------------------------------------
void CColorSet::setColor(OSType colorGroupID, OSType colorID, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	key = ((UInt64) colorGroupID << 32) | colorID;

	// Store
	mInternals->mColorsMap.set(key, color);
}

//----------------------------------------------------------------------------------------------------------------------
void CColorSet::setColorsFrom(const CColorSet& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mColorsMap = other.mInternals->mColorsMap;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CColorSet::getInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	info;

	info.set(CString(OSSTR("name")), mInternals->mName);

	TNArray<CDictionary>	colorSetColorInfos;
	for (TIteratorS<CDictionary::Item> iterator = mInternals->mColorsMap.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Get info
		UInt64	key = iterator.getValue().mKey.getUInt64();
		OSType	colorGroupID = (OSType) (key >> 32);
		OSType	colorID = (OSType) (key & 0xFFFFFFFF);

		CDictionary	colorSetColorInfo;
		colorSetColorInfo.set(CString(OSSTR("groupID")), colorGroupID);
		colorSetColorInfo.set(CString(OSSTR("colorID")), colorID);
		colorSetColorInfo.set(CString(OSSTR("color")), ((CColor*) iterator.getValue().mValue.getOpaque())->getInfo());

		// Add to array
		colorSetColorInfos += colorSetColorInfo;
	}
	info.set(CString(OSSTR("colors")), colorSetColorInfos);

	return info;
}

//----------------------------------------------------------------------------------------------------------------------
bool CColorSet::matchesColorsOf(const CColorSet& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mColorsMap == other.mInternals->mColorsMap;
}

//----------------------------------------------------------------------------------------------------------------------
bool CColorSet::operator==(const CColorSet& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return	(mInternals->mName == other.mInternals->mName) &&
			(mInternals->mID == other.mInternals->mID) &&
			(mInternals->mColorsMap == other.mInternals->mColorsMap);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorRegistry::Internals

class CColorRegistry::Internals {
	public:
					Internals(CNotificationCenter& notificationCenter,
							OI<CPreferences::Pref> pref = OI<CPreferences::Pref>()) :
						mNotificationCenter(notificationCenter), mPref(pref)
						{}

			void	writeToPrefs()
						{
							// Check if have pref
							if (mPref.hasInstance()) {
								// Setup
								CDictionary	info;

								// Collect info
								TNArray<CDictionary>	colorSetInfos;
								for (CArray::ItemIndex i = 0; i < mColorSets.getCount(); i++)
									// Add info
									colorSetInfos += mColorSets[i].getInfo();
								info.set(CString(OSSTR("presets")), colorSetInfos);

								if (mCurrentColorSet.hasInstance())
									// Add current color set
									info.set(CString(OSSTR("currentColorSet")), mCurrentColorSet->getInfo());

								// Write
								CPreferences::mDefault.set(*mPref, info);
							}
						}

		CColorSet&	getCurrentColorSet()
						{
							// Check if we have current color set
							if (!mCurrentColorSet.hasInstance()) {
								// Create current color set
								mCurrentColorSet = OI<CColorSet>(CColorSet(CString::mEmpty));

								// Check what to initialize it to
								if (!mColorSetPresets.isEmpty())
									// Use first preset
									mCurrentColorSet->setColorsFrom(mColorSetPresets[0]);
								else if (!mColorSets.isEmpty())
									// Use first color set
									mCurrentColorSet->setColorsFrom(mColorSets[0]);
							}

							return *mCurrentColorSet;
						}

		CNotificationCenter&							mNotificationCenter;
		OI<CColorSet>									mCurrentColorSet;
		OI<CPreferences::Pref>							mPref;
		TNKeyConvertibleDictionary<OSType, CColorGroup>	mColorGroupMap;
		TNArray<CColorSet>								mColorSets;
		TNArray<CColorSet>								mColorSetPresets;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorRegistry

// MARK: Notifications

const	CString CColorRegistry::mColorChangedNotificationName(OSSTR("colorRegistryColorChangedNotification"));

const	CString	CColorRegistry::mGroupIDKey(OSSTR("groupID"));
const	CString	CColorRegistry::mColorIDKey(OSSTR("colorID"));
const	CString	CColorRegistry::mColorKey(OSSTR("color"));


// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::CColorRegistry(CNotificationCenter& notificationCenter)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(notificationCenter);
}

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::CColorRegistry(CNotificationCenter& notificationCenter, const CPreferences::Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(notificationCenter, OI<CPreferences::Pref>(pref));

	// Load from prefs
	OV<CDictionary>	info = CPreferences::mDefault.getDictionary(pref);
	if (info.hasValue()) {
		// Color sets
		TArray<CDictionary>	colorSetInfos = info->getArrayOfDictionaries(CString(OSSTR("presets")));
		for (CArray::ItemIndex i = 0; i < colorSetInfos.getCount(); i++)
			// Add to color sets
			mInternals->mColorSets += CColorSet(colorSetInfos[i]);

		// Current color set
		CDictionary	currentColorSetInfo = info->getDictionary(CString(OSSTR("currentColorSet")));
		if (!currentColorSetInfo.isEmpty())
			// Setup current color set
			mInternals->mCurrentColorSet = OI<CColorSet>(CColorSet(currentColorSetInfo));
	}
}

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::~CColorRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CColorGroup& CColorRegistry::registerColorGroup(OSType id, UInt32 displayIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	mInternals->mColorGroupMap.set(id, CColorGroup(id, displayIndex));

	return *mInternals->mColorGroupMap[id];
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CColorGroup> CColorRegistry::getColorGroups() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get color groups
	TNArray<CColorGroup>	colorGroups;
	for (TIteratorS<CDictionary::Item> iterator = mInternals->mColorGroupMap.getIterator();
			iterator.hasValue(); iterator.advance())
		// Insert value
		colorGroups += *((CColorGroup*) iterator.getValue().mValue.getOpaque());

	// Sort by display index
	colorGroups.sort(CColorGroup::compareDisplayIndexes);

	return colorGroups;
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet& CColorRegistry::registerColorSetPreset(OSType id)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	mInternals->mColorSetPresets += CColorSet(id);

	return mInternals->mColorSetPresets.getLast();
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CColorSet> CColorRegistry::getColorSets(bool includeColorSetPresets) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CColorSet>	colorSets;
	if (includeColorSetPresets)
		colorSets += mInternals->mColorSetPresets;
	colorSets += mInternals->mColorSets;

	return colorSets;
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::removeColorSet(const CColorSet& colorSet)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all color sets
	for (CArray::ItemIndex i = 0; i < mInternals->mColorSets.getCount(); i++) {
		// Check if this one matches
		if (mInternals->mColorSets[i] == colorSet) {
			// Remove from array
			mInternals->mColorSets.removeAtIndex(i);

			// Done checking
			break;
		}
	}

	// Save to prefs
	mInternals->writeToPrefs();
}

//----------------------------------------------------------------------------------------------------------------------
const CColorSet& CColorRegistry::getCurrentColorSet() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getCurrentColorSet();
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::setAsCurrent(const CColorSet& colorSet)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->getCurrentColorSet().setColorsFrom(colorSet);

	// Save to prefs
	mInternals->writeToPrefs();

	// Iterate color groups
	CDictionary			info;
	TArray<CColorGroup>	colorGroups = getColorGroups();
	for (CArray::ItemIndex i = 0; i < colorGroups.getCount(); i++) {
		// Get info
		CColorGroup&			colorGroup = colorGroups[i];
		OSType					colorGroupID = colorGroup.getID();
		TNumberArray<OSType>	colorIDs = colorGroup.getColorIDs();

		// Setup
		info.set(mGroupIDKey, colorGroupID);

		// Iterate color IDs
		for (CArray::ItemIndex j = 0; j < colorIDs.getCount(); j++) {
			// Get info
					OSType	colorID = colorIDs[j];
			const	CColor&	color = mInternals->getCurrentColorSet().getColor(colorGroupID, colorID);

			// Setup
			info.set(mColorIDKey, colorID);
			info.set(mColorKey, &color);

			// Send notification
			mInternals->mNotificationCenter.queue(mColorChangedNotificationName, this, info);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::setCurrentColorSetColor(OSType colorGroupID, OSType colorID, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->getCurrentColorSet().setColor(colorGroupID, colorID, color);

	// Save to prefs
	mInternals->writeToPrefs();

	// Send notification
	CDictionary	info;
	info.set(mGroupIDKey, colorGroupID);
	info.set(mColorIDKey, colorID);
	info.set(mColorKey, &color);

//	mInternals->mNotificationCenter.queue(mColorChangedNotificationName, this, info);
}

//----------------------------------------------------------------------------------------------------------------------
const CColorSet& CColorRegistry::createNewFromCurrentColorSet(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CColorSet	colorSet(name);
	colorSet.setColorsFrom(mInternals->getCurrentColorSet());

	// Add
	mInternals->mColorSets += colorSet;

	// Save to prefs
	mInternals->writeToPrefs();

	return mInternals->mColorSets.getLast();
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::updateFromCurrentColorSet(CColorSet& colorSet) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	colorSet.setColorsFrom(mInternals->getCurrentColorSet());

	// Save to prefs
	mInternals->writeToPrefs();
}

//----------------------------------------------------------------------------------------------------------------------
OR<CColorSet> CColorRegistry::getFirstMatchingColorsOfCurrentColorSet() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Examine preset color sets first
	for (TIteratorD<CColorSet> iterator = mInternals->mColorSetPresets.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Check this color set
		if (iterator.getValue().matchesColorsOf(mInternals->getCurrentColorSet()))
			// Match
			return OR<CColorSet>(iterator.getValue());
	}

	// Examine user defined color sets
	for (TIteratorD<CColorSet> iterator = mInternals->mColorSets.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Check this color set
		if (iterator.getValue().matchesColorsOf(mInternals->getCurrentColorSet()))
			// Match
			return OR<CColorSet>(iterator.getValue());
	}

	return OR<CColorSet>();
}
