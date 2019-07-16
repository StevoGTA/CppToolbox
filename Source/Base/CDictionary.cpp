//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.cpp			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDictionary.h"

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SDictionaryItemInfo

struct SDictionaryItemInfo {

			// Lifecycle methods
			SDictionaryItemInfo(UInt32 keyHashValue, const SDictionaryItem& item) :
				mKeyHashValue(keyHashValue), mItem(item), mNextItemInfo(nil)
				{}
			SDictionaryItemInfo(const SDictionaryItemInfo& itemInfo, CDictionaryItemCopyProc itemCopyProc) :
				mKeyHashValue(itemInfo.mKeyHashValue), mItem(itemInfo.mItem), mNextItemInfo(nil)
				{
					// Check value type
					if (mItem.mValue.mValueType == kDictionaryValueTypeArrayOfDictionaries)
						// Array of dictionaries
						mItem.mValue.mValue.mArrayOfDictionaries =
								new TArray<CDictionary>(*mItem.mValue.mValue.mArrayOfDictionaries);
					else if (mItem.mValue.mValueType == kDictionaryValueTypeArrayOfStrings)
						// Array of strings
						mItem.mValue.mValue.mArrayOfStrings =
								new TArray<CString>(*mItem.mValue.mValue.mArrayOfStrings);
					else if (mItem.mValue.mValueType == kDictionaryValueTypeData)
						// Data
						mItem.mValue.mValue.mData = new CData(*mItem.mValue.mValue.mData);
					else if (mItem.mValue.mValueType == kDictionaryValueTypeDictionary)
						// Dictionary
						mItem.mValue.mValue.mDictionary = new CDictionary(*mItem.mValue.mValue.mDictionary);
					else if (mItem.mValue.mValueType == kDictionaryValueTypeString)
						// String
						mItem.mValue.mValue.mString = new CString(*mItem.mValue.mValue.mString);
					else if ((mItem.mValue.mValueType == kDictionaryValueTypeItemRef) && (itemCopyProc != nil))
						// Item Ref and have item copy proc
						mItem.mValue.mValue.mItemRef = itemCopyProc(mItem.mValue.mValue.mItemRef);
				}

			// Instance methods
	bool	doesMatch(UInt32 hashValue, const CString& key)
				{ return (hashValue == mKeyHashValue) && (key == mItem.mKey); }
	void	disposeValue(CDictionaryItemDisposeProc itemDisposeProc)
				{
					// Check value type
					if (mItem.mValue.mValueType == kDictionaryValueTypeArrayOfDictionaries) {
						// Array of dictionaries
						DisposeOf(mItem.mValue.mValue.mArrayOfDictionaries);
					} else if (mItem.mValue.mValueType == kDictionaryValueTypeArrayOfStrings) {
						// Array of strings
						DisposeOf(mItem.mValue.mValue.mArrayOfStrings);
					} else if (mItem.mValue.mValueType == kDictionaryValueTypeData) {
						// Data
						DisposeOf(mItem.mValue.mValue.mData);
					} else if (mItem.mValue.mValueType == kDictionaryValueTypeDictionary) {
						// Dictionary
						DisposeOf(mItem.mValue.mValue.mDictionary);
					} else if (mItem.mValue.mValueType == kDictionaryValueTypeString) {
						// String
						DisposeOf(mItem.mValue.mValue.mString);
					} else if ((mItem.mValue.mValueType == kDictionaryValueTypeItemRef) && (itemDisposeProc != nil))
						// Item Ref and have item dispose proc
						itemDisposeProc(mItem.mValue.mValue.mItemRef);
				}

	// Properties
	UInt32					mKeyHashValue;
	SDictionaryItem			mItem;
	SDictionaryItemInfo*	mNextItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionaryIteratorInfo

struct CDictionaryIteratorInfo : public CIteratorInfo {
	// Methods
	public:
					// Lifecycle methods
					CDictionaryIteratorInfo(const CDictionaryInternals& internals, UInt32 initialReference) :
						CIteratorInfo(), mInternals(internals), mInitialReference(initialReference), mCurrentIndex(0),
								mCurrentItemInfo(nil)
						{}

					// CIteratorInfo methods
	CIteratorInfo*	copy()
						{
							// Make copy
							CDictionaryIteratorInfo*	iteratorInfo =
																new CDictionaryIteratorInfo(mInternals,
																		mInitialReference);
							iteratorInfo->mCurrentIndex = mCurrentIndex;
							iteratorInfo->mCurrentItemInfo = mCurrentItemInfo;

							return iteratorInfo;
						}

