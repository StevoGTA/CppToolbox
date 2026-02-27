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
	public:
												TDictionaryInternals() : TCopyOnWriteReferenceCountable<T>() {}
												TDictionaryInternals(const TDictionaryInternals& other) :
													TCopyOnWriteReferenceCountable<T>()
													{}
		virtual									~TDictionaryInternals() {}

		virtual	CDictionary::Count				getCount() const = 0;
		virtual	OR<SValue>						getValue(const CString& key) const = 0;
		virtual	void							set(const CString& key, const SValue& value) = 0;
		virtual	void							remove(const CString& key) = 0;
		virtual	void							remove(const TSet<CString>& keys) = 0;
		virtual	void							removeAll() = 0;

		virtual	I<CDictionary::IteratorInfo>	getIteratorInfo() const = 0;

		virtual	void							prepareForWrite(TDictionaryInternals<T>** internals) = 0;
		virtual	SValue::OpaqueEqualsProc		getOpaqueEqualsProc() const = 0;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary::Internals

class CDictionary::Internals : public TDictionaryInternals<CDictionary::Internals> {};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary::Internals

class CDictionary::IteratorInfo {
	public:
		virtual						~IteratorInfo() {}

		virtual			UInt32		getCurrentIndex() const = 0;
		virtual	const	CString&	getCurrentKey() const = 0;
		virtual			SValue*		getCurrentValue() const = 0;
		virtual			void		advance() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary::Iterator

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
	return mIteratorInfo->getCurrentValue() != nil;
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
	return (SValue&) *mIteratorInfo->getCurrentValue();
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
// MARK: - CStandardDictionaryInternals

class CStandardDictionaryInternals : public TDictionaryInternals<CStandardDictionaryInternals> {
	class IteratorInfo : public CDictionary::IteratorInfo {
		public:
								IteratorInfo(const CStandardDictionaryInternals& internals,
										UInt32 initialReference, UInt32 itemInfosIndex,
										SDictionaryItemInfo* initialItemInfo) :
									CDictionary::IteratorInfo(),
											mInternals(internals), mInitialReference(initialReference),
											mCurrentItemInfoIndex(itemInfosIndex), mCurrentItemInfo(initialItemInfo),
											mCurrentIndex(0)
									{}

					UInt32		getCurrentIndex() const
									{ return mCurrentIndex; }
			const	CString&	getCurrentKey() const
									{ return mCurrentItemInfo->mItem.getKey(); }
					SValue*		getCurrentValue() const
									{ return (mCurrentItemInfo != nil) ? &mCurrentItemInfo->mItem.getValue() : nil; }
					void		advance()
									{
										// Internals check
										AssertFailIf(mInitialReference != mInternals.mReference);

										// Check for additional item info in linked list
										if (mCurrentItemInfo->mNextItemInfo != nil) {
											// Have next item info
											mCurrentItemInfo = mCurrentItemInfo->mNextItemInfo;
											mCurrentIndex++;
										} else {
											// End of item info linked list
											while ((++mCurrentItemInfoIndex < mInternals.mItemInfosCount) &&
													(mInternals.mItemInfos[mCurrentItemInfoIndex] == nil)) ;

											// Check if found another item info
											if (mCurrentItemInfoIndex < mInternals.mItemInfosCount) {
												// Found another item info
												mCurrentItemInfo = mInternals.mItemInfos[mCurrentItemInfoIndex];
												mCurrentIndex++;
											} else
												// No more item infos
												mCurrentItemInfo = nil;
										}
									}

		private:
			const	CStandardDictionaryInternals&	mInternals;

					UInt32							mInitialReference;
					UInt32							mCurrentItemInfoIndex;
					SDictionaryItemInfo*			mCurrentItemInfo;

					UInt32							mCurrentIndex;
	};


