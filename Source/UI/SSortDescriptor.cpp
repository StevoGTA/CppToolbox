//----------------------------------------------------------------------------------------------------------------------
//	SSortDescriptor.cpp			©2026 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SSortDescriptor.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SSortDescriptor

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CDictionary> SSortDescriptor::getInfos(const TArray<SSortDescriptor>& sortDescriptors)
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert
	TNArray<CDictionary>	infos;
	for (TArray<SSortDescriptor>::Iterator iterator = sortDescriptors.getIterator(); iterator; iterator++) {
		// Add info
		CDictionary	info;
		info.set(CString(OSSTR("identifier")), iterator->getIdentifier());
		info.set(CString(OSSTR("isAscending")), iterator->getIsAscending());
		infos += info;
	}

	return infos;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SSortDescriptor> SSortDescriptor::getSortDescriptors(const TArray<CDictionary>& infos)
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert
	TNArray<SSortDescriptor>	sortDescriptors;
	for (TArray<CDictionary>::Iterator iterator = infos.getIterator(); iterator; iterator++) {
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
