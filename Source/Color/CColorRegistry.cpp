//----------------------------------------------------------------------------------------------------------------------
//	CColorRegistry.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CColorRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CColorRegistry::Internals

class CColorRegistry::Internals {
	// ColorByGroupEntryIDs
	public:
		struct ColorByGroupEntryIDs : public TNKeyConvertibleDictionary<UInt64, CColor> {
			// Methods
			public:
				// Lifecycle methods
				ColorByGroupEntryIDs() :
					TNKeyConvertibleDictionary<UInt64, CColor>((SValue::OpaqueEqualsProc) CColor::areEqual)
					{}
		};

	// GroupInfo
	public:
		struct GroupInfo {
			// Methods
			public:
				// Lifecycle methods
				GroupInfo(const Group& group, const bool& isEnabled) :
					mGroup(group), mIsEnabled(isEnabled)
					{}

			// Properties
			public:
						Group					mGroup;
				const	bool&					mIsEnabled;
						TNArray<Group::Entry>	mEntries;
		};

	// PresetInfo
	public:
		struct PresetInfo {
			// Methods
			public:
				// Lifecycle methods
				PresetInfo(const Preset& preset,
						const ColorByGroupEntryIDs& colorByGroupEntryIDs = ColorByGroupEntryIDs()) :
					mPreset(preset), mColorByGroupEntryIDs(colorByGroupEntryIDs)
					{}

			// Properties
			public:
				Preset					mPreset;
				ColorByGroupEntryIDs	mColorByGroupEntryIDs;
		};

	// Methods
	public:
										// Instance methods
				OR<GroupInfo>			getGroupInfo(OSType groupID) const
											{
												// Iterate Group infos
												for (TArray<GroupInfo>::Iterator iterator = mGroupInfos.getIterator();
														iterator; iterator++) {
													// Check if this is the Group
													if (iterator->mGroup.getID() == groupID)
														// Found
														return OR<GroupInfo>(*iterator);
												}

												return OR<GroupInfo>();
											}
				OR<PresetInfo>			getPresetInfoMatchingCurrentColors(const TDictionary<PresetInfo>& presetInfos)
												const
											{
												// Iterate Preset infos
												for (TDictionary<PresetInfo>::ValueIterator iterator =
																presetInfos.getValueIterator();
														iterator; iterator++) {
													// Check if this one matches
													if (iterator->mColorByGroupEntryIDs == mCurrentColorByGroupEntryIDs)
														// Match
														return OR<PresetInfo>(*iterator);
												}

												return OR<PresetInfo>();
											}
				ColorByGroupEntryIDs	getColorByGroupEntryIDs(const TArray<CDictionary>& colorInfos) const
											{
												// Collect saved colors
												ColorByGroupEntryIDs	savedColorByGroupEntryIDs;
												for (TArray<CDictionary>::Iterator iterator = colorInfos.getIterator();
														iterator; iterator++)
													// Add color
													savedColorByGroupEntryIDs.set(
															((UInt64) iterator->getOSType(CString(OSSTR("groupID"))) <<
																			32) |
																	iterator->getOSType(CString(OSSTR("entryID"))),
															CColor(iterator->getDictionary(CString(OSSTR("color")))));

												// Compose a color for every Entry - the saved color, otherwise the
												//	first built-in Preset's color
												const	OR<PresetInfo>			firstBuiltInPresetInfo =
																						mFirstBuiltInPresetID
																										.hasValue() ?
																								mBuiltInPresetInfoByID[
																										*mFirstBuiltInPresetID] :
																								OR<PresetInfo>();
														ColorByGroupEntryIDs	colorByGroupEntryIDs;
												for (TArray<GroupInfo>::Iterator groupInfoIterator =
																mGroupInfos.getIterator();
														groupInfoIterator; groupInfoIterator++) {
													// Iterate Entries
													for (TArray<Group::Entry>::Iterator entryIterator =
																	groupInfoIterator->mEntries.getIterator();
															entryIterator; entryIterator++) {
														// Get color
																UInt64		key =
																					((UInt64) groupInfoIterator->
																									mGroup.getID() <<
																											32) |
																							entryIterator->getID();
														const	OR<CColor>	savedColor = savedColorByGroupEntryIDs[key];
														const	OR<CColor>	color =
																					(savedColor.hasReference() ||
																							!firstBuiltInPresetInfo
																									.hasReference()) ?
																						savedColor :
																						firstBuiltInPresetInfo->
																								mColorByGroupEntryIDs[
																										key];

														// Check if have color
														if (color.hasReference())
															// Add color
															colorByGroupEntryIDs.set(key, *color);
													}
												}

												return colorByGroupEntryIDs;
											}
				void					writeToPrefs()
											{
												// Compose User Preset infos
												TNArray<CDictionary>	presetInfos;
												for (TDictionary<PresetInfo>::ValueIterator iterator =
																mUserPresetInfoByUniqueID.getValueIterator();
														iterator; iterator++) {
													// Compose info
													CDictionary	presetInfo;
													presetInfo.set(CString(OSSTR("uniqueID")),
															iterator->mPreset.getUniqueID());
													presetInfo.set(CString(OSSTR("name")), iterator->mPreset.getName());
													presetInfo.set(CString(OSSTR("colors")),
															getColorInfos(iterator->mColorByGroupEntryIDs));

													// Add info
													presetInfos += presetInfo;
												}

												// Compose info
												CDictionary		info;
												OR<PresetInfo>	builtInPresetInfo =
																		getPresetInfoMatchingCurrentColors(
																				mBuiltInPresetInfoByID);
												info.set(CString(OSSTR("presets")), presetInfos);
												info.set(CString(OSSTR("currentColors")),
														getColorInfos(mCurrentColorByGroupEntryIDs));
												if (builtInPresetInfo.hasReference())
													// Current colors match a built-in Preset
													info.set(CString(OSSTR("currentBuiltInPresetID")),
															*builtInPresetInfo->mPreset.getID());

												// Write
												CPreferences::shared().set(*mPref, info);
											}