	public:
												CStandardDictionaryInternals(SValue::OpaqueCopyProc opaqueCopyProc,
														SValue::OpaqueEqualsProc opaqueEqualsProc,
														SValue::OpaqueDisposeProc opaqueDisposeProc);
												CStandardDictionaryInternals(const CStandardDictionaryInternals& other);
												~CStandardDictionaryInternals();

				CStandardDictionaryInternals*	copy() const;

				CDictionary::Count				getCount() const;
				OR<SValue>						getValue(const CString& key) const;
				void							set(const CString& key, const SValue& value);
				void							remove(const CString& key);
				void							remove(const TSet<CString>& keys);
				void							removeAll();

				I<CDictionary::IteratorInfo>	getIteratorInfo() const;

				void							prepareForWrite(
														TDictionaryInternals<CStandardDictionaryInternals>** internals);
				SValue::OpaqueEqualsProc		getOpaqueEqualsProc() const;

				void							removeAllInternal();
				void							remove(SDictionaryItemInfo* itemInfo, bool removeAll);

		SValue::OpaqueCopyProc		mOpaqueCopyProc;
		SValue::OpaqueEqualsProc	mOpaqueEqualsProc;
		SValue::OpaqueDisposeProc	mOpaqueDisposeProc;

		CDictionary::Count			mCount;
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
CDictionary::Count CStandardDictionaryInternals::getCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mCount;
}

//----------------------------------------------------------------------------------------------------------------------
OR<SValue> CStandardDictionaryInternals::getValue(const CString& key) const
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

	return (itemInfo != nil) ? OR<SValue>(itemInfo->mItem.getValue()) : OR<SValue>();
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
		currentItemInfo->mItem.getValue() = SValue(value, mOpaqueCopyProc);
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
	for (TSet<CString>::Iterator iterator = keys.getIterator(); iterator; iterator++)
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
I<CDictionary::IteratorInfo> CStandardDictionaryInternals::getIteratorInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Find first item info
	for (UInt32 i = 0; i < mItemInfosCount; i++) {
		// Check if have item in this slot
		if (mItemInfos[i] != nil)
			// Found first item
			return I<CDictionary::IteratorInfo>(
					new CStandardDictionaryInternals::IteratorInfo(*this, mReference, i, mItemInfos[i]));
	}

	return I<CDictionary::IteratorInfo>(new CStandardDictionaryInternals::IteratorInfo(*this, mReference, 0, nil));
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
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProcsDictionaryInternals

class CProcsDictionaryInternals : public TDictionaryInternals<CProcsDictionaryInternals> {
	class IteratorInfo : public CDictionary::IteratorInfo {
		public:
								IteratorInfo(const CProcsDictionaryInternals& internals,
										const OI<CDictionary::Item>& initialItem) :
									CDictionary::IteratorInfo(),
											mInternals(internals),
											mItem(initialItem),
											mCurrentIndex(0)
									{}

					UInt32		getCurrentIndex() const
									{ return mCurrentIndex; }
			const	CString&	getCurrentKey() const
									{ return mItem->getKey(); }
					SValue*		getCurrentValue() const
									{ return mItem.hasInstance() ? &mItem->getValue() : nil; }
					void		advance()
									{
										// Advance
										if (++mCurrentIndex < mInternals.mProcs.getCount()) {
											// Have more keys
											CString	key = mInternals.mProcs.getKeyAtIndex(mCurrentIndex);
											mItem.setInstance(
													new CDictionary::Item(key, *mInternals.mProcs.getValue(key)));
										} else
											// No more keys
											mItem.setInstance();
									}

		private:
			const	CProcsDictionaryInternals&	mInternals;

					OI<CDictionary::Item>		mItem;

					UInt32						mCurrentIndex;
	};

	public:
										CProcsDictionaryInternals(const CDictionary::Procs& procs) :
											TDictionaryInternals(), mProcs(procs)
											{}
										CProcsDictionaryInternals(const CProcsDictionaryInternals& other) :
											TDictionaryInternals(), mProcs(other.mProcs)
											{}
										~CProcsDictionaryInternals()
											{ mProcs.disposeUserData(); }

