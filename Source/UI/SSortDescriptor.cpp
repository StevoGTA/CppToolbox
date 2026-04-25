//----------------------------------------------------------------------------------------------------------------------
//	SSortDescriptor.cpp			©2026 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SSortDescriptor.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SSortDescriptor

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CDictionary> SSortDescriptor::getStorageInfos(const TArray<SSortDescriptor>& sortDescriptors)
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert
	TNArray<CDictionary>	sotrageInfos;
	for (TArray<SSortDescriptor>::Iterator iterator = sortDescriptors.getIterator(); iterator; iterator++) {
		// Add info
		CDictionary	info;
		info.set(CString(OSSTR("identifier")), iterator->getIdentifier());
		info.set(CString(OSSTR("isAscending")), iterator->getIsAscending());
		sotrageInfos += info;
	}

	return sotrageInfos;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SSortDescriptor> SSortDescriptor::getSortDescriptors(const TArray<CDictionary>& sotrageInfos)
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert
	TNArray<SSortDescriptor>	sortDescriptors;
	for (TArray<CDictionary>::Iterator iterator = sotrageInfos.getIterator(); iterator; iterator++) {
		// Get info
		OV<CString>	identifier = iterator->getOVString(CString(OSSTR("identifier")));
		OV<bool>	isAscending = iterator->getOVBool(CString(OSSTR("isAscending")));
		if (!identifier.hasValue() || !isAscending.hasValue())
			// Missing info
			continue;

		sortDescriptors += SSortDescriptor(*identifier, *isAscending);
	}

	return sortDescriptors;
}
