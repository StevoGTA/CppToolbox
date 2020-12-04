//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.cpp			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDictionary.h"

#include "CppToolboxAssert.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TDictionaryInternals

template <typename T> class TDictionaryInternals : public TCopyOnWriteReferenceCountable<T> {
	// Methods
	public:
											// Lifecycle methods
											TDictionaryInternals() : TCopyOnWriteReferenceCountable<T>() {}
											TDictionaryInternals(const TDictionaryInternals& other) :
												TCopyOnWriteReferenceCountable<T>()
												{}
		virtual								~TDictionaryInternals() {}

											// Instance methods
		virtual	CDictionary::KeyCount		getKeyCount() = 0;
		virtual	OR<SDictionaryValue>		getValue(const CString& key) = 0;
		virtual	CDictionaryInternals*		set(const CString& key, const SDictionaryValue& value) = 0;
		virtual	CDictionaryInternals*		remove(const CString& key) = 0;
		virtual	CDictionaryInternals*		removeAll() = 0;

		virtual	TIteratorS<SDictionaryItem>	getIterator() const = 0;

		virtual	CDictionary::ItemEqualsProc	getItemEqualsProc() const = 0;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionaryInternals

class CDictionaryInternals : public TDictionaryInternals<CDictionaryInternals> {};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SDictionaryItemInfo

struct SDictionaryItemInfo {
			// Lifecycle methods
			SDictionaryItemInfo(UInt32 keyHashValue, const CString& key, const SDictionaryValue& value) :
				mKeyHashValue(keyHashValue), mItem(key, value), mNextItemInfo(nil)
				{}
			SDictionaryItemInfo(const SDictionaryItemInfo& itemInfo, CDictionary::ItemCopyProc itemCopyProc) :
				mKeyHashValue(itemInfo.mKeyHashValue), mItem(itemInfo.mItem, itemCopyProc), mNextItemInfo(nil)
				{}

			// Instance methods
	bool	doesMatch(UInt32 hashValue, const CString& key)
				{ return (hashValue == mKeyHashValue) && (key == mItem.mKey); }
	void	disposeValue(CDictionary::ItemDisposeProc itemDisposeProc)
				{ mItem.mValue.dispose(itemDisposeProc); }

	// Properties
	UInt32					mKeyHashValue;
	SDictionaryItem			mItem;
	SDictionaryItemInfo*	mNextItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionaryIteratorInfo

class CStandardDictionaryInternals;

struct CDictionaryIteratorInfo : public CIterator::Info {
	// Methods
	public:
							// Lifecycle methods
							CDictionaryIteratorInfo(const CStandardDictionaryInternals& internals,
									UInt32 initialReference) :
								CIterator::Info(),
										mInternals(internals), mInitialReference(initialReference), mCurrentIndex(0),
										mCurrentItemInfo(nil)
								{}

							// CIterator::Info methods
		CIterator::Info*	copy()
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
	const	CStandardDictionaryInternals&	mInternals;
			UInt32							mInitialReference;
			UInt32							mCurrentIndex;
			SDictionaryItemInfo*			mCurrentItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CStandardDictionaryInternals

class CStandardDictionaryInternals : public TDictionaryInternals<CStandardDictionaryInternals> {
	public:
												// Lifecycle methods
												CStandardDictionaryInternals(CDictionary::ItemCopyProc itemCopyProc,
														CDictionary::ItemDisposeProc itemDisposeProc,
														CDictionary::ItemEqualsProc itemEqualsProc);
												CStandardDictionaryInternals(const CStandardDictionaryInternals& other);
												~CStandardDictionaryInternals();

												// TCopyOnWriteReferenceCountable methods
				CStandardDictionaryInternals*	copy() const;

												// TDictionaryInternals methods
				CDictionary::KeyCount			getKeyCount();
				OR<SDictionaryValue>			getValue(const CString& key);
				CDictionaryInternals*			set(const CString& key, const SDictionaryValue& value);
				CDictionaryInternals*			remove(const CString& key);
				CDictionaryInternals*			removeAll();

				TIteratorS<SDictionaryItem>		getIterator() const;

				CDictionary::ItemEqualsProc		getItemEqualsProc() const;

												// Private methods
				void							removeAllInternal();
				void							remove(SDictionaryItemInfo* itemInfo, bool removeAll);

												// Class methods
		static	void*							iteratorAdvance(CIterator::Info& iteratorInfo);

		CDictionary::ItemCopyProc		mItemCopyProc;
		CDictionary::ItemDisposeProc	mItemDisposeProc;
		CDictionary::ItemEqualsProc		mItemEqualsProc;

		CDictionary::KeyCount			mCount;
		SDictionaryItemInfo**			mItemInfos;
		UInt32							mItemInfosCount;
		UInt32							mReference;
};

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CStandardDictionaryInternals::CStandardDictionaryInternals(CDictionary::ItemCopyProc itemCopyProc,
		CDictionary::ItemDisposeProc itemDisposeProc, CDictionary::ItemEqualsProc itemEqualsProc) :
	TDictionaryInternals(),
			mItemCopyProc(itemCopyProc), mItemDisposeProc(itemDisposeProc), mItemEqualsProc(itemEqualsProc), mCount(0),
			mItemInfosCount(16), mReference(0)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mItemInfos = (SDictionaryItemInfo**) ::calloc(mItemInfosCount, sizeof(SDictionaryItemInfo*));
}

//----------------------------------------------------------------------------------------------------------------------
CStandardDictionaryInternals::CStandardDictionaryInternals(const CStandardDictionaryInternals& other) :
	TDictionaryInternals(other),
			mItemCopyProc(other.mItemCopyProc), mItemDisposeProc(other.mItemDisposeProc),
			mItemEqualsProc(other.mItemEqualsProc),
			mCount(other.mCount), mItemInfosCount(other.mItemInfosCount), mReference(0)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mItemInfos = (SDictionaryItemInfo**) ::calloc(mItemInfosCount, sizeof(SDictionaryItemInfo*));

	// Copy item infos
	for (UInt32 i = 0; i < mItemInfosCount; i++) {
		// Setup for this linked list
		SDictionaryItemInfo*	itemInfo = other.mItemInfos[i];
		SDictionaryItemInfo*	dictionaryInternalsItemInfo = nil;

		while (itemInfo != nil) {
			// Check for first in the linked list
			if (dictionaryInternalsItemInfo == nil) {
				// First in this linked list
				mItemInfos[i] = new SDictionaryItemInfo(*itemInfo, mItemCopyProc);
				dictionaryInternalsItemInfo = mItemInfos[i];
			} else {
				// Next one in this linked list
				dictionaryInternalsItemInfo->mNextItemInfo = new SDictionaryItemInfo(*itemInfo, mItemCopyProc);
				dictionaryInternalsItemInfo = dictionaryInternalsItemInfo->mNextItemInfo;
			}

			// Next
			itemInfo = itemInfo->mNextItemInfo;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
CStandardDictionaryInternals::~CStandardDictionaryInternals()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove all
	removeAllInternal();

	// Cleanup
	::free(mItemInfos);
}

// MARK: TDictionaryInternals methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::KeyCount CStandardDictionaryInternals::getKeyCount()
//----------------------------------------------------------------------------------------------------------------------
{
	return mCount;
}

//----------------------------------------------------------------------------------------------------------------------
OR<SDictionaryValue> CStandardDictionaryInternals::getValue(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	hashValue = CHasher::getValueForHashable(key);
	UInt32	index = hashValue & (mItemInfosCount - 1);

	// Find item info that matches
	SDictionaryItemInfo*	itemInfo = mItemInfos[index];
	while ((itemInfo != nil) && !itemInfo->doesMatch(hashValue, key))
		// Advance to next item info
		itemInfo = itemInfo->mNextItemInfo;

	return (itemInfo != nil) ? OR<SDictionaryValue>(itemInfo->mItem.mValue) : OR<SDictionaryValue>();
}

//----------------------------------------------------------------------------------------------------------------------
CDictionaryInternals* CStandardDictionaryInternals::set(const CString& key, const SDictionaryValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	CStandardDictionaryInternals*	dictionaryInternals = (CStandardDictionaryInternals*) prepareForWrite();

	// Setup
	UInt32	hashValue = CHasher::getValueForHashable(key);
	UInt32	index = hashValue & (dictionaryInternals->mItemInfosCount - 1);

	// Find
	SDictionaryItemInfo*	previousItemInfo = nil;
	SDictionaryItemInfo*	currentItemInfo = dictionaryInternals->mItemInfos[index];
	while ((currentItemInfo != nil) && !currentItemInfo->doesMatch(hashValue, key)) {
		// Next in linked list
		previousItemInfo = currentItemInfo;
		currentItemInfo = currentItemInfo->mNextItemInfo;
	}

	// Check results
	if (currentItemInfo == nil) {
		// Did not find
		if (previousItemInfo == nil)
			// First one
			dictionaryInternals->mItemInfos[index] = new SDictionaryItemInfo(hashValue, key, value);
		else
			// Add to the end
			previousItemInfo->mNextItemInfo = new SDictionaryItemInfo(hashValue, key, value);

		// Update info
		dictionaryInternals->mCount++;
		dictionaryInternals->mReference++;
	} else {
		// Did find a match
		currentItemInfo->disposeValue(mItemDisposeProc);
		currentItemInfo->mItem.mValue = value;
	}

	return (CDictionaryInternals*) dictionaryInternals;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionaryInternals* CStandardDictionaryInternals::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	CStandardDictionaryInternals*	dictionaryInternals = (CStandardDictionaryInternals*) prepareForWrite();

	// Setup
	UInt32	hashValue = CHasher::getValueForHashable(key);
	UInt32	index = hashValue & (dictionaryInternals->mItemInfosCount - 1);

	// Find
	SDictionaryItemInfo*	previousItemInfo = nil;
	SDictionaryItemInfo*	currentItemInfo = dictionaryInternals->mItemInfos[index];
	while ((currentItemInfo != nil) && !currentItemInfo->doesMatch(hashValue, key)) {
		// Next in linked list
		previousItemInfo = currentItemInfo;
		currentItemInfo = currentItemInfo->mNextItemInfo;
	}

	// Check results
	if (currentItemInfo != nil) {
		// Did find a match
		if (previousItemInfo == nil)
			// First item info
			dictionaryInternals->mItemInfos[index] = currentItemInfo->mNextItemInfo;
		else
			// Not the first item info
			previousItemInfo->mNextItemInfo = currentItemInfo->mNextItemInfo;

		// Cleanup
		remove(currentItemInfo, false);

		// Update info
		dictionaryInternals->mCount--;
		dictionaryInternals->mReference++;
	}

	return (CDictionaryInternals*) dictionaryInternals;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionaryInternals* CStandardDictionaryInternals::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if empty
	if (mCount == 0)
		// Nothing to remove
		return (CDictionaryInternals*) this;

	// Prepare for write
	CStandardDictionaryInternals*	dictionaryInternals = (CStandardDictionaryInternals*) prepareForWrite();

	// Remove all
	dictionaryInternals->removeAllInternal();

	// Update info
	dictionaryInternals->mCount = 0;
	dictionaryInternals->mReference++;

	return (CDictionaryInternals*) dictionaryInternals;
}

//----------------------------------------------------------------------------------------------------------------------
TIteratorS<SDictionaryItem> CStandardDictionaryInternals::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionaryIteratorInfo*	iteratorInfo = new CDictionaryIteratorInfo(*this, mReference);

	// Find first item info
	while ((mItemInfos[iteratorInfo->mCurrentIndex] == nil) && (++iteratorInfo->mCurrentIndex < mItemInfosCount)) ;

	SDictionaryItem*	firstItem = nil;
	if (iteratorInfo->mCurrentIndex < mItemInfosCount) {
		// Have first item info
		iteratorInfo->mCurrentItemInfo = mItemInfos[iteratorInfo->mCurrentIndex];
		firstItem = &mItemInfos[iteratorInfo->mCurrentIndex]->mItem;
	}

	return TIteratorS<SDictionaryItem>(firstItem, iteratorAdvance, *iteratorInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::ItemEqualsProc CStandardDictionaryInternals::getItemEqualsProc() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mItemEqualsProc;
}

// MARK: Private methods

//----------------------------------------------------------------------------------------------------------------------
void CStandardDictionaryInternals::removeAllInternal()
//----------------------------------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------------------------------
void CStandardDictionaryInternals::remove(SDictionaryItemInfo* itemInfo, bool removeAll)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for next item info
	if (removeAll && (itemInfo->mNextItemInfo != nil))
		// Remove the next item info
		remove(itemInfo->mNextItemInfo, true);

	// Dispose
	itemInfo->disposeValue(mItemDisposeProc);

	Delete(itemInfo);
}

//----------------------------------------------------------------------------------------------------------------------
void* CStandardDictionaryInternals::iteratorAdvance(CIterator::Info& iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionaryIteratorInfo&	dictionaryIteratorInfo = (CDictionaryIteratorInfo&) iteratorInfo;

	// Internals check
	AssertFailIf(dictionaryIteratorInfo.mInitialReference != dictionaryIteratorInfo.mInternals.mReference);

	// Check for additional item info in linked list
	if (dictionaryIteratorInfo.mCurrentItemInfo->mNextItemInfo != nil) {
		// Have next item info
		dictionaryIteratorInfo.mCurrentItemInfo = dictionaryIteratorInfo.mCurrentItemInfo->mNextItemInfo;
	} else {
		// End of item info linked list
		while ((++dictionaryIteratorInfo.mCurrentIndex < dictionaryIteratorInfo.mInternals.mItemInfosCount) &&
				(dictionaryIteratorInfo.mInternals.mItemInfos [dictionaryIteratorInfo.mCurrentIndex] == nil)) ;

		// Check if found another item info
		if (dictionaryIteratorInfo.mCurrentIndex < dictionaryIteratorInfo.mInternals.mItemInfosCount)
			// Found another item info
			dictionaryIteratorInfo.mCurrentItemInfo =
					dictionaryIteratorInfo.mInternals.mItemInfos[dictionaryIteratorInfo.mCurrentIndex];
		else
			// No more item infos
			dictionaryIteratorInfo.mCurrentItemInfo = nil;
	}

	return (dictionaryIteratorInfo.mCurrentItemInfo != nil) ?
			(void*) &dictionaryIteratorInfo.mCurrentItemInfo->mItem : nil;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProcsDictionaryInternals

class CProcsDictionaryInternals : public TDictionaryInternals<CProcsDictionaryInternals> {
	public:
											// Lifecycle methods
											CProcsDictionaryInternals(const CDictionary::ProcsInfo& procsInfo) :
												TDictionaryInternals(),
														mProcsInfo(procsInfo)
												{}
											CProcsDictionaryInternals(const CProcsDictionaryInternals& other) :
												TDictionaryInternals(),
														mProcsInfo(other.mProcsInfo)
												{
// TODO
AssertFailUnimplemented();
												}
											~CProcsDictionaryInternals()
												{ mProcsInfo.disposeUserData(); }

											// TDictionaryInternals methods
				CDictionary::KeyCount		getKeyCount()
												{ return mProcsInfo.getKeyCount(); }
				OR<SDictionaryValue>		getValue(const CString& key)
												{ return mProcsInfo.getValue(key); }
				CDictionaryInternals*		set(const CString& key, const SDictionaryValue& value)
												{
//													// Convert to standard dictionary
//													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

// TODO
AssertFailUnimplemented();

//													return dictionaryInternals;
return (CDictionaryInternals*) this;
												}
				CDictionaryInternals*		remove(const CString& key)
												{
//													// Convert to standard dictionary
//													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

// TODO
AssertFailUnimplemented();

//													return dictionaryInternals;
return (CDictionaryInternals*) this;
												}
				CDictionaryInternals*		removeAll()
												{
//													// Convert to standard dictionary
//													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

// TODO
AssertFailUnimplemented();

//													return dictionaryInternals;
return (CDictionaryInternals*) this;
												}

				TIteratorS<SDictionaryItem>	getIterator() const
												{
// TODO
AssertFailUnimplemented();
CIterator::Info*	iteratorInfo = nil;
return TIteratorS<SDictionaryItem>(nil, nil, *iteratorInfo);
												}

				CDictionary::ItemEqualsProc	getItemEqualsProc() const
												{ return nil; }

		CDictionary::ProcsInfo	mProcsInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary

// MARK: Properties

CDictionary	CDictionary::mEmpty;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(ItemCopyProc itemCopyProc, ItemDisposeProc itemDisposeProc, ItemEqualsProc itemEqualsProc) :
	CEquatable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			(CDictionaryInternals*) new CStandardDictionaryInternals(itemCopyProc, itemDisposeProc, itemEqualsProc);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const ProcsInfo& procsInfo) : CEquatable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = (CDictionaryInternals*) new CProcsDictionaryInternals(procsInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const CDictionary& other) : CEquatable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = (CDictionaryInternals*) other.mInternals->addReference();
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
CDictionary::KeyCount CDictionary::getKeyCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getKeyCount();
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
	return mInternals->getValue(key).hasReference();
}

//----------------------------------------------------------------------------------------------------------------------
const SDictionaryValue& CDictionary::getValue(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->getValue(key);
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::getBool(const CString& key, bool defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getBool(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TNArray<CDictionary>& CDictionary::getArrayOfDictionaries(const CString& key,
		const TNArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getArrayOfDictionaries(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TNArray<CString>& CDictionary::getArrayOfStrings(const CString& key, const TNArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getArrayOfStrings(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CData& CDictionary::getData(const CString& key, const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getData(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& CDictionary::getDictionary(const CString& key, const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getDictionary(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CDictionary::getString(const CString& key, const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getString(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CDictionary::getFloat32(const CString& key, Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getFloat32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CDictionary::getFloat64(const CString& key, Float64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getFloat64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CDictionary::getSInt8(const CString& key, SInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt8(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CDictionary::getSInt16(const CString& key, SInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt16(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CDictionary::getSInt32(const CString& key, SInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDictionary::getSInt64(const CString& key, SInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CDictionary::getUInt8(const CString& key, UInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt8(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CDictionary::getUInt16(const CString& key, UInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt16(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CDictionary::getUInt32(const CString& key, UInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDictionary::getUInt64(const CString& key, UInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CDictionary>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CString>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CData& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CDictionary& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CString& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, ItemRef value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, SDictionaryValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const SDictionaryValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, value);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	mInternals = (CDictionaryInternals*) mInternals->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove all
	mInternals = (CDictionaryInternals*) mInternals->removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::equals(const CDictionary& other, void* itemCompareProcUserData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check count
	if (mInternals->getKeyCount() != other.mInternals->getKeyCount())
		// Counts differ
		return false;

	// Iterate all items
	for (TIteratorS<SDictionaryItem> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get value
		SDictionaryItem			item = iterator.getValue();
		OR<SDictionaryValue>	value = other.mInternals->getValue(item.mKey);
		if (!value.hasReference() || !item.mValue.equals(*value, mInternals->getItemEqualsProc()))
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
	// Check if assignment to self
	if (this == &other)
		return *this;

	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary& CDictionary::operator+=(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all items
	for (TIteratorS<SDictionaryItem> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance())
		// Set this value
		mInternals->set(iterator.getValue().mKey, iterator.getValue().mValue);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CDictionary::ItemRef> CDictionary::getItemRef(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<ItemRef>(value->getItemRef()) : OV<ItemRef>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SDictionaryValue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(bool value) : mValueType(kValueTypeBool), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const TArray<CDictionary>& value) :
	mValueType(kValueTypeArrayOfDictionaries), mValue(new TArray<CDictionary>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const TArray<CString>& value) :
	mValueType(kValueTypeArrayOfStrings), mValue(new TArray<CString>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const CData& value) : mValueType(kValueTypeData), mValue(new CData(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const CDictionary& value) :
	mValueType(kValueTypeDictionary), mValue(new CDictionary(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const CString& value) : mValueType(kValueTypeString), mValue(new CString(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(Float32 value) : mValueType(kValueTypeFloat32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(Float64 value) : mValueType(kValueTypeFloat64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(SInt8 value) : mValueType(kValueTypeSInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(SInt16 value) : mValueType(kValueTypeSInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(SInt32 value) : mValueType(kValueTypeSInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(SInt64 value) : mValueType(kValueTypeSInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(UInt8 value) : mValueType(kValueTypeUInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(UInt16 value) : mValueType(kValueTypeUInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(UInt32 value) : mValueType(kValueTypeUInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(UInt64 value) : mValueType(kValueTypeUInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(CDictionary::ItemRef value) : mValueType(kValueTypeItemRef), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const SDictionaryValue& other) :
	mValueType(other.mValueType), mValue(other.mValue)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const SDictionaryValue& other, CDictionary::ItemCopyProc itemCopyProc) :
	mValueType(other.mValueType), mValue(other.mValue)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mValueType == kValueTypeArrayOfDictionaries)
		// Array of dictionaries
		mValue.mArrayOfDictionaries = new TNArray<CDictionary>(*mValue.mArrayOfDictionaries);
	else if (mValueType == kValueTypeArrayOfStrings)
		// Array of strings
		mValue.mArrayOfStrings = new TNArray<CString>(*mValue.mArrayOfStrings);
	else if (mValueType == kValueTypeData)
		// Data
		mValue.mData = new CData(*mValue.mData);
	else if (mValueType == kValueTypeDictionary)
		// Dictionary
		mValue.mDictionary = new CDictionary(*mValue.mDictionary);
	else if (mValueType == kValueTypeString)
		// String
		mValue.mString = new CString(*mValue.mString);
	else if ((mValueType == kValueTypeItemRef) && (itemCopyProc != nil))
		// Item Ref and have item copy proc
		mValue.mItemRef = itemCopyProc(mValue.mItemRef);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool SDictionaryValue::getBool(bool defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeBool:	return mValue.mBool;
		case kValueTypeSInt8:	return mValue.mSInt8 == 1;
		case kValueTypeSInt16:	return mValue.mSInt16 == 1;
		case kValueTypeSInt32:	return mValue.mSInt32 == 1;
		case kValueTypeSInt64:	return mValue.mSInt64 == 1;
		case kValueTypeUInt8:	return mValue.mUInt8 == 1;
		case kValueTypeUInt16:	return mValue.mUInt16 == 1;
		case kValueTypeUInt32:	return mValue.mUInt32 == 1;
		case kValueTypeUInt64:	return mValue.mUInt64 == 1;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
const TNArray<CDictionary>& SDictionaryValue::getArrayOfDictionaries(const TNArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kValueTypeArrayOfDictionaries);

	return (mValueType == kValueTypeArrayOfDictionaries) ? *mValue.mArrayOfDictionaries : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TNArray<CString>& SDictionaryValue::getArrayOfStrings(const TNArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kValueTypeArrayOfStrings);

	return (mValueType == kValueTypeArrayOfStrings) ? *mValue.mArrayOfStrings : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CData& SDictionaryValue::getData(const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kValueTypeData);

	return (mValueType == kValueTypeData) ? *mValue.mData : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& SDictionaryValue::getDictionary(const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kValueTypeDictionary);

	return (mValueType == kValueTypeDictionary) ? *mValue.mDictionary : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& SDictionaryValue::getString(const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kValueTypeString);

	return (mValueType == kValueTypeString) ? *mValue.mString : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 SDictionaryValue::getFloat32(Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeFloat32:	return mValue.mFloat32;
		case kValueTypeFloat64:	return (Float32) mValue.mFloat64;
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return mValue.mSInt16;
		case kValueTypeSInt32:	return (Float32) mValue.mSInt32;
		case kValueTypeSInt64:	return (Float32) mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return mValue.mUInt16;
		case kValueTypeUInt32:	return (Float32) mValue.mUInt32;
		case kValueTypeUInt64:	return (Float32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Float64 SDictionaryValue::getFloat64(Float64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeFloat32:	return mValue.mFloat32;
		case kValueTypeFloat64:	return mValue.mFloat64;
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return mValue.mSInt16;
		case kValueTypeSInt32:	return mValue.mSInt32;
		case kValueTypeSInt64:	return (Float64) mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return mValue.mUInt16;
		case kValueTypeUInt32:	return mValue.mUInt32;
		case kValueTypeUInt64:	return (Float64) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 SDictionaryValue::getSInt8(SInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return (SInt8) mValue.mSInt16;
		case kValueTypeSInt32:	return (SInt8)mValue.mSInt32;
		case kValueTypeSInt64:	return (SInt8)mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return (SInt8)mValue.mUInt16;
		case kValueTypeUInt32:	return (SInt8)mValue.mUInt32;
		case kValueTypeUInt64:	return (SInt8)mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 SDictionaryValue::getSInt16(SInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return mValue.mSInt16;
		case kValueTypeSInt32:	return mValue.mSInt32;
		case kValueTypeSInt64:	return (SInt16) mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return mValue.mUInt16;
		case kValueTypeUInt32:	return mValue.mUInt32;
		case kValueTypeUInt64:	return (SInt16) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 SDictionaryValue::getSInt32(SInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return mValue.mSInt16;
		case kValueTypeSInt32:	return mValue.mSInt32;
		case kValueTypeSInt64:	return (SInt32) mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return mValue.mUInt16;
		case kValueTypeUInt32:	return mValue.mUInt32;
		case kValueTypeUInt64:	return (SInt32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 SDictionaryValue::getSInt64(SInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return mValue.mSInt16;
		case kValueTypeSInt32:	return mValue.mSInt32;
		case kValueTypeSInt64:	return mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return mValue.mUInt16;
		case kValueTypeUInt32:	return mValue.mUInt32;
		case kValueTypeUInt64:	return mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 SDictionaryValue::getUInt8(UInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return (UInt8) mValue.mSInt16;
		case kValueTypeSInt32:	return (UInt8) mValue.mSInt32;
		case kValueTypeSInt64:	return (UInt8) mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return (UInt8) mValue.mUInt16;
		case kValueTypeUInt32:	return (UInt8) mValue.mUInt32;
		case kValueTypeUInt64:	return (UInt8) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 SDictionaryValue::getUInt16(UInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return mValue.mSInt16;
		case kValueTypeSInt32:	return mValue.mSInt32;
		case kValueTypeSInt64:	return (UInt16) mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return mValue.mUInt16;
		case kValueTypeUInt32:	return mValue.mUInt32;
		case kValueTypeUInt64:	return (UInt16) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 SDictionaryValue::getUInt32(UInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return mValue.mSInt16;
		case kValueTypeSInt32:	return mValue.mSInt32;
		case kValueTypeSInt64:	return (UInt32) mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return mValue.mUInt16;
		case kValueTypeUInt32:	return mValue.mUInt32;
		case kValueTypeUInt64:	return (UInt32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 SDictionaryValue::getUInt64(UInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kValueTypeSInt8:	return mValue.mSInt8;
		case kValueTypeSInt16:	return mValue.mSInt16;
		case kValueTypeSInt32:	return mValue.mSInt32;
		case kValueTypeSInt64:	return mValue.mSInt64;
		case kValueTypeUInt8:	return mValue.mUInt8;
		case kValueTypeUInt16:	return mValue.mUInt16;
		case kValueTypeUInt32:	return mValue.mUInt32;
		case kValueTypeUInt64:	return mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::ItemRef SDictionaryValue::getItemRef() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kValueTypeItemRef);

	return (mValueType == kValueTypeItemRef) ? mValue.mItemRef : nil;
}

//----------------------------------------------------------------------------------------------------------------------
bool SDictionaryValue::equals(const SDictionaryValue& other, CDictionary::ItemEqualsProc itemEqualsProc) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mValueType != other.mValueType)
		// Mismatch
		return false;

	switch (mValueType) {
		case kValueTypeBool:
			// Bool
			return mValue.mBool == other.mValue.mBool;

		case kValueTypeArrayOfDictionaries:
			// Array of Dictionaries
			return *mValue.mArrayOfDictionaries == *other.mValue.mArrayOfDictionaries;

		case kValueTypeArrayOfStrings:
			// Array of Strings
			return *mValue.mArrayOfStrings == *other.mValue.mArrayOfStrings;

		case kValueTypeData:
			// Data
			return *mValue.mData == *other.mValue.mData;

		case kValueTypeDictionary:
			// Dictionary
			return *mValue.mDictionary == *other.mValue.mDictionary;

		case kValueTypeString:
			// String
			return *mValue.mString == *other.mValue.mString;

		case kValueTypeFloat32:
			// Float32
			return mValue.mFloat32 == other.mValue.mFloat32;

		case kValueTypeFloat64:
			// Float64
			return mValue.mFloat64 == other.mValue.mFloat64;

		case kValueTypeSInt8:
			// SInt8
			return mValue.mSInt8 == other.mValue.mSInt8;

		case kValueTypeSInt16:
			// SInt16
			return mValue.mSInt16 == other.mValue.mSInt16;

		case kValueTypeSInt32:
			// SInt32
			return mValue.mSInt32 == other.mValue.mSInt32;

		case kValueTypeSInt64:
			// SInt64
			return mValue.mSInt64 == other.mValue.mSInt64;

		case kValueTypeUInt8:
			// UInt8
			return mValue.mUInt8 == other.mValue.mUInt8;

		case kValueTypeUInt16:
			// UInt16
			return mValue.mUInt16 == other.mValue.mUInt16;

		case kValueTypeUInt32:
			// UInt32
			return mValue.mUInt32 == other.mValue.mUInt32;

		case kValueTypeUInt64:
			// UInt64
			return mValue.mUInt64 == other.mValue.mUInt64;

		case kValueTypeItemRef:
			// ItemRef
			return (itemEqualsProc != nil) ?
					itemEqualsProc(mValue.mItemRef, other.mValue.mItemRef) : mValue.mItemRef == other.mValue.mItemRef;

#if TARGET_OS_WINDOWS
		default:
			// Just to make compiler happy.  Will never get here.
			return false;
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
void SDictionaryValue::dispose(CDictionary::ItemDisposeProc itemDisposeProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mValueType == kValueTypeArrayOfDictionaries) {
		// Array of dictionaries
		Delete(mValue.mArrayOfDictionaries);
	} else if (mValueType == kValueTypeArrayOfStrings) {
		// Array of strings
		Delete(mValue.mArrayOfStrings);
	} else if (mValueType == kValueTypeData) {
		// Data
		Delete(mValue.mData);
	} else if (mValueType == kValueTypeDictionary) {
		// Dictionary
		Delete(mValue.mDictionary);
	} else if (mValueType == kValueTypeString) {
		// String
		Delete(mValue.mString);
	} else if ((mValueType == kValueTypeItemRef) && (itemDisposeProc != nil)) {
		// Item Ref and have item dispose proc
		itemDisposeProc(mValue.mItemRef);
	}
}