										// Class methods
		static	TNArray<CDictionary>	getColorInfos(const ColorByGroupEntryIDs& colorByGroupEntryIDs)
											{
												// Iterate colors
												TNArray<CDictionary>	colorInfos;
												for (ColorByGroupEntryIDs::Iterator iterator =
																colorByGroupEntryIDs.getIterator();
														iterator; iterator++) {
													// Compose info
													CDictionary	colorInfo;
													colorInfo.set(CString(OSSTR("groupID")),
															(OSType) (iterator.getKey() >> 32));
													colorInfo.set(CString(OSSTR("entryID")),
															(OSType) (iterator.getKey() & 0xFFFFFFFF));
													colorInfo.set(CString(OSSTR("color")),
															iterator.getValue().getStorageInfo());

													// Add info
													colorInfos += colorInfo;
												}

												return colorInfos;
											}
		static	bool					compareNames(const Preset& preset1, const Preset& preset2, void* userData)
											{ return preset1.getName().compareTo(preset2.getName(),
													CString::kCompareToOptionsSortDefault); }

	// Properties
	public:
		CImmediateNotificationCenter					mNotificationCenter;
		TNArray<GroupInfo>								mGroupInfos;
		TNKeyConvertibleDictionary<OSType, PresetInfo>	mBuiltInPresetInfoByID;
		OV<OSType>										mFirstBuiltInPresetID;
		TNDictionary<PresetInfo>						mUserPresetInfoByUniqueID;
		ColorByGroupEntryIDs							mCurrentColorByGroupEntryIDs;
		OI<CPreferences::Pref>							mPref;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorRegistry

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::CColorRegistry() : mInternals(new Internals())
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry::~CColorRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::noteGroup(OSType groupID, const CString& name, const bool& isEnabled)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(mInternals->mPref.hasInstance());
	AssertFailIf(mInternals->getGroupInfo(groupID).hasReference());

	// Add
	mInternals->mGroupInfos += Internals::GroupInfo(Group(groupID, name), isEnabled);
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::noteGroupEntry(OSType groupID, OSType entryID, const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<Internals::GroupInfo>	groupInfo = mInternals->getGroupInfo(groupID);

	// Validate
	AssertFailIf(mInternals->mPref.hasInstance());
	AssertFailIf(!groupInfo.hasReference());
	for (TArray<Group::Entry>::Iterator iterator = groupInfo->mEntries.getIterator(); iterator; iterator++)
		// Check if this Entry is already noted
		AssertFailIf(iterator->getID() == entryID);

	// Add
	groupInfo->mEntries += Group::Entry(entryID, name);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CColorRegistry::Group> CColorRegistry::getEnabledGroups() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());

