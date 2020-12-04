//----------------------------------------------------------------------------------------------------------------------
//	CSet.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSet.h"

#include "CppToolboxAssert.h"
#include "TReferenceTracking.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SSetItemInfo

struct SSetItemInfo {
			// Lifecycle methods
			SSetItemInfo(UInt32 hashValue, const CHashable& hashable) :
				mHashValue(hashValue), mHashable(hashable), mNextItemInfo(nil)
				{}
			SSetItemInfo(const SSetItemInfo& other) :
				mHashValue(other.mHashValue), mHashable(other.mHashable), mNextItemInfo(nil)
				{}

			// Instance methods
	bool	doesMatch(UInt32 hashValue, const CHashable& hashable)
				{ return (hashValue == mHashValue) && (hashable == mHashable); }

	// Properties
			UInt32			mHashValue;
	const	CHashable&		mHashable;
			SSetItemInfo*	mNextItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSetIteratorInfo

struct CSetIteratorInfo : public CIterator::Info {
	// Methods
	public:
						// Lifecycle methods
						CSetIteratorInfo(const CSetInternals& internals, UInt32 initialReference) :
							CIterator::Info(),
									mInternals(internals), mInitialReference(initialReference), mCurrentIndex(0),
									mCurrentItemInfo(nil)
							{}

						// CIterator::Info methods
	CIterator::Info*	copy()
							{
								// Make copy
								CSetIteratorInfo*	iteratorInfo = new CSetIteratorInfo(mInternals, mInitialReference);
								iteratorInfo->mCurrentIndex = mCurrentIndex;
								iteratorInfo->mCurrentItemInfo = mCurrentItemInfo;

								return iteratorInfo;
							}

	// Properties
	const	CSetInternals&	mInternals;
			UInt32			mInitialReference;
			UInt32			mCurrentIndex;
			SSetItemInfo*	mCurrentItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSetInternals

class CSetInternals : public TCopyOnWriteReferenceCountable<CSetInternals> {
	public:
										CSetInternals(bool ownsItems) :
											TCopyOnWriteReferenceCountable(),
													mOwnsItems(ownsItems), mCount(0), mReference(0), mItemInfosCount(16)
											{
												mItemInfos =
														(SSetItemInfo**)
																::calloc(mItemInfosCount, sizeof(SSetItemInfo*));
											}
										CSetInternals(const CSetInternals& other) :
											TCopyOnWriteReferenceCountable(),
													mOwnsItems(other.mOwnsItems), mCount(other.mCount), mReference(0),
													mItemInfosCount(other.mItemInfosCount)
											{
												// Ensure we do not own items
												AssertFailIf(mOwnsItems && (mCount > 0));

												// Finish setup
												mItemInfos =
														(SSetItemInfo**)
																::calloc(mItemInfosCount, sizeof(SSetItemInfo*));

												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Setup for this linked list
													SSetItemInfo*	itemInfo = other.mItemInfos[i];
													SSetItemInfo*	setInternalsItemInfo = nil;

													while (itemInfo != nil) {
														// Check for first in the linked list
														if (setInternalsItemInfo == nil) {
															// First in this linked list
															mItemInfos[i] = new SSetItemInfo(*itemInfo);
															setInternalsItemInfo = mItemInfos[i];
														} else {
															// Next one in this linked list
															setInternalsItemInfo->mNextItemInfo =
																	new SSetItemInfo(*itemInfo);
															setInternalsItemInfo =
																	setInternalsItemInfo->mNextItemInfo;
														}
													}
												}
											}
										~CSetInternals()
											{
												// Iterate all item infos
												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Check if have an item info
													if (mItemInfos[i] != nil)
														// Remove this chain
														remove(mItemInfos[i], mOwnsItems);
												}

												::free(mItemInfos);
											}