	// Properties
	const	CDictionaryInternals&	mInternals;
			UInt32					mInitialReference;
			UInt32					mCurrentIndex;
			SDictionaryItemInfo*	mCurrentItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionaryInternals

class CDictionaryInternals {
	public:
											CDictionaryInternals(CDictionaryItemCopyProc itemCopyProc,
													CDictionaryItemDisposeProc itemDisposeProc,
													CDictionaryItemEqualsProc itemEqualsProc) :
												mItemCopyProc(itemCopyProc), mItemDisposeProc(itemDisposeProc),
														mItemEqualsProc(itemEqualsProc), mCount(0), mItemInfosCount(16),
														mReferenceCount(1), mReference(0)
												{
													mItemInfos =
															(SDictionaryItemInfo**)
																	::calloc(mItemInfosCount,
																			sizeof(SDictionaryItemInfo*));
												}
											~CDictionaryInternals()
												{
													// Remove all
													removeAllInternal();

													// Cleanup
													::free(mItemInfos);
												}

				CDictionaryInternals*		addReference() { mReferenceCount++; return this; }
				void						removeReference()
												{
													// Decrement reference count and check if we are the last one
													if (--mReferenceCount == 0) {
														// We going away
														CDictionaryInternals*	THIS = this;
														DisposeOf(THIS);
													}
												}

				CDictionaryInternals*		prepareForWrite()
												{
													// Check reference count.  If there is more than 1 reference, we
													//	implement a "copy on write".  So we will clone ourselves so we
													//	have a personal buffer that can be changed while leaving the
													//	exiting buffer as-is for the other references.
													if (mReferenceCount > 1) {
														// Multiple references, copy
														CDictionaryInternals*	dictionaryInternals =
																						new CDictionaryInternals(
																								mItemCopyProc,
																								mItemDisposeProc,
																								mItemEqualsProc);
														dictionaryInternals->mCount = mCount;
														dictionaryInternals->mReference = mReference;

														for (UInt32 i = 0; i < mItemInfosCount; i++) {
															// Setup for this linked list
															SDictionaryItemInfo*	itemInfo = mItemInfos[i];
															SDictionaryItemInfo*	dictionaryInternalsItemInfo = nil;

															while (itemInfo != nil) {
																// Check for first in the linked list
																if (dictionaryInternalsItemInfo == nil) {
																	// First in this linked list
																	dictionaryInternals->mItemInfos[i] =
																			new SDictionaryItemInfo(*itemInfo,
																					mItemCopyProc);
																	dictionaryInternalsItemInfo =
																			dictionaryInternals->mItemInfos[i];
																} else {
																	// Next one in this linked list
																	dictionaryInternalsItemInfo->mNextItemInfo =
																			new SDictionaryItemInfo(*itemInfo,
																					mItemCopyProc);
																	dictionaryInternalsItemInfo =
																			dictionaryInternalsItemInfo->mNextItemInfo;
																}

																// Next
																itemInfo = itemInfo->mNextItemInfo;
															}
														}

														// One less reference here
														mReferenceCount--;

														return dictionaryInternals;
													} else
														// Only a single reference
														return this;
												}
				SDictionaryValue*			getValue(const CString& key)
												{
													// Setup
													UInt32	hashValue = CHasher::getValueForHashable(key);
													UInt32	index = hashValue & (mItemInfosCount - 1);

													// Find item info that matches
													SDictionaryItemInfo*	itemInfo = mItemInfos[index];
													while ((itemInfo != nil) && !itemInfo->doesMatch(hashValue, key))
														// Advance to next item info
														itemInfo = itemInfo->mNextItemInfo;

													return (itemInfo != nil) ? &itemInfo->mItem.mValue : nil;
												}
				CDictionaryInternals*		set(const CString& key, const SDictionaryValue& value)
												{
													// Prepare for write
													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

													// Setup
													UInt32	hashValue = CHasher::getValueForHashable(key);
													UInt32	index =
																	hashValue &
																			(dictionaryInternals->mItemInfosCount - 1);

													// Find
													SDictionaryItemInfo*	previousItemInfo = nil;
													SDictionaryItemInfo*	currentItemInfo =
																					dictionaryInternals->
																							mItemInfos[index];
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
															dictionaryInternals->mItemInfos[index] =
																	new SDictionaryItemInfo(hashValue,
																			SDictionaryItem(key, value));
														else
															// Add to the end
															previousItemInfo->mNextItemInfo =
																	new SDictionaryItemInfo(hashValue,
																			SDictionaryItem(key, value));

														// Update info
														dictionaryInternals->mCount++;
														dictionaryInternals->mReference++;
													} else {
														// Did find a match
														currentItemInfo->disposeValue(mItemDisposeProc);
														currentItemInfo->mItem.mValue = value;
													}

