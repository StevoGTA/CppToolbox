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
		virtual									~TDictionaryInternals() {}

												// Instance methods
		virtual	CDictionary::KeyCount			getKeyCount() = 0;
		virtual	OR<CDictionary::Value>			getValue(const CString& key) = 0;
		virtual	CDictionaryInternals*			set(const CString& key, const CDictionary::Value& value) = 0;
		virtual	CDictionaryInternals*			remove(const CString& key) = 0;
		virtual	CDictionaryInternals*			remove(const TSet<CString>& keys) = 0;
		virtual	CDictionaryInternals*			removeAll() = 0;

		virtual	TIteratorS<CDictionary::Item>	getIterator() const = 0;

		virtual	CDictionary::ItemEqualsProc		getItemEqualsProc() const = 0;
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
			SDictionaryItemInfo(UInt32 keyHashValue, const CString& key, const CDictionary::Value& value) :
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
	CDictionary::Item		mItem;
	SDictionaryItemInfo*	mNextItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CStandardDictionaryInternals

class CStandardDictionaryInternals : public TDictionaryInternals<CStandardDictionaryInternals> {
	// IteratorInfo
	class IteratorInfo : public CIterator::Info {
		// Methods
		public:
								// Lifecycle methods
								IteratorInfo(const CStandardDictionaryInternals& internals, UInt32 initialReference) :
									CIterator::Info(),
											mInternals(internals), mInitialReference(initialReference),
											mCurrentIndex(0), mCurrentItemInfo(nil)
									{}
								IteratorInfo(const CStandardDictionaryInternals& internals, UInt32 initialReference,
										UInt32 currentIndex, SDictionaryItemInfo* curretItemInfo) :
									CIterator::Info(),
											mInternals(internals), mInitialReference(initialReference),
											mCurrentIndex(currentIndex), mCurrentItemInfo(curretItemInfo)
									{}

								// CIterator::Info methods
			CIterator::Info*	copy()
									{ return new IteratorInfo(mInternals, mInitialReference, mCurrentIndex,
											mCurrentItemInfo); }

		// Properties
		const	CStandardDictionaryInternals&	mInternals;
				UInt32							mInitialReference;
				UInt32							mCurrentIndex;
				SDictionaryItemInfo*			mCurrentItemInfo;
	};


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
				OR<CDictionary::Value>			getValue(const CString& key);
				CDictionaryInternals*			set(const CString& key, const CDictionary::Value& value);
				CDictionaryInternals*			remove(const CString& key);
				CDictionaryInternals*			remove(const TSet<CString>& keys);
				CDictionaryInternals*			removeAll();

				TIteratorS<CDictionary::Item>	getIterator() const;

				CDictionary::ItemEqualsProc		getItemEqualsProc() const;

												// Private methods
				void							removeAllInternal();
				void							remove(SDictionaryItemInfo* itemInfo, bool removeAll);

												// Class methods
		static	void*							iteratorAdvance(IteratorInfo& iteratorInfo);

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
OR<CDictionary::Value> CStandardDictionaryInternals::getValue(const CString& key)
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

	return (itemInfo != nil) ? OR<CDictionary::Value>(itemInfo->mItem.mValue) : OR<CDictionary::Value>();
}