										// TDictionaryInternals methods
		CDictionary::Count				getCount() const
											{ return mProcs.getCount(); }
		OR<SValue>						getValue(const CString& key) const
											{ return mProcs.getValue(key); }
		void							set(const CString& key, const SValue& value)
											{ mProcs.set(key, value); }
		void							remove(const CString& key)
											{ mProcs.removeKeys(TNSet<CString>(key)); }
		void							remove(const TSet<CString>& keys)
											{ mProcs.removeKeys(keys); }
		void							removeAll()
											{ mProcs.removeAll(); }

		I<CDictionary::IteratorInfo>	getIteratorInfo() const
											{
												// Check count
												if (mProcs.getCount() > 0) {
													// Have items
													CString	key = mProcs.getKeyAtIndex(0);

													return I<CDictionary::IteratorInfo>(
															new CProcsDictionaryInternals::IteratorInfo(*this,
																	OI<CDictionary::Item>(
																			new CDictionary::Item(key,
																					*mProcs.getValue(key)))));
												} else
													// Empty
													return I<CDictionary::IteratorInfo>(
															new CProcsDictionaryInternals::IteratorInfo(*this,
																	OI<CDictionary::Item>()));
											}

		void							prepareForWrite(
												TDictionaryInternals<CProcsDictionaryInternals>** internals)
											{ TCopyOnWriteReferenceCountable<CProcsDictionaryInternals>::
														prepareForWrite((CProcsDictionaryInternals**) internals); }
		SValue::OpaqueEqualsProc		getOpaqueEqualsProc() const
											{ return nil; }

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
	mInternals = (Internals*) new CStandardDictionaryInternals(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const Procs& procs) : CEquatable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = (Internals*) new CProcsDictionaryInternals(procs);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const CDictionary& other) : CEquatable()
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
CDictionary::Count CDictionary::getCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getCount();
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
OV<bool> CDictionary::getOVBool(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<bool>(value->getBool()) : OV<bool>();
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
OV<TArray<CDictionary> > CDictionary::getOVArrayOfDictionaries(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ?
			OV<TArray<CDictionary> >(value->getArrayOfDictionaries()) : OV<TArray<CDictionary> >();
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
OV<TArray<CString> > CDictionary::getOVArrayOfStrings(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<TArray<CString> >(value->getArrayOfStrings()) : OV<TArray<CString> >();
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
OV<CData> CDictionary::getOVData(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	OR<SValue>	value = mInternals->getValue(key);

	return value.hasReference() ? OV<CData>(value->getData()) : OV<CData>();
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
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CDictionary>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArray<CString>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CData& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CDictionary& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CString& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SValue::Opaque value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, SValue(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const SValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Set
	mInternals->set(key, value);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Remove
	mInternals->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const TArray<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Remove
	mInternals->remove(TNSet<CString>(keys));
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const TSet<CString>& keys)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

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
	mInternals->prepareForWrite((TDictionaryInternals<Internals> **) &mInternals);

	// Remove all
	mInternals->removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::equals(const CDictionary& other, void* itemCompareProcUserData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check count
	if (mInternals->getCount() != other.mInternals->getCount())
		// Counts differ
		return false;

	// Iterate all items
	for (Iterator iterator = getIterator(); iterator; iterator++) {
		// Get value
		OR<SValue>	value = other.mInternals->getValue(iterator.getKey());
		if (!value.hasReference() || !iterator.getValue().equals(*value, mInternals->getOpaqueEqualsProc()))
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
	for (Iterator iterator = other.getIterator(); iterator; iterator++)
		// Set this value
		dictionary.mInternals->set(iterator.getKey(), iterator.getValue());

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary& CDictionary::operator+=(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all items
	for (Iterator iterator = getIterator(); iterator; iterator++)
		// Set this value
		mInternals->set(iterator.getKey(), iterator.getValue());

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
I<CDictionary::IteratorInfo> CDictionary::getIteratorInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getIteratorInfo();
}
