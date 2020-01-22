//----------------------------------------------------------------------------------------------------------------------
//	CColorRegistry.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CColorRegistry.h"

#include "CNotificationCenter.h"
#include "CPreferences.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

CString eColorRegistryColorChangedNotificationName("colorRegistryColorChangedNotification");

CString	eColorRegistryGroupIDKey("groupID");
CString	eColorRegistryColorIDKey("colorID");
CString	eColorRegistryColorKey("color");

static	CString	sColorGroupIDKey("groupID");
static	CString	sColorIDKey("colorID");
static	CString	sColorInfoKey = "color";
static	CString	sColorSetColorInfosKey("colors");
static	CString	sColorSetInfosKey("presets");
static	CString	sCurrentColorSetKey = "currentColorSet";
static	CString	sNameKey("name");

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorGroupInternals

class CColorGroupInternals {
	public:
		CColorGroupInternals(OSType id, UInt32 displayIndex) : mID(id), mDisplayIndex(displayIndex) {}
		~CColorGroupInternals() {}

		OSType					mID;
		UInt32					mDisplayIndex;
		TNumericArray<OSType>	mColorIDs;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorGroup

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColorGroup::CColorGroup(OSType id, UInt32 displayIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorGroupInternals(id, displayIndex);
}

//----------------------------------------------------------------------------------------------------------------------
CColorGroup::~CColorGroup()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
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
const TNumericArray<OSType>& CColorGroup::getColorIDs() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mColorIDs;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
ECompareResult CColorGroup::compareDisplayIndexes(CColorGroup* const colorGroup1, CColorGroup* const colorGroup2,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	return eCompare(colorGroup1->getDisplayIndex(), colorGroup2->getDisplayIndex());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorSetInternals

class CColorSetInternals {
	public:
		CColorSetInternals(const CString& name, OSType id) :
			mName(name), mID(id), mColorsMap((CDictionaryItemEqualsProc) CColor::areEqual)
			{}
		~CColorSetInternals() {}

	CString											mName;
	OSType											mID;
	TOwningKeyConvertibleDictionary<UInt64, CColor>	mColorsMap;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorSet

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColorSet::CColorSet(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorSetInternals(name, 0);
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet::CColorSet(OSType id)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorSetInternals(CString::mEmpty, id);
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet::CColorSet(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Retrieve name
	CString		name = info.getString(sNameKey);

	// Setup
	mInternals = new CColorSetInternals(name, 0);

	// Setup colors
	TArray<CDictionary>	colorSetColorInfos = info.getArrayOfDictionaries(sColorSetColorInfosKey);
	for (CArrayItemIndex j = 0; j < colorSetColorInfos.getCount(); j++) {
		// Get color set color info
		const	CDictionary&	colorSetColorInfo = colorSetColorInfos[j];
				OSType			colorGroupID = colorSetColorInfo.getOSType(sColorGroupIDKey);
				OSType			colorID = colorSetColorInfo.getOSType(sColorIDKey);
				CDictionary		colorInfo = colorSetColorInfo.getDictionary(sColorInfoKey);

		// Store
		UInt64	key = ((UInt64) colorGroupID << 32) | colorID;
		mInternals->mColorsMap.set(key, CColor(colorInfo));
	}
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet::~CColorSet()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CColorSet::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mName;
}

//----------------------------------------------------------------------------------------------------------------------
OSType CColorSet::getID() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mID;
}

//----------------------------------------------------------------------------------------------------------------------
const CColor& CColorSet::getColor(OSType colorGroupID, OSType colorID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	key = ((UInt64) colorGroupID << 32) | colorID;
	CColor*	color = mInternals->mColorsMap[key];

	return (color != nil) ? *color : CColor::mClear;
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

	info.set(sNameKey, mInternals->mName);

	TArray<CDictionary>	colorSetColorInfos;
	for (TIteratorS<SDictionaryItem> iterator = mInternals->mColorsMap.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Get info
		UInt64	key = iterator.getValue().mKey.getUInt64();
		OSType	colorGroupID = (OSType) (key >> 32);
		OSType	colorID = (OSType) (key & 0xFFFFFFFF);

		CDictionary	colorSetColorInfo;
		colorSetColorInfo.set(sColorGroupIDKey, colorGroupID);
		colorSetColorInfo.set(sColorIDKey, colorID);
		colorSetColorInfo.set(sColorInfoKey, ((CColor*) iterator.getValue().mValue.getItemRef())->getInfo());

		// Add to array
		colorSetColorInfos += colorSetColorInfo;
	}
	info.set(sColorSetColorInfosKey, colorSetColorInfos);

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
// MARK: - CColorRegistryInternals

class CColorRegistryInternals {
	public:
					CColorRegistryInternals(SPref* pref) :
						mCurrentColorSet(nil), mPref(pref), mColorGroupMap(), mColorSets(true), mColorSetPresets(true)
						{}
					~CColorRegistryInternals()
						{
							DisposeOf(mCurrentColorSet);
							DisposeOf(mPref);
							mColorSets.removeAll();
							mColorSetPresets.removeAll();
						}

			void	writeToPrefs()
						{
							// Check if have pref
							if (mPref != nil) {
								// Setup
								CDictionary	info;

								// Collect info
								TArray<CDictionary>	colorSetInfos;
								for (CArrayItemIndex i = 0; i < mColorSets.getCount(); i++)
									// Add info
									colorSetInfos += mColorSets[i]->getInfo();
								info.set(sColorSetInfosKey, colorSetInfos);

								info.set(sCurrentColorSetKey, mCurrentColorSet->getInfo());

								// Write
								CPreferences::set(*mPref, info);
							}
						}

		CColorSet&	getCurrentColorSet()
						{
							// Check if we have current color set
							if (mCurrentColorSet == nil) {
								// Create current color set
								mCurrentColorSet = new CColorSet(CString::mEmpty);

								// Check what to initialize it to
								if (!mColorSetPresets.isEmpty())
									// Use first preset
									mCurrentColorSet->setColorsFrom(*mColorSetPresets[0]);
								else if (!mColorSets.isEmpty())
									// Use first color set
									mCurrentColorSet->setColorsFrom(*mColorSets[0]);
							}

							return *mCurrentColorSet;
						}

		CColorSet*										mCurrentColorSet;
		SPref*											mPref;
		TKeyConvertibleDictionary<OSType, CColorGroup*>	mColorGroupMap;
		TPtrArray<CColorSet*>							mColorSets;
		TPtrArray<CColorSet*>							mColorSetPresets;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorRegistry

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::CColorRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorRegistryInternals(nil);
}

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::CColorRegistry(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CColorRegistryInternals(new SPref(pref));

	// Load from prefs
	CDictionary	info = CPreferences::getDictionary(pref);

	// Color sets
	TArray<CDictionary>	colorSetInfos = info.getArrayOfDictionaries(sColorSetInfosKey);
	for (CArrayItemIndex i = 0; i < colorSetInfos.getCount(); i++)
		// Add to color sets
		mInternals->mColorSets += new CColorSet(colorSetInfos[i]);

	// Current color set
	CDictionary	currentColorSetInfo = info.getDictionary(sCurrentColorSetKey);
	if (!currentColorSetInfo.isEmpty())
		// Setup current color set
		mInternals->mCurrentColorSet = new CColorSet(currentColorSetInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::~CColorRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CColorGroup& CColorRegistry::registerColorGroup(OSType id, UInt32 displayIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CColorGroup*	colorGroup = new CColorGroup(id, displayIndex);

	// Add
	mInternals->mColorGroupMap.set(id, colorGroup);

	return *colorGroup;
}

//----------------------------------------------------------------------------------------------------------------------
const TPtrArray<CColorGroup*> CColorRegistry::getColorGroups() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get color groups
	TPtrArray<CColorGroup*>	colorGroups = mInternals->mColorGroupMap.getValues();

	// Sort by display index
	colorGroups.sort(CColorGroup::compareDisplayIndexes);

	return colorGroups;
}

//----------------------------------------------------------------------------------------------------------------------
CColorSet& CColorRegistry::registerColorSetPreset(OSType id)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CColorSet*	colorSet = new CColorSet(id);

	// Add
	mInternals->mColorSetPresets += colorSet;

	return *colorSet;
}

//----------------------------------------------------------------------------------------------------------------------
const TPtrArray<CColorSet*> CColorRegistry::getColorSets(bool includeColorSetPresets) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TPtrArray<CColorSet*>	colorSets;
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
	for (CArrayItemIndex i = 0; i < mInternals->mColorSets.getCount(); i++) {
		// Get info
		CColorSet*	testColorSet = mInternals->mColorSets[i];

		// Check if
		if (*testColorSet == colorSet) {
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
void CColorRegistry::setColorSetAsCurrent(const CColorSet& colorSet)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->getCurrentColorSet().setColorsFrom(colorSet);

	// Save to prefs
	mInternals->writeToPrefs();

	// Iterate color groups
			CDictionary				info;
	const	TPtrArray<CColorGroup*>	colorGroups = getColorGroups();
	for (CArrayItemIndex i = 0; i < colorGroups.getCount(); i++) {
		// Get info
		const	CColorGroup*			colorGroup = colorGroups[i];
				OSType					colorGroupID = colorGroup->getID();
				TNumericArray<OSType>	colorIDs = colorGroup->getColorIDs();

		// Setup
		info.set(eColorRegistryGroupIDKey, colorGroupID);

		// Iterate color IDs
		for (CArrayItemIndex j = 0; j < colorIDs.getCount(); j++) {
			// Get info
					OSType	colorID = colorIDs[j];
			const	CColor&	color = mInternals->getCurrentColorSet().getColor(colorGroupID, colorID);

			// Setup
			info.set(eColorRegistryColorIDKey, colorID);
			info.set(eColorRegistryColorKey, &color);

			// Send notification
		CNotificationCenter::mStandard.send(eColorRegistryColorChangedNotificationName, this, info);
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
	info.set(eColorRegistryGroupIDKey, colorGroupID);
	info.set(eColorRegistryColorIDKey, colorID);
	info.set(eColorRegistryColorKey, &color);

	CNotificationCenter::mStandard.send(eColorRegistryColorChangedNotificationName, this, info);
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::createNewColorSetFromCurrentColorSet(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CColorSet*	colorSet = new CColorSet(name);
	colorSet->setColorsFrom(mInternals->getCurrentColorSet());

	// Add
	mInternals->mColorSets += colorSet;

	// Save to prefs
	mInternals->writeToPrefs();
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::updateColorSetFromCurrentColorSet(CColorSet& colorSet) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	colorSet.setColorsFrom(mInternals->getCurrentColorSet());

	// Save to prefs
	mInternals->writeToPrefs();
}

//----------------------------------------------------------------------------------------------------------------------
OR<CColorSet> CColorRegistry::getFirstColorSetMatchingColorsOfCurrentColorSet() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Examine preset color sets first
	for (TIteratorS<CColorSet*> iterator = mInternals->mColorSetPresets.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Check this color set
		if (iterator.getValue()->matchesColorsOf(mInternals->getCurrentColorSet()))
			// Match
			return OR<CColorSet>(*iterator.getValue());
	}

	// Examine user defined color sets
	for (TIteratorS<CColorSet*> iterator = mInternals->mColorSets.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Check this color set
		if (iterator.getValue()->matchesColorsOf(mInternals->getCurrentColorSet()))
			// Match
			return OR<CColorSet>(*iterator.getValue());
	}

	return OR<CColorSet>();
}