													return dictionaryInternals;
												}
				CDictionaryInternals*		remove(const CString& key)
												{
													// Prepare for write
													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

													// Setup
													UInt32	hashValue = CHasher::getValueForHashable(key);
													UInt32	index =
																	hashValue &
																			(dictionaryInternals->mItemInfosCount - 1);

													// Find
													SDictionaryItemInfo*	previousItemInfo = nil;
													SDictionaryItemInfo*	currentItemInfo =
																					dictionaryInternals->
																							mItemInfos[index];
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
															dictionaryInternals->mItemInfos[index] =
																	currentItemInfo->mNextItemInfo;
														else
															// Not the first item info
															previousItemInfo->mNextItemInfo =
																	currentItemInfo->mNextItemInfo;

														// Cleanup
														remove(currentItemInfo, false);

														// Update info
														dictionaryInternals->mCount--;
														dictionaryInternals->mReference++;
													}

													return dictionaryInternals;
												}
				CDictionaryInternals*		removeAll()
												{
													// Check if empty
													if (mCount == 0)
														// Nothing to remove
														return this;

													// Prepare for write
													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

													// Remove all
													dictionaryInternals->removeAllInternal();

													// Update info
													dictionaryInternals->mCount = 0;
													dictionaryInternals->mReference++;

													return dictionaryInternals;
												}
				void						removeAllInternal()
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
				void						remove(SDictionaryItemInfo* itemInfo, bool removeAll)
												{
													// Check for next item info
													if (removeAll && (itemInfo->mNextItemInfo != nil))
														// Remove the next item info
														remove(itemInfo->mNextItemInfo, true);

													// Dispose
													itemInfo->disposeValue(mItemDisposeProc);

													DisposeOf(itemInfo);
												}

				TIteratorS<SDictionaryItem>	getIterator() const
												{
													// Setup
													CDictionaryIteratorInfo*	iteratorInfo =
																						new CDictionaryIteratorInfo(
																								*this, mReference);

													// Find first item info
													while ((mItemInfos[iteratorInfo->mCurrentIndex] == nil) &&
															(++iteratorInfo->mCurrentIndex < mItemInfosCount)) ;

													SDictionaryItem*	firstItem = nil;
													if (iteratorInfo->mCurrentIndex < mItemInfosCount) {
														// Have first item info
														iteratorInfo->mCurrentItemInfo =
																mItemInfos[iteratorInfo->mCurrentIndex];
														firstItem = &mItemInfos[iteratorInfo->mCurrentIndex]->mItem;
													}

													return TIteratorS<SDictionaryItem>(firstItem, iteratorAdvance,
															*iteratorInfo);
												}

		static	void*						iteratorAdvance(CIteratorInfo& iteratorInfo)
												{
													// Setup
													CDictionaryIteratorInfo&	dictionaryIteratorInfo =
																						(CDictionaryIteratorInfo&)
																								iteratorInfo;

													// Internals check
													AssertFailIf(dictionaryIteratorInfo.mInitialReference !=
															dictionaryIteratorInfo.mInternals.mReference);

													// Check for additional item info in linked list
													if (dictionaryIteratorInfo.mCurrentItemInfo->mNextItemInfo != nil) {
														// Have next item info
														dictionaryIteratorInfo.mCurrentItemInfo =
																dictionaryIteratorInfo.mCurrentItemInfo->mNextItemInfo;
													} else {
														// End of item info linked list
														while ((++dictionaryIteratorInfo.mCurrentIndex <
																		dictionaryIteratorInfo.mInternals.
																				mItemInfosCount)
																&& (dictionaryIteratorInfo.mInternals.mItemInfos
																		[dictionaryIteratorInfo.mCurrentIndex] ==
																		nil)) ;

														// Check if found another item info
														if (dictionaryIteratorInfo.mCurrentIndex <
																dictionaryIteratorInfo.mInternals.mItemInfosCount)
															// Found another item info
															dictionaryIteratorInfo.mCurrentItemInfo =
																	dictionaryIteratorInfo.mInternals
																			.mItemInfos[
																					dictionaryIteratorInfo.
																							mCurrentIndex];
														else
															// No more item infos
															dictionaryIteratorInfo.mCurrentItemInfo = nil;
													}

													return (dictionaryIteratorInfo.mCurrentItemInfo != nil) ?
															(void*) &dictionaryIteratorInfo.mCurrentItemInfo->mItem :
															nil;
												}

