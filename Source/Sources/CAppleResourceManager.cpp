//----------------------------------------------------------------------------------------------------------------------
//	CAppleResourceManager.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAppleResourceManager.h"

#include "CByteReader.h"

//----------------------------------------------------------------------------------------------------------------------
// Info from https://github.com/uliwitness/ReClassicfication

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAppleResource

class CAppleResource {
	public:
		CAppleResource(UInt16 id, const OI<CString>& name, const CData& data) : mID(id), mName(name), mData(data) {}
		CAppleResource(const CAppleResource& other) : mID(other.mID), mName(other.mName), mData(other.mData) {}

	UInt16		mID;
	OI<CString>	mName;
	CData		mData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAppleResourceManagerInternals

class CAppleResourceManagerInternals {
	public:
		CAppleResourceManagerInternals(const TNDictionary<TNArray<CAppleResource> >& resourceMap) :
			mResourceMap(resourceMap), mUpdated(false)
			{}

		TNDictionary<TNArray<CAppleResource> >	mResourceMap;
		bool									mUpdated;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local data

static	CString	sErrorDomain(OSSTR("CAppleResourceManager"));
static	SError	sInvalidResourceData(sErrorDomain, 1, CString(OSSTR("Invalid Resource Data")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAppleResourceManager

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAppleResourceManager::CAppleResourceManager(const TNDictionary<TNArray<CAppleResource> >& resourceMap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CAppleResourceManagerInternals(resourceMap);
}

//----------------------------------------------------------------------------------------------------------------------
CAppleResourceManager::~CAppleResourceManager()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OR<CData> CAppleResourceManager::get(OSType resourceType, UInt16 resourceID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get array of resources by type
	OR<TNArray<CAppleResource> >	appleResources = mInternals->mResourceMap[CString(resourceType, true, false)];
	if (!appleResources.hasReference())
		return OR<CData>();

	// Search for resource by ID
	for (TIteratorD<CAppleResource> iterator = appleResources->getIterator(); iterator.hasValue(); iterator.advance()) {
		// Check ID
		if (iterator->mID == resourceID)
			// Found
			return OR<CData>(iterator->mData);
	}

	return OR<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<CString> CAppleResourceManager::getPascalString(OSType resourceType, UInt16 resourceID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get data
	OR<CData>	data = get(resourceType, resourceID);

	return data.hasReference() ?
			OI<CString>(
					new CString((const char*) data->getBytePtr() + 1, (CString::Length) (data->getByteCount() - 1),
							CString::kEncodingMacRoman)) :
			OI<CString>();
}

#if 0

//----------------------------------------------------------------------------------------------------------------------
void CAppleResourceManager::set(OSType resourceType, UInt16 resourceID, const CString& name, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
//	// Get array of resources by type
//	CString					key(resourceType, true, false);
//	TNArray<CAppleResource>	appleResources =
//									mInternals->mResourceMap.contains(key) ?
//											*mInternals->mResourceMap[key] : TNArray<CAppleResource>();
//
//	// Remove existing item
//	for (CArrayItemIndex i = 0; i < appleResources.getCount(); i++) {
//		// Check this apple resource
//		if (appleResources[i].mID == resourceID) {
//			// Found
//			appleResources.removeAtIndex(i);
//
//			break;
//		}
//	}
//
//	// Add new item
//	appleResources += CAppleResource(resourceID, name, data);
//
//	// Update map
//	mInternals->mResourceMap.set(key, appleResources);

	// We are updated
	mInternals->mUpdated = true;
}

//----------------------------------------------------------------------------------------------------------------------
void CAppleResourceManager::set(OSType resourceType, UInt16 resourceID, const CString& name,
		const CString& pascalString)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get length
	UInt8	length = (UInt8) pascalString.getLength();

	// Construct data
	CData	data;
	data.appendBytes(&length, sizeof(UInt8));
	data += pascalString.getData(CString::kEncodingMacRoman);

	// Add
	set(resourceType, resourceID, name, data);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAppleResourceManager::write()
//----------------------------------------------------------------------------------------------------------------------
{
//	// Setup
//	CData			dataData, typeListData, resourceListData, nameListData;
//	TSet<CString>	types = mInternals->mResourceMap.getKeys();
//	UInt16			uInt16Zero = 0;
//	UInt32			uInt32Zero = 0;
//
//	// Start type list data
//	SInt16	lastTypeIndex = EndianS16_NtoB((SInt16) types.getCount() - 1);
//	typeListData.appendBytes((const UInt8*) &lastTypeIndex, sizeof(SInt16));
//
//	// Iterate resource types
//	for (TIteratorS<CString> iterator = types.getIterator(); iterator.hasValue(); iterator.advance()) {
//		// Get info
//		CString&				type = iterator.getValue();
//		TArray<CAppleResource>	appleResources = *mInternals->mResourceMap[type];
//
//		// Type ID
//		OSType	typeID = EndianU32_NtoB(type.getOSType());
//		typeListData.appendBytes((const UInt8*) &typeID, sizeof(OSType));
//
//		// Last resource index
//		UInt16	lastResourceIndex = EndianU16_NtoB(appleResources.getCount() - 1);
//		typeListData.appendBytes((const UInt8*) &lastResourceIndex, sizeof(UInt16));
//
//		// Resource list offset
//		UInt16	resourceListOffset = EndianU16_NtoB(resourceListData.getByteCount() + 2 + 8 * types.getCount());
//		typeListData.appendBytes((const UInt8*) &resourceListOffset, sizeof(UInt16));
//
//		// Iterate resources for this type
//		for (CArrayItemIndex resourceIndex = 0; resourceIndex < appleResources.getCount(); resourceIndex++) {
//			// Get this apple resource
//			const	CAppleResource&	appleResource = appleResources[resourceIndex];
//
//			// ID
//			UInt16	resourceID = EndianU16_NtoB(appleResource.mID);
//			resourceListData.appendBytes((const UInt8*) &resourceID, sizeof(UInt16));
//
//			// Name
//			UInt8	nameLength = appleResource.mName.getLength();
//			if (nameLength > 0) {
//				// Have name
//				UInt16	nameOffset = EndianU16_NtoB(nameListData.getByteCount());
//				resourceListData.appendBytes((const UInt8*) &nameOffset, sizeof(UInt16));
//
//				nameListData.appendBytes((const UInt8*) &nameLength, sizeof(UInt8));
//				nameListData += appleResource.mName.getData(kStringEncodingMacRoman);
//			} else {
//				// No name
//				UInt16	nameOffset = EndianU16_NtoB(-1);
//				resourceListData.appendBytes((const UInt8*) &nameOffset, sizeof(UInt16));
//			}
//
//			// Attributes + data offset
//			UInt32	attributesAndDataOffset = EndianU32_NtoB((0 << 24) | dataData.getByteCount());
//			resourceListData.appendBytes((const UInt8*) &attributesAndDataOffset, sizeof(UInt32));
//
//			// Resource handle placeholder
//			resourceListData.appendBytes((const UInt8*) &uInt32Zero, sizeof(UInt32));
//
//			// Data
//			UInt32	dataByteCount = EndianU32_NtoB(appleResource.mData.getByteCount());
//			dataData.appendBytes((const UInt8*) &dataByteCount, sizeof(UInt32));
//			dataData += appleResource.mData;
//		}
//	}
//
//	// Prepare to write to file
//	UInt32	dataOffset = EndianU32_NtoB(256);
//	UInt32	mapOffset = EndianU32_NtoB(256 + dataData.getByteCount());
//	UInt32	dataByteCount = EndianU32_NtoB(dataData.getByteCount());
//	UInt32	mapSize =
//					EndianU32_NtoB(28 + typeListData.getByteCount() + resourceListData.getByteCount() +
//							nameListData.getByteCount());
//
//	// Update file contents
//	UError	error;
//	error = mInternals->mFile.open(kFileOpenModeWriteBufferedRemoveIfNotClosed);
//	ReturnErrorIfError(error);
//
//	// Header
//	error = mInternals->mFile.write(dataOffset);
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(mapOffset);
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(dataByteCount);
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(mapSize);
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.setPos(kFilePositionModeFromBeginning, 256);
//	ReturnErrorIfError(error);
//
//	// Data
//	error = mInternals->mFile.write(dataData);
//	ReturnErrorIfError(error);
//
//	// Map
//	error = mInternals->mFile.write(dataOffset);	// File header copy
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(mapOffset);
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(dataByteCount);
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(mapSize);
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(uInt32Zero);	// Next resource map placeholder
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(uInt16Zero);	// File ref num placeholder
//	ReturnErrorIfError(error);
//	error = mInternals->mFile.write(uInt16Zero);	// File attributes
//	ReturnErrorIfError(error);
//
//	UInt16	typeListOffset = EndianU16_NtoB(28);
//	error = mInternals->mFile.write(typeListOffset);
//	ReturnErrorIfError(error);
//
//	UInt16	nameListOffset = EndianU16_NtoB(28 + typeListData.getByteCount() + resourceListData.getByteCount());
//	error = mInternals->mFile.write(nameListOffset);
//	ReturnErrorIfError(error);
//
//	// Type list
//	error = mInternals->mFile.write(typeListData);
//	ReturnErrorIfError(error);
//
//	// Resource list
//	error = mInternals->mFile.write(resourceListData);
//	ReturnErrorIfError(error);
//
//	// Name list
//	error = mInternals->mFile.write(nameListData);
//	ReturnErrorIfError(error);

	return OI<SError>();
}

#endif

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAppleResourceManager> CAppleResourceManager::from(const I<CRandomAccessDataSource>& randomAccessDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CByteReader	byteReader(randomAccessDataSource, true);
	OI<SError>	error;

	// Read header
	if (byteReader.getByteCount() < (sizeof(UInt32) * 4))
		// Not big enough to read header
		return TIResult<CAppleResourceManager>(sInvalidResourceData);

	TVResult<UInt32>	resourceDataOffset = byteReader.readUInt32();
	ReturnValueIfResultError(resourceDataOffset, TIResult<CAppleResourceManager>(resourceDataOffset.getError()));

	TVResult<UInt32>	resourceMapOffset = byteReader.readUInt32();
	ReturnValueIfResultError(resourceMapOffset, TIResult<CAppleResourceManager>(resourceMapOffset.getError()));

	TVResult<UInt32>	resourceDataByteCount = byteReader.readUInt32();
	ReturnValueIfResultError(resourceDataByteCount, TIResult<CAppleResourceManager>(resourceDataByteCount.getError()));

	TVResult<UInt32>	resourceMapByteCount = byteReader.readUInt32();
	ReturnValueIfResultError(resourceMapByteCount, TIResult<CAppleResourceManager>(resourceMapByteCount.getError()));

	if ((byteReader.getByteCount() < (*resourceDataOffset + *resourceDataByteCount)) ||
			(byteReader.getByteCount() < (*resourceMapOffset + *resourceMapByteCount)))
		// Not big enough to read contents
		return TIResult<CAppleResourceManager>(sInvalidResourceData);

	// Read type list
	error =
			byteReader.setPos(CByteReader::kPositionFromBeginning,
					*resourceMapOffset +
					16 +					// Resource file header copy
					4 +						// Next resource map placeholder
					2);						// File ref num placeholder
	ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));

	TVResult<UInt16>	resourceFileAttributes = byteReader.readUInt16();
	ReturnValueIfResultError(resourceFileAttributes, TIResult<CAppleResourceManager>(resourceFileAttributes.getError()));

	TVResult<UInt16>	typeListOffset = byteReader.readUInt16();
	ReturnValueIfResultError(typeListOffset, TIResult<CAppleResourceManager>(typeListOffset.getError()));

	TVResult<UInt16>	nameListOffset = byteReader.readUInt16();
	ReturnValueIfResultError(nameListOffset, TIResult<CAppleResourceManager>(nameListOffset.getError()));

	// Seek to resource infos
	error = byteReader.setPos(CByteReader::kPositionFromBeginning, *resourceMapOffset + *typeListOffset);
	ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));

