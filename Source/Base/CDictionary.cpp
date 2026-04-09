//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.cpp			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDictionary.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDictionary::Iterator

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Iterator::Iterator(const I<IteratorInfo>& iteratorInfo) : CIterator(), mIteratorInfo(iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Iterator::Iterator(const Iterator& other) : CIterator(other), mIteratorInfo(other.mIteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: CIterator methods

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::Iterator::isValid() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mIteratorInfo->getCurrentIndex() < mIteratorInfo->getCount();
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CDictionary::Iterator::getIndex() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mIteratorInfo->getCurrentIndex();
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::Iterator::advance()
//----------------------------------------------------------------------------------------------------------------------
{
	// Advance
	mIteratorInfo->advance();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CDictionary::Iterator::getKey() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mIteratorInfo->getCurrentKey();
}

//----------------------------------------------------------------------------------------------------------------------
SValue& CDictionary::Iterator::getValue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mIteratorInfo->getCurrentValue();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SDictionaryItemInfo

struct SDictionaryItemInfo {
			SDictionaryItemInfo(UInt32 keyHashValue, const CString& key, const SValue& value) :
				mKeyHashValue(keyHashValue), mItem(key, value), mNextItemInfo(nil)
				{}
			SDictionaryItemInfo(const SDictionaryItemInfo& itemInfo, SValue::OpaqueCopyProc opaqueCopyProc) :
				mKeyHashValue(itemInfo.mKeyHashValue), mItem(itemInfo.mItem, opaqueCopyProc), mNextItemInfo(nil)
				{}

	bool	doesMatch(UInt32 hashValue, const CString& key)
				{ return (hashValue == mKeyHashValue) && (key == mItem.getKey()); }
	void	disposeValue(SValue::OpaqueDisposeProc opaqueDisposeProc)
				{ mItem.getValue().dispose(opaqueDisposeProc); }

	UInt32					mKeyHashValue;
	CDictionary::Item		mItem;
	SDictionaryItemInfo*	mNextItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CStandardBacking

class CStandardBacking : public CDictionary::Backing {
	class IteratorInfo : public CDictionary::IteratorInfo {
		public:
								IteratorInfo(const CStandardBacking& backing,
										UInt32 initialReference, UInt32 itemInfosIndex,
										SDictionaryItemInfo* initialItemInfo) :
									CDictionary::IteratorInfo(),
											mBacking(backing), mInitialReference(initialReference),
											mCurrentItemInfoIndex(itemInfosIndex), mCurrentItemInfo(initialItemInfo),
											mCurrentIndex(0)
									{}

					UInt32		getCount() const
									{ return mBacking.getCount(); }
					UInt32		getCurrentIndex() const
									{ return mCurrentIndex; }
			const	CString&	getCurrentKey() const
									{ return mCurrentItemInfo->mItem.getKey(); }
					SValue&		getCurrentValue() const
									{ return mCurrentItemInfo->mItem.getValue(); }
					void		advance()
									{
										// Internals check
										AssertFailIf(mInitialReference != mBacking.mReference);

										// Check for additional item info in linked list
										if (mCurrentItemInfo->mNextItemInfo != nil) {
											// Have next item info
											mCurrentItemInfo = mCurrentItemInfo->mNextItemInfo;
											mCurrentIndex++;
										} else {
											// End of item info linked list
											while ((++mCurrentItemInfoIndex < mBacking.mItemInfosCount) &&
													(mBacking.mItemInfos[mCurrentItemInfoIndex] == nil)) ;

											// Update
											mCurrentItemInfo =
													(mCurrentItemInfoIndex < mBacking.mItemInfosCount) ?
															mBacking.mItemInfos[mCurrentItemInfoIndex] : nil;
											mCurrentIndex++;
										}
									}

		private:
			const	CStandardBacking&		mBacking;

					UInt32					mInitialReference;
					UInt32					mCurrentItemInfoIndex;
					SDictionaryItemInfo*	mCurrentItemInfo;

					UInt32					mCurrentIndex;
	};

	public:

										CStandardBacking(SValue::OpaqueCopyProc opaqueCopyProc,
												SValue::OpaqueEqualsProc opaqueEqualsProc,
												SValue::OpaqueDisposeProc opaqueDisposeProc) :
											mOpaqueCopyProc(opaqueCopyProc), mOpaqueEqualsProc(opaqueEqualsProc),
													mOpaqueDisposeProc(opaqueDisposeProc),
													mCount(0), mItemInfosCount(16),
													mItemInfos(
															(SDictionaryItemInfo**)
																	::calloc(mItemInfosCount,
																			sizeof(SDictionaryItemInfo*))),
													mReference(0)
											{}
										CStandardBacking(const CStandardBacking& other) :
											mOpaqueCopyProc(other.mOpaqueCopyProc),
													mOpaqueEqualsProc(other.mOpaqueEqualsProc),
													mOpaqueDisposeProc(other.mOpaqueDisposeProc),
													mCount(other.mCount), mItemInfosCount(other.mItemInfosCount),
													mItemInfos(
															(SDictionaryItemInfo**)
																	::calloc(mItemInfosCount,
																			sizeof(SDictionaryItemInfo*))),
													mReference(0)
											{
												// Copy item infos
												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Setup for this linked list
													SDictionaryItemInfo*	itemInfo = other.mItemInfos[i];
													SDictionaryItemInfo*	dictionaryInternalsItemInfo = nil;

													while (itemInfo != nil) {
														// Check for first in the linked list
														if (dictionaryInternalsItemInfo == nil) {
															// First in this linked list
															mItemInfos[i] =
																	new SDictionaryItemInfo(*itemInfo, mOpaqueCopyProc);
															dictionaryInternalsItemInfo = mItemInfos[i];
														} else {
															// Next one in this linked list
															dictionaryInternalsItemInfo->mNextItemInfo =
																	new SDictionaryItemInfo(*itemInfo, mOpaqueCopyProc);
															dictionaryInternalsItemInfo =
																	dictionaryInternalsItemInfo->mNextItemInfo;
														}

														// Next
														itemInfo = itemInfo->mNextItemInfo;
													}
												}
											}
										~CStandardBacking()
											{
												// Remove all
												removeAllInternal();

												// Cleanup
												::free(mItemInfos);
											}

		CDictionary::Count				getCount() const
											{ return mCount; }
		OR<SValue>						getValue(const CString& key) const
											{
												// Setup
												UInt32	hashValue = key.getHashValue();
												UInt32	index = hashValue & (mItemInfosCount - 1);

												// Find item info that matches
												SDictionaryItemInfo*	itemInfo = mItemInfos[index];
												while ((itemInfo != nil) && !itemInfo->doesMatch(hashValue, key))
													// Advance to next item info
													itemInfo = itemInfo->mNextItemInfo;

												return (itemInfo != nil) ?
														OR<SValue>(itemInfo->mItem.getValue()) : OR<SValue>();
											}
		void							set(const CString& key, const SValue& value)
											{
												// Setup
												UInt32	hashValue = key.getHashValue();
												UInt32	index = hashValue & (mItemInfosCount - 1);

												// Find
												SDictionaryItemInfo*	previousItemInfo = nil;
												SDictionaryItemInfo*	currentItemInfo = mItemInfos[index];
												while ((currentItemInfo != nil) &&
														!currentItemInfo->doesMatch(hashValue, key)) {
													// Next in linked list
													previousItemInfo = currentItemInfo;
													currentItemInfo = currentItemInfo->mNextItemInfo;
												}

												// Check results
												if (currentItemInfo == nil) {
													// Did not find
													if (previousItemInfo == nil)
														// First one
														mItemInfos[index] =
																new SDictionaryItemInfo(hashValue, key, value);
													else
														// Add to the end
														previousItemInfo->mNextItemInfo =
																new SDictionaryItemInfo(hashValue, key, value);

													// Update info
													mCount++;
													mReference++;
												} else {
													// Did find a match
													currentItemInfo->disposeValue(mOpaqueDisposeProc);
													currentItemInfo->mItem.getValue() = SValue(value, mOpaqueCopyProc);
												}
											}
		void							remove(const CString& key)
											{
												// Setup
												UInt32	hashValue = key.getHashValue();
												UInt32	index = hashValue & (mItemInfosCount - 1);

												// Find
												SDictionaryItemInfo*	previousItemInfo = nil;
												SDictionaryItemInfo*	currentItemInfo = mItemInfos[index];
												while ((currentItemInfo != nil) &&
														!currentItemInfo->doesMatch(hashValue, key)) {
													// Next in linked list
													previousItemInfo = currentItemInfo;
													currentItemInfo = currentItemInfo->mNextItemInfo;
												}

												// Check results
												if (currentItemInfo != nil) {
													// Did find a match
													if (previousItemInfo == nil)
														// First item info
														mItemInfos[index] = currentItemInfo->mNextItemInfo;
													else
														// Not the first item info
														previousItemInfo->mNextItemInfo =
																currentItemInfo->mNextItemInfo;

													// Cleanup
													remove(currentItemInfo, false);

													// Update info
													mCount--;
													mReference++;
												}
											}
		void							remove(const TSet<CString>& keys)
											{
												// Iterate keys
												for (TSet<CString>::Iterator iterator = keys.getIterator(); iterator;
														iterator++)
													// Remove this key
													remove(*iterator);
											}
		void							removeAll()
											{
												// Check if empty
												if (mCount == 0)
													// Nothing to remove
													return;

												// Remove all
												removeAllInternal();

												// Update info
												mCount = 0;
												mReference++;
											}

		I<CDictionary::IteratorInfo>	getIteratorInfo() const
											{
												// Find first item info
												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Check if have item in this slot
													if (mItemInfos[i] != nil)
														// Found first item
														return I<CDictionary::IteratorInfo>(
																new CStandardBacking::IteratorInfo(*this, mReference, i,
																		mItemInfos[i]));
												}

												return I<CDictionary::IteratorInfo>(
														new CStandardBacking::IteratorInfo(*this, mReference, 0, nil));
											}

		I<Backing>						prepareForWrite()
											{ return I<Backing>(new CStandardBacking(*this)); }
		SValue::OpaqueEqualsProc		getOpaqueEqualsProc() const
											{ return mOpaqueEqualsProc; }
		private:
		void							removeAllInternal()
											{
												// Iterate all item infos
												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Check if have an item info
													if (mItemInfos[i] != nil) {
														// Remove this chain
														remove(mItemInfos[i], true);

														// Clear
														mItemInfos[i] = nil;
													}
												}
											}
		void							remove(SDictionaryItemInfo* itemInfo, bool removeAll)
											{
												// Check for next item info
												if (removeAll && (itemInfo->mNextItemInfo != nil))
													// Remove the next item info
													remove(itemInfo->mNextItemInfo, true);

												// Dispose
												itemInfo->disposeValue(mOpaqueDisposeProc);

												Delete(itemInfo);
											}

	public:
		SValue::OpaqueCopyProc		mOpaqueCopyProc;
		SValue::OpaqueEqualsProc	mOpaqueEqualsProc;
		SValue::OpaqueDisposeProc	mOpaqueDisposeProc;

		CDictionary::Count			mCount;
		UInt32						mItemInfosCount;
		SDictionaryItemInfo**		mItemInfos;
		UInt32						mReference;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary

// MARK: Properties

const	CDictionary	CDictionary::mEmpty;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(SValue::OpaqueCopyProc opaqueCopyProc, SValue::OpaqueEqualsProc opaqueEqualsProc,
		SValue::OpaqueDisposeProc opaqueDisposeProc) : CEquatable(),
		mBacking(I<Backing>(new CStandardBacking(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc)))
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const I<Backing>& backing) : CEquatable(), mBacking(backing)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const CDictionary& other) : CEquatable(), mBacking(other.mBacking)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::~CDictionary()
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Count CDictionary::getCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mBacking->getCount();
}