		CDictionaryItemCopyProc		mItemCopyProc;
		CDictionaryItemDisposeProc	mItemDisposeProc;
		CDictionaryItemEqualsProc	mItemEqualsProc;
		CDictionaryKeyCount			mCount;
		SDictionaryItemInfo**		mItemInfos;
		UInt32						mItemInfosCount;
		UInt32						mReferenceCount;
		UInt32						mReference;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	bool	sDictionaryValueIsEqualProc(const SDictionaryValue& value1, const SDictionaryValue& value2,
						CDictionaryItemEqualsProc itemEqualsProc);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary

CDictionary	CDictionary::mEmpty;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(CDictionaryItemCopyProc itemCopyProc, CDictionaryItemDisposeProc itemDisposeProc,
		CDictionaryItemEqualsProc itemEqualsProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CDictionaryInternals(itemCopyProc, itemDisposeProc, itemEqualsProc);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::~CDictionary()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CDictionaryKeyCount CDictionary::getKeyCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCount;
}

//----------------------------------------------------------------------------------------------------------------------
TSet<CString> CDictionary::getKeys() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TSet<CString>	keys;

	// Iterate all items
	for (TIteratorS<SDictionaryItem> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance())
		// Add key
		keys.add(iterator.getValue().mKey);

	return keys;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::contains(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getValue(key) != nil;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::getBool(const CString& key, bool notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeBool);
	if (value->mValueType != kDictionaryValueTypeBool)
		// Return not found value
		return notFoundValue;

	return value->mValue.mBool;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CDictionary> CDictionary::getArrayOfDictionaries(const CString& key, const TArray<CDictionary>& notFoundValue)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeArrayOfDictionaries);
	if (value->mValueType != kDictionaryValueTypeArrayOfDictionaries)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mArrayOfDictionaries;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CDictionary::getArrayOfStrings(const CString& key, const TArray<CString>& notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeArrayOfStrings);
	if (value->mValueType != kDictionaryValueTypeArrayOfStrings)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mArrayOfStrings;
}

//----------------------------------------------------------------------------------------------------------------------
CData CDictionary::getData(const CString& key, const CData& notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeData);
	if (value->mValueType != kDictionaryValueTypeData)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mData;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CDictionary::getDictionary(const CString& key, const CDictionary& notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeDictionary);
	if (value->mValueType != kDictionaryValueTypeDictionary)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mDictionary;
}

