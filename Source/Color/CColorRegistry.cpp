//----------------------------------------------------------------------------------------------------------------------
//	CColorRegistry.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CColorRegistry.h"

#include "CNotificationCenter.h"
#include "CPreferences.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CColorGroup::Internals

class CColorGroup::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(OSType id, UInt32 displayIndex) :
			TReferenceCountableAutoDelete(), mID(id), mDisplayIndex(displayIndex)
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

class CColorSet::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const CString& name, OV<OSType> id = OV<OSType>()) :
			TReferenceCountableAutoDelete(),
					mName(name), mID(id), mColorsMap((SValue::OpaqueEqualsProc) CColor::areEqual)
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
				OSType			groupID = colorSetColorInfo.getOSType(CString(OSSTR("groupID")));
				OSType			colorID = colorSetColorInfo.getOSType(CString(OSSTR("colorID")));
				CDictionary		colorInfo = colorSetColorInfo.getDictionary(CString(OSSTR("color")));

		// Store
		UInt64	key = ((UInt64) groupID << 32) | colorID;
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
const CColor& CColorSet::getColor(OSType groupID, OSType colorID, const CColor& defaultColor) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	key = ((UInt64) groupID << 32) | colorID;
	OR<CColor>	color = mInternals->mColorsMap[key];

	return color.hasReference() ? *color : defaultColor;
}

//----------------------------------------------------------------------------------------------------------------------
void CColorSet::setColor(OSType groupID, OSType colorID, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	key = ((UInt64) groupID << 32) | colorID;

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
		UInt64	key = iterator->mKey.getUInt64();
		OSType	groupID = (OSType) (key >> 32);
		OSType	colorID = (OSType) (key & 0xFFFFFFFF);

		CDictionary	colorSetColorInfo;
		colorSetColorInfo.set(CString(OSSTR("groupID")), groupID);
		colorSetColorInfo.set(CString(OSSTR("colorID")), colorID);
		colorSetColorInfo.set(CString(OSSTR("color")), ((CColor*) iterator->mValue.getOpaque())->getInfo());

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
					Internals(OI<CPreferences::Pref> pref = OI<CPreferences::Pref>()) : mPref(pref) {}

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

		CImmediateNotificationCenter					mNotificationCenter;
		OI<CColorSet>									mCurrentColorSet;
		OI<CPreferences::Pref>							mPref;
		TNKeyConvertibleDictionary<OSType, CColorGroup>	mColorGroupMap;
		TNArray<CColorSet>								mColorSets;
		TNArray<CColorSet>								mColorSetPresets;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorRegistry

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::CColorRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::CColorRegistry(const CPreferences::Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(OI<CPreferences::Pref>(pref));

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
CNotificationCenter& CColorRegistry::getNotificationCenter() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mNotificationCenter;
}

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
		colorGroups += *((CColorGroup*) iterator->mValue.getOpaque());

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

	// Post notification
	mInternals->mNotificationCenter.post(mColorSetChangedNotificationName,
			CNotificationCenter::RSender<CColorRegistry>(*this));
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::setCurrentColorSetColor(OSType groupID, OSType colorID, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->getCurrentColorSet().setColor(groupID, colorID, color);

	// Save to prefs
	mInternals->writeToPrefs();

	CDictionary	info;
	info.set(CString(OSSTR("groupID")), groupID);
	info.set(CString(OSSTR("colorID")), colorID);
	info.set(CString(OSSTR("color")), &color);

	// Post notification
	mInternals->mNotificationCenter.post(mColorChangedNotificationName,
			CNotificationCenter::RSender<CColorRegistry>(*this), info);
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
		if (iterator->matchesColorsOf(mInternals->getCurrentColorSet()))
			// Match
			return OR<CColorSet>(*iterator);
	}

	// Examine user defined color sets
	for (TIteratorD<CColorSet> iterator = mInternals->mColorSets.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Check this color set
		if (iterator->matchesColorsOf(mInternals->getCurrentColorSet()))
			// Match
			return OR<CColorSet>(*iterator);
	}

	return OR<CColorSet>();
}

// MARK: Notifications

const	CString CColorRegistry::mColorSetChangedNotificationName(OSSTR("colorRegistryColorSetChanged"));
const	CString CColorRegistry::mColorChangedNotificationName(OSSTR("colorRegistryColorChanged"));

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry& CColorRegistry::notificatnGetMediaDocument(const OR<CNotificationCenter::Sender>& sender)
//----------------------------------------------------------------------------------------------------------------------
{
	return *((const CNotificationCenter::RSender<CColorRegistry>&) sender);
}

//----------------------------------------------------------------------------------------------------------------------
OSType CColorRegistry::notificationGetGroupID(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	return info.getOSType(CString(OSSTR("groupID")));
}

//----------------------------------------------------------------------------------------------------------------------
OSType CColorRegistry::notificationGetColorID(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	return info.getOSType(CString(OSSTR("colorID")));
}

//----------------------------------------------------------------------------------------------------------------------
const CColor& CColorRegistry::notificationGetColor(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	return *((CColor*) info.getValue(CString(OSSTR("color"))).getOpaque());
}