//----------------------------------------------------------------------------------------------------------------------
TSet<CString> CDictionary::getKeys() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNSet<CString>	keys;

	// Iterate keys
	for (KeyIterator keyIterator = getKeyIterator(); keyIterator; keyIterator++)
		// Add key
		keys += *keyIterator;

	return keys;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::contains(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mBacking->getValue(key).hasReference();
}

//----------------------------------------------------------------------------------------------------------------------
const SValue& CDictionary::getValue(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mBacking->getValue(key);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SValue> CDictionary::getOValue(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<SValue>(*value) : OV<SValue>();
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::getBool(const CString& key, bool defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getBool(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<bool> CDictionary::getOVBool(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<bool>(value->getBool()) : OV<bool>();
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CDictionary>& CDictionary::getArrayOfDictionaries(const CString& key,
		const TArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getArrayOfDictionaries(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CDictionary> > CDictionary::getOVArrayOfDictionaries(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ?
			OV<TArray<CDictionary> >(value->getArrayOfDictionaries()) : OV<TArray<CDictionary> >();
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& CDictionary::getArrayOfStrings(const CString& key, const TArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getArrayOfStrings(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CString> > CDictionary::getOVArrayOfStrings(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<TArray<CString> >(value->getArrayOfStrings()) : OV<TArray<CString> >();
}

//----------------------------------------------------------------------------------------------------------------------
const CData& CDictionary::getData(const CString& key, const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getData(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CDictionary::getOVData(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<CData>(value->getData()) : OV<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& CDictionary::getDictionary(const CString& key, const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getDictionary(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CDictionary> CDictionary::getOVDictionary(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<CDictionary>(value->getDictionary()) : OV<CDictionary>();
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CDictionary::getString(const CString& key, const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getString(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CDictionary::getOVString(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<CString>(value->getString()) : OV<CString>();
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CDictionary::getFloat32(const CString& key, Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getFloat32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<Float32> CDictionary::getOVFloat32(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<Float32>(value->getFloat32()) : OV<Float32>();
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CDictionary::getFloat64(const CString& key, Float64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getFloat64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<Float64> CDictionary::getOVFloat64(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<Float64>(value->getFloat64()) : OV<Float64>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CDictionary::getSInt8(const CString& key, SInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getSInt8(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt8> CDictionary::getOVSInt8(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<SInt8>(value->getSInt8()) : OV<SInt8>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CDictionary::getSInt16(const CString& key, SInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getSInt16(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt16> CDictionary::getOVSInt16(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<SInt16>(value->getSInt16()) : OV<SInt16>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CDictionary::getSInt32(const CString& key, SInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getSInt32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt32> CDictionary::getOVSInt32(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<SInt32>(value->getSInt32()) : OV<SInt32>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDictionary::getSInt64(const CString& key, SInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getSInt64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt64> CDictionary::getOVSInt64(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<SInt64>(value->getSInt64()) : OV<SInt64>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CDictionary::getUInt8(const CString& key, UInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getUInt8(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt8> CDictionary::getOVUInt8(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<UInt8>(value->getUInt8()) : OV<UInt8>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CDictionary::getUInt16(const CString& key, UInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getUInt16(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt16> CDictionary::getOVUInt16(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<UInt16>(value->getUInt16()) : OV<UInt16>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CDictionary::getUInt32(const CString& key, UInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getUInt32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt32> CDictionary::getOVUInt32(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<UInt32>(value->getUInt32()) : OV<UInt32>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDictionary::getUInt64(const CString& key, UInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? value->getUInt64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt64> CDictionary::getOVUInt64(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<UInt64>(value->getUInt64()) : OV<UInt64>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SValue::Opaque> CDictionary::getOpaque(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mBacking->getValue(key);

	return value.hasReference() ? OV<SValue::Opaque>(value->getOpaque()) : OV<SValue::Opaque>();
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CDictionary>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CString>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CData& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CDictionary& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CString& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SValue::Opaque value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const SValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Set
	mBacking->set(key, value);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Remove
	mBacking->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const TArray<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Remove
	mBacking->remove(TNSet<CString>(keys));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const TSet<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Remove
	mBacking->remove(keys);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CDictionary::removing(const TArray<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	dictionary(*this);

	// Remove
	dictionary.remove(keys);

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CDictionary::removing(const TSet<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	dictionary(*this);

	// Remove
	dictionary.remove(keys);

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check backing reference count
	if (mBacking.getReferenceCount() > 1)
		// Prepare for write
		mBacking = mBacking->prepareForWrite();

	// Remove all
	mBacking->removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::equals(const CDictionary& other, void* itemCompareProcUserData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check count
	if (mBacking->getCount() != other.mBacking->getCount())
		// Counts differ
		return false;

	// Iterate all items
	for (Iterator iterator = getIterator(); iterator; iterator++) {
		// Get value
		OR<SValue>	value = other.mBacking->getValue(iterator.getKey());
		if (!value.hasReference() || !iterator.getValue().equals(*value, mBacking->getOpaqueEqualsProc()))
			// Value not found or value is not the same
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Iterator CDictionary::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return Iterator(getIteratorInfo());
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::KeyIterator CDictionary::getKeyIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return KeyIterator(getIteratorInfo());
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::ValueIterator CDictionary::getValueIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return ValueIterator(getIteratorInfo());
}

//----------------------------------------------------------------------------------------------------------------------
const OR<SValue> CDictionary::operator[](const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mBacking->getValue(key);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary& CDictionary::operator=(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if assignment to self
	if (this == &other)
		return *this;

	// Copy backing
	mBacking = other.mBacking;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CDictionary::operator+(const CDictionary& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	dictionary(*this);

	// Iterate all items
	for (Iterator iterator = other.getIterator(); iterator; iterator++)
		// Set this value
		dictionary.mBacking->set(iterator.getKey(), iterator.getValue());

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary& CDictionary::operator+=(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all items
	for (Iterator iterator = other.getIterator(); iterator; iterator++)
		// Set this value
		mBacking->set(iterator.getKey(), iterator.getValue());

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
I<CDictionary::IteratorInfo> CDictionary::getIteratorInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mBacking->getIteratorInfo();
}