	TVResult<SInt16>	lastTypeIndex = byteReader.readSInt16();
	ReturnValueIfResultError(lastTypeIndex, TIResult<CAppleResourceManager>(lastTypeIndex.getError()));

	TNDictionary<TNArray<CAppleResource> >	resourceMap;
	for (SInt16 typeIndex = 0; typeIndex <= *lastTypeIndex; typeIndex++) {
		// Read resource info
		TVResult<OSType>	resourceType = byteReader.readOSType();
		ReturnValueIfResultError(resourceType, TIResult<CAppleResourceManager>(resourceType.getError()));

		TVResult<UInt16>	lastResourceIndex = byteReader.readUInt16();
		ReturnValueIfResultError(lastResourceIndex, TIResult<CAppleResourceManager>(lastResourceIndex.getError()));

		TVResult<UInt16>	refListOffset = byteReader.readUInt16();
		ReturnValueIfResultError(refListOffset, TIResult<CAppleResourceManager>(refListOffset.getError()));

		// Note position
		UInt64	resourceTypeOffset = byteReader.getPos();

		// Seek to resources
		error =
				byteReader.setPos(CByteReader::kPositionFromBeginning,
						*resourceMapOffset + *typeListOffset + *refListOffset);
		ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));

		// Read resources
		TNArray<CAppleResource>	appleResources;
		for (UInt16 resourceIndex = 0; resourceIndex <= *lastResourceIndex; resourceIndex++) {
			// Read resource info
			TVResult<UInt16>	resourceID = byteReader.readUInt16();
			ReturnValueIfResultError(resourceID, TIResult<CAppleResourceManager>(resourceID.getError()));

			TVResult<SInt16>	nameOffset = byteReader.readSInt16();
			ReturnValueIfResultError(nameOffset, TIResult<CAppleResourceManager>(nameOffset.getError()));

			TVResult<UInt32>	attributesAndDataOffset = byteReader.readUInt32();
			ReturnValueIfResultError(attributesAndDataOffset,
					TIResult<CAppleResourceManager>(attributesAndDataOffset.getError()));

			UInt8	resourceAttributes = (*attributesAndDataOffset) >> 24;	(void) resourceAttributes;
			UInt32	dataOffset = (*attributesAndDataOffset) & 0x00FFFFFF;

			error = byteReader.setPos(CByteReader::kPositionFromCurrent, 4);	// Resource Handle placeholder
			ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));

			// Note position
			UInt64	innerOldOffset = byteReader.getPos();

			// Seek to resource data
			error = byteReader.setPos(CByteReader::kPositionFromBeginning, *resourceDataOffset + dataOffset);
			ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));

			// Read resource data
			TVResult<UInt32>	dataLength = byteReader.readUInt32();
			ReturnValueIfResultError(dataLength, TIResult<CAppleResourceManager>(dataLength.getError()));

			TIResult<CData>	resourceData = byteReader.readData(*dataLength);
			ReturnValueIfResultError(resourceData, TIResult<CAppleResourceManager>(resourceData.getError()));

			OI<CString>	name;
			if (*nameOffset != -1) {
				// Seek to name
				error =
						byteReader.setPos(CByteReader::kPositionFromBeginning,
								*resourceMapOffset + *nameListOffset + *nameOffset);
				ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));

				TVResult<UInt8>	nameByteCount = byteReader.readUInt8();
				ReturnValueIfResultError(nameByteCount, TIResult<CAppleResourceManager>(nameByteCount.getError()));

				// Check if actually have any bytes for name
				if (*nameByteCount > 0) {
					// Read name data
					TIResult<CData>	nameData = byteReader.readData(*nameByteCount);
					ReturnValueIfResultError(nameData, TIResult<CAppleResourceManager>(nameData.getError()));

					// Store name
					name = OI<CString>(new CString(*nameData, CString::kEncodingMacRoman));
				}
			}

			// Add to array
			appleResources += CAppleResource(*resourceID, name, *resourceData);

			// Seek to next resource position
			error = byteReader.setPos(CByteReader::kPositionFromBeginning, innerOldOffset);
			ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));
		}

		// Store resources for this resource type
		resourceMap.set(CString(*resourceType, true, false), appleResources);

		// Seek to next resource type
		error = byteReader.setPos(CByteReader::kPositionFromBeginning, resourceTypeOffset);
		ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));
	}

	return TIResult<CAppleResourceManager>(OI<CAppleResourceManager>(new CAppleResourceManager(resourceMap)));
}