				CSetInternals*			insert(const CHashable& hashable)
											{
												// Prepare for write
												CSetInternals*	setInternals = prepareForWrite();

												// Setup
												UInt32	hashValue = CHasher::getValueForHashable(hashable);
												UInt32	index = hashValue & (setInternals->mItemInfosCount - 1);

												// Find
												SSetItemInfo*	previousItemInfo = nil;
												SSetItemInfo*	currentItemInfo = setInternals->mItemInfos[index];
												while ((currentItemInfo != nil) &&
														!currentItemInfo->doesMatch(hashValue, hashable)) {
													// Next in linked list
													previousItemInfo = currentItemInfo;
													currentItemInfo = currentItemInfo->mNextItemInfo;
												}

												// Check results
												if (currentItemInfo == nil) {
													// Did not find
													if (previousItemInfo == nil)
														// First one
														setInternals->mItemInfos[index] =
																new SSetItemInfo(hashValue, hashable);
													else
														// Add to the end
														previousItemInfo->mNextItemInfo =
																new SSetItemInfo(hashValue, hashable);

													// Update info
													setInternals->mCount++;
													setInternals->mReference++;
												} else if (setInternals->mOwnsItems) {
													// Did not add
													const	CHashable*	tempHashable = &hashable;
													Delete(tempHashable);
												}

												return setInternals;
											}
				bool					contains(const CHashable& hashable)
											{
												// Setup
												UInt32	hashValue = CHasher::getValueForHashable(hashable);
												UInt32	index = hashValue & (mItemInfosCount - 1);

												// Find item info that matches
												SSetItemInfo*	itemInfo = mItemInfos[index];
												while ((itemInfo != nil) && !itemInfo->doesMatch(hashValue, hashable))
													// Advance to next item info
													itemInfo = itemInfo->mNextItemInfo;

												return itemInfo != nil;
											}
				CSetInternals*			remove(const CHashable& hashable)
											{
												// Prepare for write
												CSetInternals*	setInternals = prepareForWrite();

												// Setup
												UInt32	hashValue = CHasher::getValueForHashable(hashable);
												UInt32	index = hashValue & (setInternals->mItemInfosCount - 1);

												// Find
												SSetItemInfo*	previousItemInfo = nil;
												SSetItemInfo*	currentItemInfo = setInternals->mItemInfos[index];
												while ((currentItemInfo != nil) &&
														!currentItemInfo->doesMatch(hashValue, hashable)) {
													// Next in linked list
													previousItemInfo = currentItemInfo;
													currentItemInfo = currentItemInfo->mNextItemInfo;
												}

												// Check if found
												if (currentItemInfo != nil) {
													// Remove
													if (previousItemInfo == nil)
														// First in linked list
														setInternals->mItemInfos[index] =
																currentItemInfo->mNextItemInfo;
													else
														// Some other item info in linked list
														previousItemInfo->mNextItemInfo =
																currentItemInfo->mNextItemInfo;

													// Check if owns item
													if (setInternals->mOwnsItems) {
														// Dispose
														const	CHashable*	tempHashable = &currentItemInfo->mHashable;
														Delete(tempHashable);
													}

													// Cleanup this one
													Delete(currentItemInfo);

													// Update info
													setInternals->mCount--;
													setInternals->mReference++;
												}

												return setInternals;
											}
				CSetInternals*			removeAll()
											{
												// Prepare for write
												CSetInternals*	setInternals = prepareForWrite();

												// Iterate all item infos
												for (UInt32 i = 0; i < setInternals->mItemInfosCount; i++) {
													// Check if have an item info
													if (setInternals->mItemInfos[i] != nil) {
														// Remove this chain
														remove(setInternals->mItemInfos[i], mOwnsItems);

														// Clear
														setInternals->mItemInfos[i] = nil;
													}
												}

												// Update info
												setInternals->mCount = 0;
												setInternals->mReference++;

												return setInternals;
											}
				void					remove(SSetItemInfo* itemInfo, bool ownsItems)
											{
												// Check for next item info
												if (itemInfo->mNextItemInfo != nil)
													// Remove the next item info
													remove(itemInfo->mNextItemInfo, ownsItems);

												// Check if owns item
												if (ownsItems) {
													// Dispose
													const	CHashable*	hashable = &itemInfo->mHashable;
													Delete(hashable);
												}

												// Cleanup this one
												Delete(itemInfo);
											}