//----------------------------------------------------------------------------------------------------------------------
CDictionaryInternals* CStandardDictionaryInternals::set(const CString& key, const CDictionary::Value& value)
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
CDictionaryInternals* CStandardDictionaryInternals::remove(const TSet<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	CStandardDictionaryInternals*	dictionaryInternals = (CStandardDictionaryInternals*) prepareForWrite();

	// Iterate keys
	for (TIteratorS<CString> iterator = keys.getIterator(); iterator.hasValue(); iterator.advance())
		// Remove this key
		dictionaryInternals->remove(*iterator);

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
TIteratorS<CDictionary::Item> CStandardDictionaryInternals::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	IteratorInfo*	iteratorInfo = new IteratorInfo(*this, mReference);

	// Find first item info
	while ((mItemInfos[iteratorInfo->mCurrentIndex] == nil) && (++iteratorInfo->mCurrentIndex < mItemInfosCount)) ;

	CDictionary::Item*	firstItem = nil;
	if (iteratorInfo->mCurrentIndex < mItemInfosCount) {
		// Have first item info
		iteratorInfo->mCurrentItemInfo = mItemInfos[iteratorInfo->mCurrentIndex];
		firstItem = &mItemInfos[iteratorInfo->mCurrentIndex]->mItem;
	}

	return TIteratorS<CDictionary::Item>(firstItem, (CIterator::AdvanceProc) iteratorAdvance, *iteratorInfo);
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
void* CStandardDictionaryInternals::iteratorAdvance(IteratorInfo& iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Internals check
	AssertFailIf(iteratorInfo.mInitialReference != iteratorInfo.mInternals.mReference);

	// Check for additional item info in linked list
	if (iteratorInfo.mCurrentItemInfo->mNextItemInfo != nil) {
		// Have next item info
		iteratorInfo.mCurrentItemInfo = iteratorInfo.mCurrentItemInfo->mNextItemInfo;
	} else {
		// End of item info linked list
		while ((++iteratorInfo.mCurrentIndex < iteratorInfo.mInternals.mItemInfosCount) &&
				(iteratorInfo.mInternals.mItemInfos [iteratorInfo.mCurrentIndex] == nil)) ;

		// Check if found another item info
		if (iteratorInfo.mCurrentIndex < iteratorInfo.mInternals.mItemInfosCount)
			// Found another item info
			iteratorInfo.mCurrentItemInfo = iteratorInfo.mInternals.mItemInfos[iteratorInfo.mCurrentIndex];
		else
			// No more item infos
			iteratorInfo.mCurrentItemInfo = nil;
	}

	return (iteratorInfo.mCurrentItemInfo != nil) ? (void*) &iteratorInfo.mCurrentItemInfo->mItem : nil;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProcsDictionaryInternals

class CProcsDictionaryInternals : public TDictionaryInternals<CProcsDictionaryInternals> {
	// Types
	class IteratorInfo : public CIterator::Info {
		// Methods
		public:
			// Lifecycle methods
			IteratorInfo(const CProcsDictionaryInternals& internals) : mInternals(internals), mCurrentIndex(0) {}
			IteratorInfo(const CProcsDictionaryInternals& internals, OI<CDictionary::Item> item, UInt32 currentIndex) :
				mInternals(internals), mItem(item), mCurrentIndex(currentIndex)
				{}

								// CIterator::Info methods
			CIterator::Info*	copy()
									{ return new IteratorInfo(mInternals, mItem, mCurrentIndex); }

		// Properties
		const	CProcsDictionaryInternals&	mInternals;
				OI<CDictionary::Item>		mItem;
				UInt32						mCurrentIndex;
	};

	// Methods
	public:
												// Lifecycle methods
												CProcsDictionaryInternals(const CDictionary::Procs& procs) :
													TDictionaryInternals(), mProcs(procs)
													{}
												CProcsDictionaryInternals(const CProcsDictionaryInternals& other) :
													TDictionaryInternals(), mProcs(other.mProcs)
													{}
												~CProcsDictionaryInternals()
													{ mProcs.disposeUserData(); }

												// TDictionaryInternals methods
				CDictionary::KeyCount			getKeyCount()
													{ return mProcs.getKeyCount(); }
				OR<CDictionary::Value>			getValue(const CString& key)
													{ return mProcs.getValue(key); }
				CDictionaryInternals*			set(const CString& key, const CDictionary::Value& value)
													{ mProcs.set(key, value); return (CDictionaryInternals*) this; }
				CDictionaryInternals*			remove(const CString& key)
													{
														// Remove key
														mProcs.removeKeys(TSet<CString>(key));

														return (CDictionaryInternals*) this;
													}
				CDictionaryInternals*			remove(const TSet<CString>& keys)
													{ mProcs.removeKeys(keys); return (CDictionaryInternals*) this; }
				CDictionaryInternals*			removeAll()
													{ mProcs.removeAll(); return (CDictionaryInternals*) this; }

				TIteratorS<CDictionary::Item>	getIterator() const
													{
														// Setup
														IteratorInfo*	iteratorInfo = new IteratorInfo(*this);
														if (mProcs.getKeyCount() > 0) {
															// Have content
															CString	key = mProcs.getKeyAtIndex(0);
															iteratorInfo->mItem =
																	OI<CDictionary::Item>(
																			CDictionary::Item(key,
																					*mProcs.getValue(key)));
														}

														return TIteratorS<CDictionary::Item>(&(*iteratorInfo->mItem),
																(CIterator::AdvanceProc) iteratorAdvance,
																*iteratorInfo);
													}

				CDictionary::ItemEqualsProc		getItemEqualsProc() const
													{ return nil; }

		static	void*							iteratorAdvance(IteratorInfo& iteratorInfo)
													{
														// Setup
														const	CDictionary::Procs&	procs =
																		iteratorInfo.mInternals.mProcs;

														// Advance
														if (++iteratorInfo.mCurrentIndex < procs.getKeyCount()) {
															// Have more keys
															CString	key =
																			procs.getKeyAtIndex(
																					iteratorInfo.mCurrentIndex);
															iteratorInfo.mItem =
																	OI<CDictionary::Item>(
																			CDictionary::Item(key,
																					*procs.getValue(key)));

															return &(*iteratorInfo.mItem);
														} else {
															// No more keys
															iteratorInfo.mItem = OI<CDictionary::Item>();

															return nil;
														}
													}

		CDictionary::Procs	mProcs;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary::Value

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(bool value) : mType(kBool), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(const TArray<CDictionary>& value) :
	mType(kArrayOfDictionaries), mValue(new TArray<CDictionary>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(const TArray<CString>& value) : mType(kArrayOfStrings), mValue(new TArray<CString>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(const CData& value) : mType(kData), mValue(new CData(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(const CDictionary& value) : mType(kDictionary), mValue(new CDictionary(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(const CString& value) : mType(kString), mValue(new CString(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(Float32 value) : mType(kFloat32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(Float64 value) : mType(kFloat64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(SInt8 value) : mType(kSInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(SInt16 value) : mType(kSInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(SInt32 value) : mType(kSInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(SInt64 value) : mType(kSInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(UInt8 value) : mType(kUInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(UInt16 value) : mType(kUInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(UInt32 value) : mType(kUInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(UInt64 value) : mType(kUInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(ItemRef value) : mType(kItemRef), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value::Value(const Value& other, ItemCopyProc itemCopyProc) : mType(other.mType), mValue(other.mValue)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for ItemCopyProc
	if (itemCopyProc != nil) {
		// Copy value
		if (mType == kArrayOfDictionaries)
			// Array of dictionaries
			mValue.mArrayOfDictionaries = new TNArray<CDictionary>(*mValue.mArrayOfDictionaries);
		else if (mType == kArrayOfStrings)
			// Array of strings
			mValue.mArrayOfStrings = new TNArray<CString>(*mValue.mArrayOfStrings);
		else if (mType == kData)
			// Data
			mValue.mData = new CData(*mValue.mData);
		else if (mType == kDictionary)
			// Dictionary
			mValue.mDictionary = new CDictionary(*mValue.mDictionary);
		else if (mType == kString)
			// String
			mValue.mString = new CString(*mValue.mString);
		else if ((mType == kItemRef) && (itemCopyProc != nil))
			// Item Ref and have item copy proc
			mValue.mItemRef = itemCopyProc(mValue.mItemRef);
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::Value::getBool(bool defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kBool:		return mValue.mBool;
		case kSInt8:	return mValue.mSInt8 == 1;
		case kSInt16:	return mValue.mSInt16 == 1;
		case kSInt32:	return mValue.mSInt32 == 1;
		case kSInt64:	return mValue.mSInt64 == 1;
		case kUInt8:	return mValue.mUInt8 == 1;
		case kUInt16:	return mValue.mUInt16 == 1;
		case kUInt32:	return mValue.mUInt32 == 1;
		case kUInt64:	return mValue.mUInt64 == 1;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CDictionary>& CDictionary::Value::getArrayOfDictionaries(const TArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kArrayOfDictionaries);

	return (mType == kArrayOfDictionaries) ? *mValue.mArrayOfDictionaries : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& CDictionary::Value::getArrayOfStrings(const TArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kArrayOfStrings);

	return (mType == kArrayOfStrings) ? *mValue.mArrayOfStrings : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CData& CDictionary::Value::getData(const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kData);

	return (mType == kData) ? *mValue.mData : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& CDictionary::Value::getDictionary(const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kDictionary);

	return (mType == kDictionary) ? *mValue.mDictionary : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CDictionary::Value::getString(const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kString);

	return (mType == kString) ? *mValue.mString : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CDictionary::Value::getFloat32(Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kFloat32:	return mValue.mFloat32;
		case kFloat64:	return (Float32) mValue.mFloat64;
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return (Float32) mValue.mSInt32;
		case kSInt64:	return (Float32) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return (Float32) mValue.mUInt32;
		case kUInt64:	return (Float32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CDictionary::Value::getFloat64(Float64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kFloat32:	return mValue.mFloat32;
		case kFloat64:	return mValue.mFloat64;
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (Float64) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (Float64) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CDictionary::Value::getSInt8(SInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return (SInt8) mValue.mSInt16;
		case kSInt32:	return (SInt8)mValue.mSInt32;
		case kSInt64:	return (SInt8)mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return (SInt8)mValue.mUInt16;
		case kUInt32:	return (SInt8)mValue.mUInt32;
		case kUInt64:	return (SInt8)mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CDictionary::Value::getSInt16(SInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (SInt16) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (SInt16) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CDictionary::Value::getSInt32(SInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (SInt32) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (SInt32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDictionary::Value::getSInt64(SInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CDictionary::Value::getUInt8(UInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return (UInt8) mValue.mSInt16;
		case kSInt32:	return (UInt8) mValue.mSInt32;
		case kSInt64:	return (UInt8) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return (UInt8) mValue.mUInt16;
		case kUInt32:	return (UInt8) mValue.mUInt32;
		case kUInt64:	return (UInt8) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CDictionary::Value::getUInt16(UInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (UInt16) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (UInt16) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CDictionary::Value::getUInt32(UInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (UInt32) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (UInt32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDictionary::Value::getUInt64(UInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::ItemRef CDictionary::Value::getItemRef() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kItemRef);

	return (mType == kItemRef) ? mValue.mItemRef : nil;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::Value::equals(const Value& other, ItemEqualsProc itemEqualsProc) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mType != other.mType)
		// Mismatch
		return false;

	switch (mType) {
		case kBool:
			// Bool
			return mValue.mBool == other.mValue.mBool;

		case kArrayOfDictionaries:
			// Array of Dictionaries
			return *mValue.mArrayOfDictionaries == *other.mValue.mArrayOfDictionaries;

		case kArrayOfStrings:
			// Array of Strings
			return *mValue.mArrayOfStrings == *other.mValue.mArrayOfStrings;

		case kData:
			// Data
			return *mValue.mData == *other.mValue.mData;

		case kDictionary:
			// Dictionary
			return *mValue.mDictionary == *other.mValue.mDictionary;

		case kString:
			// String
			return *mValue.mString == *other.mValue.mString;

		case kFloat32:
			// Float32
			return mValue.mFloat32 == other.mValue.mFloat32;

		case kFloat64:
			// Float64
			return mValue.mFloat64 == other.mValue.mFloat64;

		case kSInt8:
			// SInt8
			return mValue.mSInt8 == other.mValue.mSInt8;

		case kSInt16:
			// SInt16
			return mValue.mSInt16 == other.mValue.mSInt16;

		case kSInt32:
			// SInt32
			return mValue.mSInt32 == other.mValue.mSInt32;

		case kSInt64:
			// SInt64
			return mValue.mSInt64 == other.mValue.mSInt64;

		case kUInt8:
			// UInt8
			return mValue.mUInt8 == other.mValue.mUInt8;

		case kUInt16:
			// UInt16
			return mValue.mUInt16 == other.mValue.mUInt16;

		case kUInt32:
			// UInt32
			return mValue.mUInt32 == other.mValue.mUInt32;

		case kUInt64:
			// UInt64
			return mValue.mUInt64 == other.mValue.mUInt64;

		case kItemRef:
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
void CDictionary::Value::dispose(CDictionary::ItemDisposeProc itemDisposeProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mType == kArrayOfDictionaries) {
		// Array of dictionaries
		Delete(mValue.mArrayOfDictionaries);
	} else if (mType == kArrayOfStrings) {
		// Array of strings
		Delete(mValue.mArrayOfStrings);
	} else if (mType == kData) {
		// Data
		Delete(mValue.mData);
	} else if (mType == kDictionary) {
		// Dictionary
		Delete(mValue.mDictionary);
	} else if (mType == kString) {
		// String
		Delete(mValue.mString);
	} else if ((mType == kItemRef) && (itemDisposeProc != nil)) {
		// Item Ref and have item dispose proc
		itemDisposeProc(mValue.mItemRef);
	}
}

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
CDictionary::CDictionary(const Procs& procs) : CEquatable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = (CDictionaryInternals*) new CProcsDictionaryInternals(procs);
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
	for (TIteratorS<Item> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance())
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
const CDictionary::Value& CDictionary::getValue(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->getValue(key);
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::getBool(const CString& key, bool defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getBool(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CDictionary>& CDictionary::getArrayOfDictionaries(const CString& key,
		const TArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getArrayOfDictionaries(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& CDictionary::getArrayOfStrings(const CString& key, const TArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getArrayOfStrings(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CData& CDictionary::getData(const CString& key, const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getData(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& CDictionary::getDictionary(const CString& key, const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getDictionary(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CDictionary::getString(const CString& key, const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getString(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CDictionary::getFloat32(const CString& key, Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getFloat32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CDictionary::getFloat64(const CString& key, Float64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getFloat64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CDictionary::getSInt8(const CString& key, SInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt8(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CDictionary::getSInt16(const CString& key, SInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt16(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CDictionary::getSInt32(const CString& key, SInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDictionary::getSInt64(const CString& key, SInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CDictionary::getUInt8(const CString& key, UInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt8(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CDictionary::getUInt16(const CString& key, UInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt16(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CDictionary::getUInt32(const CString& key, UInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDictionary::getUInt64(const CString& key, UInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CDictionary>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CString>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CData& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CDictionary& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CString& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, ItemRef value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, Value(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const Value& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = (CDictionaryInternals*) mInternals->set(key, value);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const OI<CDictionary::Value>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for value
	if (value.hasInstance())
		// Have value
		mInternals = (CDictionaryInternals*) mInternals->set(key, *value);
	else
		// Don't have value
		mInternals = (CDictionaryInternals*) mInternals->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const OR<CDictionary::Value>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for value
	if (value.hasReference())
		// Have value
		mInternals = (CDictionaryInternals*) mInternals->set(key, *value);
	else
		// Don't have value
		mInternals = (CDictionaryInternals*) mInternals->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	mInternals = (CDictionaryInternals*) mInternals->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const TSet<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	mInternals = (CDictionaryInternals*) mInternals->remove(keys);
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
	for (TIteratorS<Item> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get value
		Item&		item = iterator.getValue();
		OR<Value>	value = other.mInternals->getValue(item.mKey);
		if (!value.hasReference() || !item.mValue.equals(*value, mInternals->getItemEqualsProc()))
			// Value not found or value is not the same
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
TIteratorS<CDictionary::Item> CDictionary::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getIterator();
}

//----------------------------------------------------------------------------------------------------------------------
const OR<CDictionary::Value> CDictionary::operator[](const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getValue(key);
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
	for (TIteratorS<Item> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance())
		// Set this value
		mInternals->set(iterator.getValue().mKey, iterator.getValue().mValue);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CDictionary::ItemRef> CDictionary::getItemRef(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<Value>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<ItemRef>(value->getItemRef()) : OV<ItemRef>();
}
