//----------------------------------------------------------------------------------------------------------------------
//	CSet.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSet.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSet::Internals

class CSet::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
		struct ItemInfo {
			public:
						ItemInfo(UInt32 hashValue, CHashable* hashable) :
							mHashValue(hashValue), mHashable(hashable), mNextItemInfo(nil)
							{}

				bool	doesMatch(UInt32 hashValue, const CHashable& hashable)
							{ return (hashValue == mHashValue) && (hashable == *mHashable); }

				UInt32		mHashValue;
				CHashable*	mHashable;
				ItemInfo*	mNextItemInfo;
		};

	public:
										Internals(CSet::CopyProc copyProc, CSet::DisposeProc disposeProc) :
											TCopyOnWriteReferenceCountable(),
													mCopyProc(copyProc), mDisposeProc(disposeProc),
													mCount(0), mReference(0), mItemInfosCount(16)
											{
												mItemInfos = (ItemInfo**) ::calloc(mItemInfosCount, sizeof(ItemInfo*));
											}
										Internals(const Internals& other) :
											TCopyOnWriteReferenceCountable(),
													mCopyProc(other.mCopyProc),
													mDisposeProc(other.mDisposeProc),
													mCount(other.mCount), mReference(0),
													mItemInfosCount(other.mItemInfosCount)
											{
												// Finish setup
												mItemInfos = (ItemInfo**) ::calloc(mItemInfosCount, sizeof(ItemInfo*));

												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Setup for this linked list
													ItemInfo*	otherItemInfo = other.mItemInfos[i];
													ItemInfo*	internalsItemInfo = nil;

													while (otherItemInfo != nil) {
														// Clone
														ItemInfo*	itemInfo =
																			new ItemInfo(
																					otherItemInfo->mHashValue,
																					(mCopyProc != nil) ?
																							mCopyProc(
																									*otherItemInfo->
																											mHashable) :
																							otherItemInfo->mHashable);

														// Check for first in the linked list
														if (internalsItemInfo == nil) {
															// First in this linked list
															mItemInfos[i] = itemInfo;
															internalsItemInfo = mItemInfos[i];
														} else {
															// Next one in this linked list
															internalsItemInfo->mNextItemInfo = itemInfo;
															internalsItemInfo = internalsItemInfo->mNextItemInfo;
														}

														// Next
														otherItemInfo = otherItemInfo->mNextItemInfo;
													}
												}
											}
										~Internals()
											{
												// Iterate all item infos
												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Check if have an item info
													if (mItemInfos[i] != nil)
														// Remove this chain
														remove(mItemInfos[i]);
												}

												::free(mItemInfos);
											}

						void			insert(const CHashable& hashable)
											{
												// Setup
												UInt32		hashValue = hashable.getHashValue();
												UInt32		index = hashValue & (mItemInfosCount - 1);
												ItemInfo*	itemInfo =
																	new ItemInfo(hashValue,
																			(mCopyProc != nil) ?
																					mCopyProc(hashable) :
																					(CHashable*) &hashable);

												// Find
												ItemInfo*	previousItemInfo = nil;
												ItemInfo*	currentItemInfo = mItemInfos[index];
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
														mItemInfos[index] = itemInfo;
													else
														// Add to the end
														previousItemInfo->mNextItemInfo = itemInfo;

													// Update info
													mCount++;
													mReference++;
												}
											}
						bool			contains(const CHashable& hashable)
											{
												// Setup
												UInt32	hashValue = hashable.getHashValue();
												UInt32	index = hashValue & (mItemInfosCount - 1);

												// Find item info that matches
												ItemInfo*	itemInfo = mItemInfos[index];
												while ((itemInfo != nil) && !itemInfo->doesMatch(hashValue, hashable))
													// Advance to next item info
													itemInfo = itemInfo->mNextItemInfo;

												return itemInfo != nil;
											}
						void			remove(const CHashable& hashable)
											{
												// Setup
												UInt32	hashValue = hashable.getHashValue();
												UInt32	index = hashValue & (mItemInfosCount - 1);

												// Find
												ItemInfo*	previousItemInfo = nil;
												ItemInfo*	currentItemInfo = mItemInfos[index];
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
														mItemInfos[index] = currentItemInfo->mNextItemInfo;
													else
														// Some other item info in linked list
														previousItemInfo->mNextItemInfo =
																currentItemInfo->mNextItemInfo;

													// Check if have dispose proc
													if (mDisposeProc != nil)
														// Dispose
														mDisposeProc(currentItemInfo->mHashable);

													// Cleanup this one
													Delete(currentItemInfo);

													// Update info
													mCount--;
													mReference++;
												}
											}
						void			removeAll()
											{
												// Iterate all item infos
												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Check if have an item info
													if (mItemInfos[i] != nil) {
														// Remove this chain
														remove(mItemInfos[i]);

														// Clear
														mItemInfos[i] = nil;
													}
												}

												// Update info
												mCount = 0;
												mReference++;
											}
						void			remove(ItemInfo* itemInfo)
											{
												// Check for next item info
												if (itemInfo->mNextItemInfo != nil)
													// Remove the next item info
													remove(itemInfo->mNextItemInfo);

												// Check if have dispose proc
												if (mDisposeProc != nil)
													// Dispose
													mDisposeProc(itemInfo->mHashable);

												// Cleanup this one
												Delete(itemInfo);
											}
				const	OR<CHashable>	getAny() const
											{
												// Find first item info
												UInt32	index = 0;
												while ((mItemInfos[index] == nil) && (++index < mItemInfosCount)) ;

												return (index < mItemInfosCount) ?
													OR<CHashable>(*mItemInfos[index]->mHashable) : OR<CHashable>();
											}

		CSet::CopyProc		mCopyProc;
		CSet::DisposeProc	mDisposeProc;

		CSet::ItemCount		mCount;
		UInt32				mReference;

		ItemInfo**			mItemInfos;
		UInt32				mItemInfosCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSet::IteratorInfo

