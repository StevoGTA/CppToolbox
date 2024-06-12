//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.cpp			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDictionary.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"
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
		virtual	OR<SValue>						getValue(const CString& key) = 0;
		virtual	void							set(const CString& key, const SValue& value) = 0;
		virtual	void							remove(const CString& key) = 0;
		virtual	void							remove(const TSet<CString>& keys) = 0;
		virtual	void							removeAll() = 0;

		virtual	TIteratorS<CDictionary::Item>	getIterator() const = 0;

		virtual	void							prepareForWrite(TDictionaryInternals<T>** internals) = 0;
		virtual	SValue::OpaqueEqualsProc		getOpaqueEqualsProc() const = 0;
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
			SDictionaryItemInfo(UInt32 keyHashValue, const CString& key, const SValue& value) :
				mKeyHashValue(keyHashValue), mItem(key, value), mNextItemInfo(nil)
				{}
			SDictionaryItemInfo(const SDictionaryItemInfo& itemInfo, SValue::OpaqueCopyProc opaqueCopyProc) :
				mKeyHashValue(itemInfo.mKeyHashValue), mItem(itemInfo.mItem, opaqueCopyProc), mNextItemInfo(nil)
				{}

			// Instance methods
	bool	doesMatch(UInt32 hashValue, const CString& key)
				{ return (hashValue == mKeyHashValue) && (key == mItem.mKey); }
	void	disposeValue(SValue::OpaqueDisposeProc opaqueDisposeProc)
				{ mItem.mValue.dispose(opaqueDisposeProc); }

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
												CStandardDictionaryInternals(SValue::OpaqueCopyProc opaqueCopyProc,
														SValue::OpaqueEqualsProc opaqueEqualsProc,
														SValue::OpaqueDisposeProc opaqueDisposeProc);
												CStandardDictionaryInternals(const CStandardDictionaryInternals& other);
												~CStandardDictionaryInternals();

												// TCopyOnWriteReferenceCountable methods
				CStandardDictionaryInternals*	copy() const;

												// TDictionaryInternals methods
				CDictionary::KeyCount			getKeyCount();
				OR<SValue>						getValue(const CString& key);
				void							set(const CString& key, const SValue& value);
				void							remove(const CString& key);
				void							remove(const TSet<CString>& keys);
				void							removeAll();

				TIteratorS<CDictionary::Item>	getIterator() const;

				void							prepareForWrite(
														TDictionaryInternals<CStandardDictionaryInternals>** internals);
				SValue::OpaqueEqualsProc		getOpaqueEqualsProc() const;

												// Private methods
				void							removeAllInternal();
				void							remove(SDictionaryItemInfo* itemInfo, bool removeAll);

												// Class methods
		static	void*							iteratorAdvance(IteratorInfo& iteratorInfo);

		SValue::OpaqueCopyProc		mOpaqueCopyProc;
		SValue::OpaqueEqualsProc	mOpaqueEqualsProc;
		SValue::OpaqueDisposeProc	mOpaqueDisposeProc;

		CDictionary::KeyCount		mCount;
		SDictionaryItemInfo**		mItemInfos;
		UInt32						mItemInfosCount;
		UInt32						mReference;
};

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CStandardDictionaryInternals::CStandardDictionaryInternals(SValue::OpaqueCopyProc opaqueCopyProc,
		SValue::OpaqueEqualsProc opaqueEqualsProc, SValue::OpaqueDisposeProc opaqueDisposeProc) :
	TDictionaryInternals(),
			mOpaqueCopyProc(opaqueCopyProc), mOpaqueEqualsProc(opaqueEqualsProc), mOpaqueDisposeProc(opaqueDisposeProc),
			mCount(0), mItemInfosCount(16), mReference(0)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mItemInfos = (SDictionaryItemInfo**) ::calloc(mItemInfosCount, sizeof(SDictionaryItemInfo*));
}