//----------------------------------------------------------------------------------------------------------------------
CString CDictionary::getString(const CString& key, const CString& notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeString);
	if (value->mValueType != kDictionaryValueTypeString)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mString;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CDictionary::getFloat32(const CString& key, Float32 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeFloat32:	return value->mValue.mFloat32;
		case kDictionaryValueTypeFloat64:	return value->mValue.mFloat64;
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CDictionary::getFloat64(const CString& key, Float64 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeFloat32:	return value->mValue.mFloat32;
		case kDictionaryValueTypeFloat64:	return value->mValue.mFloat64;
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CDictionary::getSInt8(const CString& key, SInt8 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CDictionary::getSInt16(const CString& key, SInt16 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CDictionary::getSInt32(const CString& key, SInt32 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (SInt32) value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (SInt32) value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDictionary::getSInt64(const CString& key, SInt64 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CDictionary::getUInt8(const CString& key, UInt8 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CDictionary::getUInt16(const CString& key, UInt16 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CDictionary::getUInt32(const CString& key, UInt32 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (UInt32) value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (UInt32) value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDictionary::getUInt64(const CString& key, UInt64 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CDictionaryItemRef CDictionary::getItemRef(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return nil;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeItemRef);
	if (value->mValueType != kDictionaryValueTypeItemRef)
		// Return not found value
		return nil;

	return value->mValue.mItemRef;
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeBool;
	dictionaryValue.mValue.mBool = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CDictionary>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeArrayOfDictionaries;
	dictionaryValue.mValue.mArrayOfDictionaries = new TArray<CDictionary>(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CString>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeArrayOfStrings;
	dictionaryValue.mValue.mArrayOfStrings = new TArray<CString>(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CData& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeData;
	dictionaryValue.mValue.mData = new CData(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CDictionary& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeDictionary;
	dictionaryValue.mValue.mDictionary = new CDictionary(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CString& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeString;
	dictionaryValue.mValue.mString = new CString(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeFloat32;
	dictionaryValue.mValue.mFloat32 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeFloat64;
	dictionaryValue.mValue.mFloat64 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeSInt8;
	dictionaryValue.mValue.mSInt8 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeSInt16;
	dictionaryValue.mValue.mSInt16 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeSInt32;
	dictionaryValue.mValue.mSInt32 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeSInt64;
	dictionaryValue.mValue.mSInt64 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeUInt8;
	dictionaryValue.mValue.mUInt8 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeUInt16;
	dictionaryValue.mValue.mUInt16 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeUInt32;
	dictionaryValue.mValue.mUInt32 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeUInt64;
	dictionaryValue.mValue.mUInt64 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, CDictionaryItemRef value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeItemRef;
	dictionaryValue.mValue.mItemRef = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	mInternals = mInternals->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove all
	mInternals = mInternals->removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::equals(const CDictionary& other, void* itemCompareProcUserData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check count
	if (mInternals->mCount != other.mInternals->mCount)
		// Counts differ
		return false;

	// Iterate all items
	for (TIteratorS<SDictionaryItem> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get value
		SDictionaryItem		item = iterator.getValue();
		SDictionaryValue*	value = other.mInternals->getValue(item.mKey);
		if ((value == nil) || !sDictionaryValueIsEqualProc(item.mValue, *value, mInternals->mItemEqualsProc))
			// Value not found or value is not the same
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
TIteratorS<SDictionaryItem> CDictionary::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getIterator();
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary& CDictionary::operator=(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions
//----------------------------------------------------------------------------------------------------------------------
bool sDictionaryValueIsEqualProc(const SDictionaryValue& value1, const SDictionaryValue& value2,
		CDictionaryItemEqualsProc itemEqualsProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (value1.mValueType != value2.mValueType)
		// Mismatch
		return false;

	switch (value1.mValueType) {
		case kDictionaryValueTypeBool:
			// Bool
			return value1.mValue.mBool == value2.mValue.mBool;

		case kDictionaryValueTypeArrayOfDictionaries:
			// Array of Dictionaries
			return *value1.mValue.mArrayOfDictionaries == *value2.mValue.mArrayOfDictionaries;

		case kDictionaryValueTypeArrayOfStrings:
			// Array of Strings
			return *value1.mValue.mArrayOfStrings == *value2.mValue.mArrayOfStrings;

		case kDictionaryValueTypeData:
			// Data
			return *value1.mValue.mData == *value2.mValue.mData;

		case kDictionaryValueTypeDictionary:
			// Dictionary
			return *value1.mValue.mDictionary == *value2.mValue.mDictionary;

		case kDictionaryValueTypeString:
			// String
			return *value1.mValue.mString == *value2.mValue.mString;

		case kDictionaryValueTypeFloat32:
			// Float32
			return value1.mValue.mFloat32 == value2.mValue.mFloat32;

		case kDictionaryValueTypeFloat64:
			// Float64
			return value1.mValue.mFloat64 == value2.mValue.mFloat64;

		case kDictionaryValueTypeSInt8:
			// SInt8
			return value1.mValue.mSInt8 == value2.mValue.mSInt8;

		case kDictionaryValueTypeSInt16:
			// SInt16
			return value1.mValue.mSInt16 == value2.mValue.mSInt16;

		case kDictionaryValueTypeSInt32:
			// SInt32
			return value1.mValue.mSInt32 == value2.mValue.mSInt32;

		case kDictionaryValueTypeSInt64:
			// SInt64
			return value1.mValue.mSInt64 == value2.mValue.mSInt64;

		case kDictionaryValueTypeUInt8:
			// UInt8
			return value1.mValue.mUInt8 == value2.mValue.mUInt8;

		case kDictionaryValueTypeUInt16:
			// UInt16
			return value1.mValue.mUInt16 == value2.mValue.mUInt16;

		case kDictionaryValueTypeUInt32:
			// UInt32
			return value1.mValue.mUInt32 == value2.mValue.mUInt32;

		case kDictionaryValueTypeUInt64:
			// UInt64
			return value1.mValue.mUInt64 == value2.mValue.mUInt64;

		case kDictionaryValueTypeItemRef:
			// ItemRef
			return (itemEqualsProc != nil) ?
					itemEqualsProc(value1.mValue.mItemRef, value2.mValue.mItemRef) :
					(value1.mValue.mItemRef == value2.mValue.mItemRef);
	}
}
