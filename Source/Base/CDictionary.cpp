//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.cpp			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDictionary.h"

#include "CppToolboxAssert.h"

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
		virtual	CDictionaryKeyCount			getKeyCount() = 0;
		virtual	OR<SDictionaryValue>		getValue(const CString& key) = 0;
		virtual	CDictionaryInternals*		set(const CString& key, const SDictionaryValue& value) = 0;
		virtual	CDictionaryInternals*		remove(const CString& key) = 0;
		virtual	CDictionaryInternals*		removeAll() = 0;

		virtual	TIteratorS<SDictionaryItem>	getIterator() const = 0;

		virtual	CDictionaryItemEqualsProc	getItemEqualsProc() const = 0;
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
			SDictionaryItemInfo(const SDictionaryItemInfo& itemInfo, CDictionaryItemCopyProc itemCopyProc) :
				mKeyHashValue(itemInfo.mKeyHashValue), mItem(itemInfo.mItem, itemCopyProc), mNextItemInfo(nil)
				{}

			// Instance methods
	bool	doesMatch(UInt32 hashValue, const CString& key)
				{ return (hashValue == mKeyHashValue) && (key == mItem.mKey); }
	void	disposeValue(CDictionaryItemDisposeProc itemDisposeProc)
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

struct CDictionaryIteratorInfo : public CIteratorInfo {
	// Methods
	public:
						// Lifecycle methods
						CDictionaryIteratorInfo(const CStandardDictionaryInternals& internals,
								UInt32 initialReference) :
							CIteratorInfo(), mInternals(internals), mInitialReference(initialReference),
									mCurrentIndex(0), mCurrentItemInfo(nil)
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
												CStandardDictionaryInternals(CDictionaryItemCopyProc itemCopyProc,
														CDictionaryItemDisposeProc itemDisposeProc,
														CDictionaryItemEqualsProc itemEqualsProc);
												CStandardDictionaryInternals(const CStandardDictionaryInternals& other);
												~CStandardDictionaryInternals();

												// TCopyOnWriteReferenceCountable methods
				CStandardDictionaryInternals*	copy() const;

												// TDictionaryInternals methods
				CDictionaryKeyCount				getKeyCount();
				OR<SDictionaryValue>			getValue(const CString& key);
				CDictionaryInternals*			set(const CString& key, const SDictionaryValue& value);
				CDictionaryInternals*			remove(const CString& key);
				CDictionaryInternals*			removeAll();

				TIteratorS<SDictionaryItem>		getIterator() const;

				CDictionaryItemEqualsProc		getItemEqualsProc() const;

												// Private methods
				void							removeAllInternal();
				void							remove(SDictionaryItemInfo* itemInfo, bool removeAll);

												// Class methods
		static	void*							iteratorAdvance(CIteratorInfo& iteratorInfo);

		CDictionaryItemCopyProc		mItemCopyProc;
		CDictionaryItemDisposeProc	mItemDisposeProc;
		CDictionaryItemEqualsProc	mItemEqualsProc;

		CDictionaryKeyCount			mCount;
		SDictionaryItemInfo**		mItemInfos;
		UInt32						mItemInfosCount;
		UInt32						mReference;
};

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CStandardDictionaryInternals::CStandardDictionaryInternals(CDictionaryItemCopyProc itemCopyProc,
		CDictionaryItemDisposeProc itemDisposeProc, CDictionaryItemEqualsProc itemEqualsProc) :
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
CDictionaryKeyCount CStandardDictionaryInternals::getKeyCount()
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
CDictionaryItemEqualsProc CStandardDictionaryInternals::getItemEqualsProc() const
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
void* CStandardDictionaryInternals::iteratorAdvance(CIteratorInfo& iteratorInfo)
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
											CProcsDictionaryInternals(const SDictionaryProcsInfo& procsInfo) :
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
				CDictionaryKeyCount			getKeyCount()
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
CIteratorInfo*	iteratorInfo = nil;
return TIteratorS<SDictionaryItem>(nil, nil, *iteratorInfo);
												}

				CDictionaryItemEqualsProc	getItemEqualsProc() const
												{ return nil; }

		SDictionaryProcsInfo	mProcsInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary

CDictionary	CDictionary::mEmpty;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(CDictionaryItemCopyProc itemCopyProc, CDictionaryItemDisposeProc itemDisposeProc,
		CDictionaryItemEqualsProc itemEqualsProc) : CEquatable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			(CDictionaryInternals*) new CStandardDictionaryInternals(itemCopyProc, itemDisposeProc, itemEqualsProc);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const SDictionaryProcsInfo& procsInfo) : CEquatable()
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
CDictionaryKeyCount CDictionary::getKeyCount() const
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
void CDictionary::set(const CString& key, CDictionaryItemRef value)
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
OV<CDictionaryItemRef> CDictionary::getItemRef(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SDictionaryValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<CDictionaryItemRef>(value->getItemRef()) : OV<CDictionaryItemRef>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SDictionaryValue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(bool value) :
	mValueType(kDictionaryValueTypeBool), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const TArray<CDictionary>& value) :
	mValueType(kDictionaryValueTypeArrayOfDictionaries),
	mValue(new TArray<CDictionary>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const TArray<CString>& value) :
	mValueType(kDictionaryValueTypeArrayOfStrings), mValue(new TArray<CString>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const CData& value) :
	mValueType(kDictionaryValueTypeData), mValue(new CData(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const CDictionary& value) :
	mValueType(kDictionaryValueTypeDictionary), mValue(new CDictionary(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const CString& value) :
	mValueType(kDictionaryValueTypeString), mValue(new CString(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(Float32 value) :
	mValueType(kDictionaryValueTypeFloat32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(Float64 value) :
	mValueType(kDictionaryValueTypeFloat64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(SInt8 value) :
	mValueType(kDictionaryValueTypeSInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(SInt16 value) :
	mValueType(kDictionaryValueTypeSInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(SInt32 value) :
	mValueType(kDictionaryValueTypeSInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(SInt64 value) :
	mValueType(kDictionaryValueTypeSInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(UInt8 value) :
	mValueType(kDictionaryValueTypeUInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(UInt16 value) :
	mValueType(kDictionaryValueTypeUInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(UInt32 value) :
	mValueType(kDictionaryValueTypeUInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(UInt64 value) :
	mValueType(kDictionaryValueTypeUInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(CDictionaryItemRef value) :
	mValueType(kDictionaryValueTypeItemRef), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const SDictionaryValue& other) :
	mValueType(other.mValueType), mValue(other.mValue)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SDictionaryValue::SDictionaryValue(const SDictionaryValue& other, CDictionaryItemCopyProc itemCopyProc) :
	mValueType(other.mValueType), mValue(other.mValue)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mValueType == kDictionaryValueTypeArrayOfDictionaries)
		// Array of dictionaries
		mValue.mArrayOfDictionaries = new TNArray<CDictionary>(*mValue.mArrayOfDictionaries);
	else if (mValueType == kDictionaryValueTypeArrayOfStrings)
		// Array of strings
		mValue.mArrayOfStrings = new TNArray<CString>(*mValue.mArrayOfStrings);
	else if (mValueType == kDictionaryValueTypeData)
		// Data
		mValue.mData = new CData(*mValue.mData);
	else if (mValueType == kDictionaryValueTypeDictionary)
		// Dictionary
		mValue.mDictionary = new CDictionary(*mValue.mDictionary);
	else if (mValueType == kDictionaryValueTypeString)
		// String
		mValue.mString = new CString(*mValue.mString);
	else if ((mValueType == kDictionaryValueTypeItemRef) && (itemCopyProc != nil))
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
		case kDictionaryValueTypeBool:		return mValue.mBool;
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8 == 1;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16 == 1;
		case kDictionaryValueTypeSInt32:	return mValue.mSInt32 == 1;
		case kDictionaryValueTypeSInt64:	return mValue.mSInt64 == 1;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8 == 1;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16 == 1;
		case kDictionaryValueTypeUInt32:	return mValue.mUInt32 == 1;
		case kDictionaryValueTypeUInt64:	return mValue.mUInt64 == 1;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
const TNArray<CDictionary>& SDictionaryValue::getArrayOfDictionaries(const TNArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kDictionaryValueTypeArrayOfDictionaries);

	return (mValueType == kDictionaryValueTypeArrayOfDictionaries) ? *mValue.mArrayOfDictionaries : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TNArray<CString>& SDictionaryValue::getArrayOfStrings(const TNArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kDictionaryValueTypeArrayOfStrings);

	return (mValueType == kDictionaryValueTypeArrayOfStrings) ? *mValue.mArrayOfStrings : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CData& SDictionaryValue::getData(const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kDictionaryValueTypeData);

	return (mValueType == kDictionaryValueTypeData) ? *mValue.mData : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& SDictionaryValue::getDictionary(const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kDictionaryValueTypeDictionary);

	return (mValueType == kDictionaryValueTypeDictionary) ? *mValue.mDictionary : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& SDictionaryValue::getString(const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kDictionaryValueTypeString);

	return (mValueType == kDictionaryValueTypeString) ? *mValue.mString : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 SDictionaryValue::getFloat32(Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeFloat32:	return mValue.mFloat32;
		case kDictionaryValueTypeFloat64:	return (Float32) mValue.mFloat64;
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return (Float32) mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (Float32) mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return (Float32) mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (Float32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Float64 SDictionaryValue::getFloat64(Float64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeFloat32:	return mValue.mFloat32;
		case kDictionaryValueTypeFloat64:	return mValue.mFloat64;
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (Float64) mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (Float64) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 SDictionaryValue::getSInt8(SInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return (SInt8) mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return (SInt8)mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (SInt8)mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return (SInt8)mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return (SInt8)mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (SInt8)mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 SDictionaryValue::getSInt16(SInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (SInt16) mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (SInt16) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 SDictionaryValue::getSInt32(SInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (SInt32) mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (SInt32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 SDictionaryValue::getSInt64(SInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 SDictionaryValue::getUInt8(UInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return (UInt8) mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return (UInt8) mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (UInt8) mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return (UInt8) mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return (UInt8) mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (UInt8) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 SDictionaryValue::getUInt16(UInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (UInt16) mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (UInt16) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 SDictionaryValue::getUInt32(UInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (UInt32) mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (UInt32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 SDictionaryValue::getUInt64(UInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mValueType) {
		case kDictionaryValueTypeSInt8:		return mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CDictionaryItemRef SDictionaryValue::getItemRef() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mValueType != kDictionaryValueTypeItemRef);

	return (mValueType == kDictionaryValueTypeItemRef) ? mValue.mItemRef : nil;
}

//----------------------------------------------------------------------------------------------------------------------
bool SDictionaryValue::equals(const SDictionaryValue& other, CDictionaryItemEqualsProc itemEqualsProc) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mValueType != other.mValueType)
		// Mismatch
		return false;

	switch (mValueType) {
		case kDictionaryValueTypeBool:
			// Bool
			return mValue.mBool == other.mValue.mBool;

		case kDictionaryValueTypeArrayOfDictionaries:
			// Array of Dictionaries
			return *mValue.mArrayOfDictionaries == *other.mValue.mArrayOfDictionaries;

		case kDictionaryValueTypeArrayOfStrings:
			// Array of Strings
			return *mValue.mArrayOfStrings == *other.mValue.mArrayOfStrings;

		case kDictionaryValueTypeData:
			// Data
			return *mValue.mData == *other.mValue.mData;

		case kDictionaryValueTypeDictionary:
			// Dictionary
			return *mValue.mDictionary == *other.mValue.mDictionary;

		case kDictionaryValueTypeString:
			// String
			return *mValue.mString == *other.mValue.mString;

		case kDictionaryValueTypeFloat32:
			// Float32
			return mValue.mFloat32 == other.mValue.mFloat32;

		case kDictionaryValueTypeFloat64:
			// Float64
			return mValue.mFloat64 == other.mValue.mFloat64;

		case kDictionaryValueTypeSInt8:
			// SInt8
			return mValue.mSInt8 == other.mValue.mSInt8;

		case kDictionaryValueTypeSInt16:
			// SInt16
			return mValue.mSInt16 == other.mValue.mSInt16;

		case kDictionaryValueTypeSInt32:
			// SInt32
			return mValue.mSInt32 == other.mValue.mSInt32;

		case kDictionaryValueTypeSInt64:
			// SInt64
			return mValue.mSInt64 == other.mValue.mSInt64;

		case kDictionaryValueTypeUInt8:
			// UInt8
			return mValue.mUInt8 == other.mValue.mUInt8;

		case kDictionaryValueTypeUInt16:
			// UInt16
			return mValue.mUInt16 == other.mValue.mUInt16;

		case kDictionaryValueTypeUInt32:
			// UInt32
			return mValue.mUInt32 == other.mValue.mUInt32;

		case kDictionaryValueTypeUInt64:
			// UInt64
			return mValue.mUInt64 == other.mValue.mUInt64;

		case kDictionaryValueTypeItemRef:
			// ItemRef
			return (itemEqualsProc != nil) ?
					itemEqualsProc(mValue.mItemRef, other.mValue.mItemRef) : mValue.mItemRef == other.mValue.mItemRef;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void SDictionaryValue::dispose(CDictionaryItemDisposeProc itemDisposeProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mValueType == kDictionaryValueTypeArrayOfDictionaries) {
		// Array of dictionaries
		Delete(mValue.mArrayOfDictionaries);
	} else if (mValueType == kDictionaryValueTypeArrayOfStrings) {
		// Array of strings
		Delete(mValue.mArrayOfStrings);
	} else if (mValueType == kDictionaryValueTypeData) {
		// Data
		Delete(mValue.mData);
	} else if (mValueType == kDictionaryValueTypeDictionary) {
		// Dictionary
		Delete(mValue.mDictionary);
	} else if (mValueType == kDictionaryValueTypeString) {
		// String
		Delete(mValue.mString);
	} else if ((mValueType == kDictionaryValueTypeItemRef) && (itemDisposeProc != nil)) {
		// Item Ref and have item dispose proc
		itemDisposeProc(mValue.mItemRef);
	}
}