	// Collect enabled Groups
	TNArray<Group>	groups;
	for (TArray<Internals::GroupInfo>::Iterator iterator = mInternals->mGroupInfos.getIterator(); iterator;
			iterator++) {
		// Check if enabled
		if (iterator->mIsEnabled)
			// Add
			groups += iterator->mGroup;
	}

	return groups;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CColorRegistry::Group::Entry> CColorRegistry::getGroupEntries(const Group& group) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<Internals::GroupInfo>	groupInfo = mInternals->getGroupInfo(group.getID());

	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());
	AssertFailIf(!groupInfo.hasReference());

	return groupInfo->mEntries;
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::notePreset(OSType presetID, const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(mInternals->mPref.hasInstance());
	AssertFailIf(mInternals->mBuiltInPresetInfoByID[presetID].hasReference());

	// Add
	mInternals->mBuiltInPresetInfoByID.set(presetID, Internals::PresetInfo(Preset(presetID, name)));
	if (!mInternals->mFirstBuiltInPresetID.hasValue())
		// First built-in Preset
		mInternals->mFirstBuiltInPresetID = presetID;
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::setColor(OSType presetID, OSType groupID, OSType entryID, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(mInternals->mPref.hasInstance());

	// Store
	mInternals->mBuiltInPresetInfoByID[presetID]->mColorByGroupEntryIDs.set(((UInt64) groupID << 32) | entryID,
			color);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CColorRegistry::Preset> CColorRegistry::getPresets() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());

	// Collect Presets
	TNArray<Preset>	presets;
	for (TKeyConvertibleDictionary<OSType, Internals::PresetInfo>::ValueIterator iterator =
					mInternals->mBuiltInPresetInfoByID.getValueIterator();
			iterator; iterator++)
		// Add
		presets += iterator->mPreset;
	for (TDictionary<Internals::PresetInfo>::ValueIterator iterator =
					mInternals->mUserPresetInfoByUniqueID.getValueIterator();
			iterator; iterator++)
		// Add
		presets += iterator->mPreset;

	return presets.sort(Internals::compareNames);
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::usePreset(const Preset& preset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());

	// Update
	mInternals->mCurrentColorByGroupEntryIDs =
			preset.getID().hasValue() ?
					mInternals->mBuiltInPresetInfoByID[*preset.getID()]->mColorByGroupEntryIDs :
					mInternals->mUserPresetInfoByUniqueID[preset.getUniqueID()]->mColorByGroupEntryIDs;

	// Save to prefs
	mInternals->writeToPrefs();

	// Post notification
	mInternals->mNotificationCenter.post(mColorsChangedNotificationName,
			CNotificationCenter::RSender<CColorRegistry>(*this));
}

//----------------------------------------------------------------------------------------------------------------------
OV<CColorRegistry::Preset> CColorRegistry::getPresetMatchingCurrentColors() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());

	// Check built-in Presets
	OR<Internals::PresetInfo>	builtInPresetInfo =
										mInternals->getPresetInfoMatchingCurrentColors(
												mInternals->mBuiltInPresetInfoByID);
	if (builtInPresetInfo.hasReference())
		// Match
		return OV<Preset>(builtInPresetInfo->mPreset);

	// Check the User's Presets
	OR<Internals::PresetInfo>	userPresetInfo =
										mInternals->getPresetInfoMatchingCurrentColors(
												mInternals->mUserPresetInfoByUniqueID);

	return userPresetInfo.hasReference() ? OV<Preset>(userPresetInfo->mPreset) : OV<Preset>();
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::createPresetFromCurrentColors(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());

	// Add
	Preset	preset(name);
	mInternals->mUserPresetInfoByUniqueID.set(preset.getUniqueID(),
			Internals::PresetInfo(preset, mInternals->mCurrentColorByGroupEntryIDs));

	// Save to prefs
	mInternals->writeToPrefs();
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::updatePresetFromCurrentColors(const Preset& preset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());
	AssertFailIf(preset.getID().hasValue());

	// Update
	mInternals->mUserPresetInfoByUniqueID[preset.getUniqueID()]->mColorByGroupEntryIDs =
			mInternals->mCurrentColorByGroupEntryIDs;

	// Save to prefs
	mInternals->writeToPrefs();
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::removePreset(const Preset& preset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());
	AssertFailIf(preset.getID().hasValue());

	// Remove
	mInternals->mUserPresetInfoByUniqueID.remove(preset.getUniqueID());

	// Save to prefs
	mInternals->writeToPrefs();
}

