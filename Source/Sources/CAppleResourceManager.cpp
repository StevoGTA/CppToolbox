//----------------------------------------------------------------------------------------------------------------------
//	CAppleResourceManager.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAppleResourceManager.h"

#include "CByteReader.h"

//----------------------------------------------------------------------------------------------------------------------
// Info from https://github.com/uliwitness/ReClassicfication

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CAppleResourceManager"));
static	SError	sInvalidResourceData(sErrorDomain, 1, CString(OSSTR("Invalid Resource Data")));
static	SError	sInvalidResourceName(sErrorDomain, 2, CString(OSSTR("Invalid Resource Name")));
static	SError	sInvalidResourcePayload(sErrorDomain, 3, CString(OSSTR("Invalid Resource Payload")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAppleResourceManager::Resource

class CAppleResourceManager::Resource {
	public:
		Resource(UInt16 id, const OV<CString>& name, const CData& data) : mID(id), mName(name), mData(data) {}
		Resource(const Resource& other) : mID(other.mID), mName(other.mName), mData(other.mData) {}

	UInt16		mID;
	OV<CString>	mName;
	CData		mData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAppleResourceManager::Internals

class CAppleResourceManager::Internals {
	public:
		Internals() : mUpdated(false) {}
		Internals(const TNDictionary<TNArray<Resource> >& resourceMap) : mResourceMap(resourceMap), mUpdated(false) {}

		TNDictionary<TNArray<Resource> >	mResourceMap;
		bool								mUpdated;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAppleResourceManager

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAppleResourceManager::CAppleResourceManager()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CAppleResourceManager::CAppleResourceManager(const TNDictionary<TNArray<Resource> >& resourceMap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(resourceMap);
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
	OR<TNArray<Resource> >	resources = mInternals->mResourceMap[CString(resourceType, true, false)];
	if (!resources.hasReference())
		return OR<CData>();

	// Search for resource by ID
	for (TIteratorD<Resource> iterator = resources->getIterator(); iterator.hasValue(); iterator.advance()) {
		// Check ID
		if (iterator->mID == resourceID)
			// Found
			return OR<CData>(iterator->mData);
	}

	return OR<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CAppleResourceManager::getPascalString(OSType resourceType, UInt16 resourceID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get data
	OR<CData>	data = get(resourceType, resourceID);

	return data.hasReference() ? OV<CString>(CString::fromPascal(*data)) : OV<CString>();
}

//----------------------------------------------------------------------------------------------------------------------
void CAppleResourceManager::set(OSType resourceType, UInt16 resourceID, const CString& name, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get array of resources by type
	CString				key(resourceType, true, false);
	TNArray<Resource>	resources =
								mInternals->mResourceMap.contains(key) ?
										*mInternals->mResourceMap[key] : TNArray<Resource>();

	// Remove existing item
	for (CArray::ItemIndex i = 0; i < resources.getCount(); i++) {
		// Check this apple resource
		if (resources[i].mID == resourceID) {
			// Found
			resources.removeAtIndex(i);

			break;
		}
	}

	// Add new item
	resources += Resource(resourceID, name, data);

	// Update map
	mInternals->mResourceMap.set(key, resources);

	// We are updated
	mInternals->mUpdated = true;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAppleResourceManager::set(OSType resourceType, UInt16 resourceID, const CString& name,
		const CString& pascalString)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get length
	UInt8	length = (UInt8) pascalString.getLength();

	// Construct data
	CData	data;
	data.appendBytes(&length, sizeof(UInt8));

	OV<CData>	pascalStringData = pascalString.getData(CString::kEncodingMacRoman);
	if (!pascalStringData.hasValue())
		return OV<SError>(sInvalidResourcePayload);
	data += *pascalStringData;

	// Add
	set(resourceType, resourceID, name, data);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CAppleResourceManager::getAsData()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData			dataData, typeListData, resourceListData, nameListData;
	TSet<CString>	types = mInternals->mResourceMap.getKeys();
	UInt32			uInt32Zero = 0;

	// Start type list data
	SInt16	lastTypeIndex = EndianS16_NtoB((SInt16) types.getCount() - 1);
	typeListData.appendBytes((const UInt8*) &lastTypeIndex, sizeof(SInt16));

	// Iterate resource types
	for (TIteratorS<CString> iterator = types.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get info
		CString&			type = *iterator;
		TArray<Resource>	resources = *mInternals->mResourceMap[type];

		// Type ID
		OSType	typeID = EndianU32_NtoB(type.getOSType());
		typeListData.appendBytes((const UInt8*) &typeID, sizeof(OSType));

		// Last resource index
		UInt16	lastResourceIndex = EndianU16_NtoB((UInt16) (resources.getCount() - 1));
		typeListData.appendBytes((const UInt8*) &lastResourceIndex, sizeof(UInt16));

		// Resource list offset
		UInt16	resourceListOffset =
						EndianU16_NtoB((UInt16) (resourceListData.getByteCount() + 2 + 8 * types.getCount()));
		typeListData.appendBytes((const UInt8*) &resourceListOffset, sizeof(UInt16));

		// Iterate resources for this type
		for (CArray::ItemIndex resourceIndex = 0; resourceIndex < resources.getCount(); resourceIndex++) {
			// Get this apple resource
			const	Resource&	resource = resources[resourceIndex];

			// ID
			UInt16	resourceID = EndianU16_NtoB(resource.mID);
			resourceListData.appendBytes((const UInt8*) &resourceID, sizeof(UInt16));

			// Name
			UInt8	nameLength = resource.mName.hasValue() ? (UInt8) resource.mName->getLength() : 0;
			if (nameLength > 0) {
				// Have name
				UInt16	nameOffset = EndianU16_NtoB((UInt16) nameListData.getByteCount());
				resourceListData.appendBytes((const UInt8*) &nameOffset, sizeof(UInt16));

				nameListData.appendBytes((const UInt8*) &nameLength, sizeof(UInt8));

				OV<CData>	nameData = resource.mName->getData(CString::kEncodingMacRoman);
				if (!nameData.hasValue())
					return TVResult<CData>(sInvalidResourceName);
				nameListData += *nameData;
			} else {
				// No name
				UInt16	nameOffset = EndianU16_NtoB(0xFFFF);
				resourceListData.appendBytes((const UInt8*) &nameOffset, sizeof(UInt16));
			}

			// Attributes + data offset
			UInt32	attributesAndDataOffset = EndianU32_NtoB((0 << 24) | (UInt32) dataData.getByteCount());
			resourceListData.appendBytes((const UInt8*) &attributesAndDataOffset, sizeof(UInt32));

			// Resource handle placeholder
			resourceListData.appendBytes((const UInt8*) &uInt32Zero, sizeof(UInt32));

			// Data
			UInt32	dataByteCount = EndianU32_NtoB((UInt16) resource.mData.getByteCount());
			dataData.appendBytes((const UInt8*) &dataByteCount, sizeof(UInt32));
			dataData += resource.mData;
		}
	}

	// Prepare to compose data
	UInt32	dataOffset = EndianU32_NtoB(256);
	UInt32	mapOffset = EndianU32_NtoB(256 + (UInt32) dataData.getByteCount());
	UInt32	dataByteCount = EndianU32_NtoB((UInt32) dataData.getByteCount());
	UInt32	mapSize =
					EndianU32_NtoB((UInt32) (28 + typeListData.getByteCount() + resourceListData.getByteCount() +
							nameListData.getByteCount()));
	UInt16	typeListOffset = EndianU16_NtoB(28);
	UInt16	nameListOffset =
					EndianU16_NtoB((UInt16) (28 + typeListData.getByteCount() + resourceListData.getByteCount()));
	UInt16	uInt16Zero = 0;

	return
			TVResult<CData>(
					// Header
					CData(&dataOffset, sizeof(dataOffset)) +
					CData(&mapOffset, sizeof(mapOffset)) +
					CData(&dataByteCount, sizeof(dataByteCount)) +
					CData(&mapSize, sizeof(mapSize)) +
					CData(
							(CData::ByteCount)
									(256 - sizeof(dataOffset) - sizeof(mapOffset) -sizeof(dataByteCount) - sizeof(mapSize))) +

					// Data
					dataData +

					// Map
					CData(&dataOffset, sizeof(dataOffset)) +	// File header copy
					CData(&mapOffset, sizeof(mapOffset)) +
					CData(&dataByteCount, sizeof(dataByteCount)) +
					CData(&mapSize, sizeof(mapSize)) +
					CData(&uInt32Zero, sizeof(uInt32Zero)) +	// Next resource map placeholder
					CData(&uInt16Zero, sizeof(uInt16Zero)) +	// File ref num placeholder
					CData(&uInt16Zero, sizeof(uInt16Zero)) +	// File attributes

					CData(&typeListOffset, sizeof(typeListOffset)) +

					CData(&nameListOffset, sizeof(nameListOffset)) +

					// Type list
					typeListData +

					// Resource list
					resourceListData +

					// Name list
					nameListData);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAppleResourceManager> CAppleResourceManager::from(const I<CRandomAccessDataSource>& randomAccessDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CByteReader	byteReader(randomAccessDataSource, true);
	OV<SError>	error;

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

	TNDictionary<TNArray<Resource> >	resourceMap;
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
		TNArray<Resource>	resources;
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

			TVResult<CData>	resourceData = byteReader.readData(*dataLength);
			ReturnValueIfResultError(resourceData, TIResult<CAppleResourceManager>(resourceData.getError()));

			OV<CString>	name;
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
					TVResult<CData>	nameData = byteReader.readData(*nameByteCount);
					ReturnValueIfResultError(nameData, TIResult<CAppleResourceManager>(nameData.getError()));

					// Store name
					name = OV<CString>(CString(*nameData, CString::kEncodingMacRoman));
				}
			}

			// Add to array
			resources += Resource(*resourceID, name, *resourceData);

			// Seek to next resource position
			error = byteReader.setPos(CByteReader::kPositionFromBeginning, innerOldOffset);
			ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));
		}

		// Store resources for this resource type
		resourceMap.set(CString(*resourceType, true, false), resources);

		// Seek to next resource type
		error = byteReader.setPos(CByteReader::kPositionFromBeginning, resourceTypeOffset);
		ReturnValueIfError(error, TIResult<CAppleResourceManager>(*error));
	}

	return TIResult<CAppleResourceManager>(OI<CAppleResourceManager>(new CAppleResourceManager(resourceMap)));
}