//----------------------------------------------------------------------------------------------------------------------
CStandardDictionaryInternals::CStandardDictionaryInternals(const CStandardDictionaryInternals& other) :
	TDictionaryInternals(other),
			mOpaqueCopyProc(other.mOpaqueCopyProc), mOpaqueEqualsProc(other.mOpaqueEqualsProc),
					mOpaqueDisposeProc(other.mOpaqueDisposeProc),
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
				mItemInfos[i] = new SDictionaryItemInfo(*itemInfo, mOpaqueCopyProc);
				dictionaryInternalsItemInfo = mItemInfos[i];
			} else {
				// Next one in this linked list
				dictionaryInternalsItemInfo->mNextItemInfo = new SDictionaryItemInfo(*itemInfo, mOpaqueCopyProc);
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
OR<SValue> CStandardDictionaryInternals::getValue(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	hashValue = key.getHashValue();
	UInt32	index = hashValue & (mItemInfosCount - 1);

	// Find item info that matches
	SDictionaryItemInfo*	itemInfo = mItemInfos[index];
	while ((itemInfo != nil) && !itemInfo->doesMatch(hashValue, key))
		// Advance to next item info
		itemInfo = itemInfo->mNextItemInfo;

	return (itemInfo != nil) ? OR<SValue>(itemInfo->mItem.mValue) : OR<SValue>();
}

//----------------------------------------------------------------------------------------------------------------------
void CStandardDictionaryInternals::set(const CString& key, const SValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	hashValue = key.getHashValue();
	UInt32	index = hashValue & (mItemInfosCount - 1);

	// Find
	SDictionaryItemInfo*	previousItemInfo = nil;
	SDictionaryItemInfo*	currentItemInfo = mItemInfos[index];
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
			mItemInfos[index] = new SDictionaryItemInfo(hashValue, key, value);
		else
			// Add to the end
			previousItemInfo->mNextItemInfo = new SDictionaryItemInfo(hashValue, key, value);

		// Update info
		mCount++;
		mReference++;
	} else {
		// Did find a match
		currentItemInfo->disposeValue(mOpaqueDisposeProc);
		currentItemInfo->mItem.mValue = value;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CStandardDictionaryInternals::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	hashValue = key.getHashValue();
	UInt32	index = hashValue & (mItemInfosCount - 1);

	// Find
	SDictionaryItemInfo*	previousItemInfo = nil;
	SDictionaryItemInfo*	currentItemInfo = mItemInfos[index];
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
			mItemInfos[index] = currentItemInfo->mNextItemInfo;
		else
			// Not the first item info
			previousItemInfo->mNextItemInfo = currentItemInfo->mNextItemInfo;

		// Cleanup
		remove(currentItemInfo, false);

		// Update info
		mCount--;
		mReference++;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CStandardDictionaryInternals::remove(const TSet<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate keys
	for (TIteratorS<CString> iterator = keys.getIterator(); iterator.hasValue(); iterator.advance())
		// Remove this key
		remove(*iterator);
}

//----------------------------------------------------------------------------------------------------------------------
void CStandardDictionaryInternals::removeAll()
//----------------------------------------------------------------------------------------------------------------------
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
void CStandardDictionaryInternals::prepareForWrite(TDictionaryInternals<CStandardDictionaryInternals>** internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	TCopyOnWriteReferenceCountable<CStandardDictionaryInternals>::prepareForWrite(
			(CStandardDictionaryInternals**) internals);
}

//----------------------------------------------------------------------------------------------------------------------
SValue::OpaqueEqualsProc CStandardDictionaryInternals::getOpaqueEqualsProc() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mOpaqueEqualsProc;
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
	itemInfo->disposeValue(mOpaqueDisposeProc);

	Delete(itemInfo);
}

//----------------------------------------------------------------------------------------------------------------------
void* CStandardDictionaryInternals::iteratorAdvance(IteratorInfo& iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Internals check
	AssertFailIf(iteratorInfo.mInitialReference != iteratorInfo.mInternals.mReference);

	// Check for additional item info in linked list
	if (iteratorInfo.mCurrentItemInfo->mNextItemInfo != nil)
		// Have next item info
		iteratorInfo.mCurrentItemInfo = iteratorInfo.mCurrentItemInfo->mNextItemInfo;
	else {
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
				OR<SValue>						getValue(const CString& key)
													{ return mProcs.getValue(key); }
				void							set(const CString& key, const SValue& value)
													{ mProcs.set(key, value); }
				void							remove(const CString& key)
													{ mProcs.removeKeys(TNSet<CString>(key)); }
				void							remove(const TSet<CString>& keys)
													{ mProcs.removeKeys(keys); }
				void							removeAll()
													{ mProcs.removeAll(); }

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

				void							prepareForWrite(
														TDictionaryInternals<CProcsDictionaryInternals>** internals)
													{
														// Prepare for write
														TCopyOnWriteReferenceCountable<CProcsDictionaryInternals>::
																prepareForWrite(
																		(CProcsDictionaryInternals**) internals);
													}
				SValue::OpaqueEqualsProc		getOpaqueEqualsProc() const
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
// MARK: - CDictionary

// MARK: Properties

const	CDictionary	CDictionary::mEmpty;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(SValue::OpaqueCopyProc opaqueCopyProc, SValue::OpaqueEqualsProc opaqueEqualsProc,
		SValue::OpaqueDisposeProc opaqueDisposeProc) : CEquatable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			(CDictionaryInternals*)
					new CStandardDictionaryInternals(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc);
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
	TNSet<CString>	keys;

	// Iterate all items
	for (TIteratorS<Item> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance())
		// Add key
		keys.insert(iterator->mKey);

	return keys;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::contains(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getValue(key).hasReference();
}

//----------------------------------------------------------------------------------------------------------------------
const SValue& CDictionary::getValue(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->getValue(key);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SValue> CDictionary::getOValue(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<SValue>(*value) : OV<SValue>();
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::getBool(const CString& key, bool defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getBool(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CDictionary>& CDictionary::getArrayOfDictionaries(const CString& key,
		const TArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getArrayOfDictionaries(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& CDictionary::getArrayOfStrings(const CString& key, const TArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getArrayOfStrings(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CData& CDictionary::getData(const CString& key, const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getData(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& CDictionary::getDictionary(const CString& key, const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getDictionary(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CDictionary> CDictionary::getOVDictionary(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<CDictionary>(value->getDictionary()) : OV<CDictionary>();
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CDictionary::getString(const CString& key, const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getString(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CDictionary::getOVString(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<CString>(value->getString()) : OV<CString>();
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CDictionary::getFloat32(const CString& key, Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getFloat32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<Float32> CDictionary::getOVFloat32(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<Float32>(value->getFloat32()) : OV<Float32>();
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CDictionary::getFloat64(const CString& key, Float64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getFloat64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<Float64> CDictionary::getOVFloat64(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<Float64>(value->getFloat64()) : OV<Float64>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CDictionary::getSInt8(const CString& key, SInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt8(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt8> CDictionary::getOVSInt8(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<SInt8>(value->getSInt8()) : OV<SInt8>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CDictionary::getSInt16(const CString& key, SInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt16(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt16> CDictionary::getOVSInt16(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<SInt16>(value->getSInt16()) : OV<SInt16>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CDictionary::getSInt32(const CString& key, SInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt32> CDictionary::getOVSInt32(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<SInt32>(value->getSInt32()) : OV<SInt32>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDictionary::getSInt64(const CString& key, SInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getSInt64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt64> CDictionary::getOVSInt64(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<SInt64>(value->getSInt64()) : OV<SInt64>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CDictionary::getUInt8(const CString& key, UInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt8(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt8> CDictionary::getOVUInt8(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<UInt8>(value->getUInt8()) : OV<UInt8>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CDictionary::getUInt16(const CString& key, UInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt16(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt16> CDictionary::getOVUInt16(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<UInt16>(value->getUInt16()) : OV<UInt16>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CDictionary::getUInt32(const CString& key, UInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt32(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt32> CDictionary::getOVUInt32(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<UInt32>(value->getUInt32()) : OV<UInt32>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDictionary::getUInt64(const CString& key, UInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? value->getUInt64(defaultValue) : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt64> CDictionary::getOVUInt64(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<UInt64>(value->getUInt64()) : OV<UInt64>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SValue::Opaque> CDictionary::getOpaque(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<SValue::Opaque>(value->getOpaque()) : OV<SValue::Opaque>();
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CDictionary>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CString>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CData& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CDictionary& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CString& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SValue::Opaque value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const SValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Set
	mInternals->set(key, value);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Remove
	mInternals->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const TArray<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Remove
	mInternals->remove(TNSet<CString>(keys));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const TSet<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Remove
	mInternals->remove(keys);
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
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<CDictionaryInternals> **) &mInternals);

	// Remove all
	mInternals->removeAll();
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
		OR<SValue>	value = other.mInternals->getValue(iterator->mKey);
		if (!value.hasReference() || !iterator->mValue.equals(*value, mInternals->getOpaqueEqualsProc()))
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
const OR<SValue> CDictionary::operator[](const CString& key) const
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
CDictionary CDictionary::operator+(const CDictionary& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	dictionary(*this);

	// Iterate all items
	for (TIteratorS<Item> iterator = other.mInternals->getIterator(); iterator.hasValue(); iterator.advance())
		// Set this value
		dictionary.mInternals->set(iterator->mKey, iterator->mValue);

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary& CDictionary::operator+=(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all items
	for (TIteratorS<Item> iterator = other.mInternals->getIterator(); iterator.hasValue(); iterator.advance())
		// Set this value
		mInternals->set(iterator->mKey, iterator->mValue);

	return *this;
}