//----------------------------------------------------------------------------------------------------------------------
const CColor& CColorRegistry::getColor(OSType groupID, OSType entryID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());

	// Get color
	const	OR<CColor>	color = mInternals->mCurrentColorByGroupEntryIDs[((UInt64) groupID << 32) | entryID];

	return color.hasReference() ? *color : CColor::mClear;
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::setColor(OSType groupID, OSType entryID, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(!mInternals->mPref.hasInstance());

	// Setup - keep the existing transparency; only the color itself changes
	CColor::RGBValues	rgbValues = color.getRGBValues();
	CColor				updatedColor(
								CColor::RGBValues(rgbValues.getRed(), rgbValues.getGreen(), rgbValues.getBlue(),
										getColor(groupID, entryID).getRGBValues().getAlpha()));

	// Update
	mInternals->mCurrentColorByGroupEntryIDs.set(((UInt64) groupID << 32) | entryID, updatedColor);

	// Save to prefs
	mInternals->writeToPrefs();

	// Post notification (info holds a pointer to the color, which is only valid while posting - the notification
	//	center is immediate)
	CDictionary	info;
	info.set(CString(OSSTR("groupID")), groupID);
	info.set(CString(OSSTR("entryID")), entryID);
	info.set(CString(OSSTR("color")), &updatedColor);
	mInternals->mNotificationCenter.post(mColorChangedNotificationName,
			CNotificationCenter::RSender<CColorRegistry>(*this), info);
}

//----------------------------------------------------------------------------------------------------------------------
CNotificationCenter& CColorRegistry::getNotificationCenter() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mNotificationCenter;
}

//----------------------------------------------------------------------------------------------------------------------
void CColorRegistry::usePersistence(const CPreferences::Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	AssertFailIf(mInternals->mPref.hasInstance());

	// Store
	mInternals->mPref = OI<CPreferences::Pref>(pref);

	// Setup
	CDictionary	info = CPreferences::shared().getDictionary(pref).getValue(CDictionary::mEmpty);

	// Restore the User's Presets
	const	TArray<CDictionary>&	presetInfos = info.getArrayOfDictionaries(CString(OSSTR("presets")));
	for (TArray<CDictionary>::Iterator iterator = presetInfos.getIterator(); iterator; iterator++) {
		// Add
		Preset	preset(iterator->getString(CString(OSSTR("uniqueID"))), iterator->getString(CString(OSSTR("name"))));
		mInternals->mUserPresetInfoByUniqueID.set(preset.getUniqueID(),
				Internals::PresetInfo(preset,
						mInternals->getColorByGroupEntryIDs(
								iterator->getArrayOfDictionaries(CString(OSSTR("colors"))))));
	}

	// Restore current colors - from the built-in Preset they matched when saved, so that Preset's colors from this
	//	release are used, otherwise the saved colors (with no saved colors, the first built-in Preset's colors)
	OR<Internals::PresetInfo>	builtInPresetInfo =
										mInternals->mBuiltInPresetInfoByID[
												info.getOSType(CString(OSSTR("currentBuiltInPresetID")))];
	mInternals->mCurrentColorByGroupEntryIDs =
			builtInPresetInfo.hasReference() ?
					builtInPresetInfo->mColorByGroupEntryIDs :
					mInternals->getColorByGroupEntryIDs(
							info.getArrayOfDictionaries(CString(OSSTR("currentColors"))));
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CColorRegistry& CColorRegistry::shared()
//----------------------------------------------------------------------------------------------------------------------
{
	// Local data
	static	CColorRegistry	sColorRegistry;

	return sColorRegistry;
}

// MARK: Notifications

const	CString	CColorRegistry::mColorsChangedNotificationName(OSSTR("colorRegistryColorsChanged"));
const	CString	CColorRegistry::mColorChangedNotificationName(OSSTR("colorRegistryColorChanged"));

//----------------------------------------------------------------------------------------------------------------------
OSType CColorRegistry::notificationGetGroupID(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	return info.getOSType(CString(OSSTR("groupID")));
}

//----------------------------------------------------------------------------------------------------------------------
OSType CColorRegistry::notificationGetGroupEntryID(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	return info.getOSType(CString(OSSTR("entryID")));
}

//----------------------------------------------------------------------------------------------------------------------
const CColor& CColorRegistry::notificationGetColor(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	return *((const CColor*) info.getValue(CString(OSSTR("color"))).getOpaque());
}