				TIteratorS<CHashable>	getIterator() const
											{
												// Setup
												CSetIteratorInfo*	iteratorInfo =
																			new CSetIteratorInfo(*this, mReference);

												// Find first item info
												while ((mItemInfos[iteratorInfo->mCurrentIndex] == nil) &&
														(++iteratorInfo->mCurrentIndex < mItemInfosCount)) ;

												CHashable*	firstValue = nil;
												if (iteratorInfo->mCurrentIndex < mItemInfosCount) {
													// Have first item info
													iteratorInfo->mCurrentItemInfo =
															mItemInfos[iteratorInfo->mCurrentIndex];
													firstValue =
															(CHashable*)
																	&mItemInfos[iteratorInfo->mCurrentIndex]->mHashable;
												}

												return TIteratorS<CHashable>(firstValue, iteratorAdvance, *iteratorInfo);
											}

		static	void*					iteratorAdvance(CIterator::Info& iteratorInfo)
											{
												// Setup
												CSetIteratorInfo&	setIteratorInfo = (CSetIteratorInfo&) iteratorInfo;

												// Internals check
												AssertFailIf(setIteratorInfo.mInitialReference !=
														setIteratorInfo.mInternals.mReference);

												// Check for additional item info in linked list
												if (setIteratorInfo.mCurrentItemInfo->mNextItemInfo != nil) {
													// Have next item info
													setIteratorInfo.mCurrentItemInfo =
															setIteratorInfo.mCurrentItemInfo->mNextItemInfo;
												} else {
													// End of item info linked list
													while ((++setIteratorInfo.mCurrentIndex <
																	setIteratorInfo.mInternals.mItemInfosCount) &&
															(setIteratorInfo.mInternals.mItemInfos
																	[setIteratorInfo.mCurrentIndex] ==
																	nil)) ;

													// Check if found another item info
													if (setIteratorInfo.mCurrentIndex <
															setIteratorInfo.mInternals.mItemInfosCount)
														// Found another item info
														setIteratorInfo.mCurrentItemInfo =
																setIteratorInfo.mInternals
																		.mItemInfos[setIteratorInfo.mCurrentIndex];
													else
														// No more item infos
														setIteratorInfo.mCurrentItemInfo = nil;
												}

												return (setIteratorInfo.mCurrentItemInfo != nil) ?
														(void*) &setIteratorInfo.mCurrentItemInfo->mHashable : nil;
											}

		bool			mOwnsItems;
		CSet::ItemCount	mCount;
		UInt32			mReference;

		SSetItemInfo**	mItemInfos;
		UInt32			mItemInfosCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSet

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSet::CSet(bool ownsItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CSetInternals(ownsItems);
}

//----------------------------------------------------------------------------------------------------------------------
CSet::CSet(const CSet& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CSet::~CSet()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CSet& CSet::add(const CHashable* hashable)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add hashable
	mInternals = mInternals->insert(*hashable);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
bool CSet::contains(const CHashable& hashable) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->contains(hashable);
}

//----------------------------------------------------------------------------------------------------------------------
CSet::ItemCount CSet::getCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCount;
}

//----------------------------------------------------------------------------------------------------------------------
CSet& CSet::remove(const CHashable& hashable)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove hashable
	mInternals = mInternals->remove(hashable);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CSet& CSet::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove all
	mInternals = mInternals->removeAll();

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
TIteratorS<CHashable> CSet::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getIterator();
}

//----------------------------------------------------------------------------------------------------------------------
CSet& CSet::apply(ApplyProc applyProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all hashables
	for (TIteratorS<CHashable> iterator = getIterator(); iterator.hasValue(); iterator.advance())
		// Call proc
		applyProc(iterator.getValue(), userData);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CSet& CSet::operator=(const CSet& other)
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