struct CSet::IteratorInfo {
	public:
		IteratorInfo(const Internals& internals, UInt32 initialReference, UInt32 itemInfosIndex,
				Internals::ItemInfo* initialItemInfo) :
			mInternals(internals),
					mInitialReference(initialReference), mCurrentItemInfoIndex(itemInfosIndex),
					mCurrentItemInfo(initialItemInfo),
					mCurrentIndex(0)
			{}

		const	Internals&				mInternals;

				UInt32					mInitialReference;
				UInt32					mCurrentItemInfoIndex;
				Internals::ItemInfo*	mCurrentItemInfo;

				UInt32					mCurrentIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSet

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSet::CSet(CopyProc copyProc, DisposeProc disposeProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(copyProc, disposeProc);
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
CSet& CSet::insert(const CHashable& hashable)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Add hashable
	mInternals->insert(hashable);

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
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Remove hashable
	mInternals->remove(hashable);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CSet& CSet::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Remove all
	mInternals->removeAll();

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
const OR<CHashable> CSet::getAny() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getAny();
}

//----------------------------------------------------------------------------------------------------------------------
I<CSet::IteratorInfo> CSet::getIteratorInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Find first item info
	for (UInt32 i = 0; i < mInternals->mItemInfosCount; i++) {
		// Check if have item in this slot
		if (mInternals->mItemInfos[i] != nil)
			// Found first item
			return I<IteratorInfo>(new IteratorInfo(*mInternals, mInternals->mReference, i, mInternals->mItemInfos[i]));
	}

	return I<IteratorInfo>(new IteratorInfo(*mInternals, mInternals->mReference, 0, nil));
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

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CSet::iteratorGetCurrentIndex(const I<IteratorInfo>& iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	return iteratorInfo->mCurrentIndex;
}

//----------------------------------------------------------------------------------------------------------------------
CHashable* CSet::iteratorGetCurrentItem(const I<IteratorInfo>& iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	return (iteratorInfo->mCurrentItemInfo != nil) ? iteratorInfo->mCurrentItemInfo->mHashable : nil;
}

//----------------------------------------------------------------------------------------------------------------------
CHashable* CSet::iteratorAdvance(const I<IteratorInfo>& iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Internals check
	AssertFailIf(iteratorInfo->mInitialReference != iteratorInfo->mInternals.mReference);

	// Check for additional item info in linked list
	if (iteratorInfo->mCurrentItemInfo->mNextItemInfo != nil) {
		// Have next item info
		iteratorInfo->mCurrentItemInfo = iteratorInfo->mCurrentItemInfo->mNextItemInfo;
		iteratorInfo->mCurrentIndex++;
	} else {
		// End of item info linked list
		while ((++iteratorInfo->mCurrentItemInfoIndex < iteratorInfo->mInternals.mItemInfosCount) &&
				(iteratorInfo->mInternals.mItemInfos[iteratorInfo->mCurrentItemInfoIndex] == nil)) ;

		// Check if found another item info
		if (iteratorInfo->mCurrentItemInfoIndex < iteratorInfo->mInternals.mItemInfosCount) {
			// Found another item info
			iteratorInfo->mCurrentItemInfo = iteratorInfo->mInternals.mItemInfos[iteratorInfo->mCurrentItemInfoIndex];
			iteratorInfo->mCurrentIndex++;
		} else
			// No more item infos
			iteratorInfo->mCurrentItemInfo = nil;
	}

	return (iteratorInfo->mCurrentItemInfo != nil) ? iteratorInfo->mCurrentItemInfo->mHashable : nil;
}
