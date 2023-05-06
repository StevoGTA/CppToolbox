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
// MARK: - CSet::IteratorInfo

struct CSet::IteratorInfo : public CIterator::Info {
	// Methods
	public:
						// Lifecycle methods
						IteratorInfo(const Internals& internals, UInt32 initialReference) :
							CIterator::Info(),
									mInternals(internals), mInitialReference(initialReference), mCurrentIndex(0),
									mCurrentItemInfo(nil)
							{}

						// CIterator::Info methods
	CIterator::Info*	copy()
							{
								// Make copy
								IteratorInfo*	iteratorInfo = new IteratorInfo(mInternals, mInitialReference);
								iteratorInfo->mCurrentIndex = mCurrentIndex;
								iteratorInfo->mCurrentItemInfo = mCurrentItemInfo;

								return iteratorInfo;
							}

	// Properties
	const	Internals&		mInternals;
			UInt32			mInitialReference;
			UInt32			mCurrentIndex;
			SSetItemInfo*	mCurrentItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSet::Internals

class CSet::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
										Internals(CSet::CopyProc copyProc, CSet::DisposeProc disposeProc) :
											TCopyOnWriteReferenceCountable(),
													mCopyProc(copyProc), mDisposeProc(disposeProc),
													mCount(0), mReference(0), mItemInfosCount(16)
											{
												mItemInfos =
														(SSetItemInfo**)
																::calloc(mItemInfosCount, sizeof(SSetItemInfo*));
											}
										Internals(const Internals& other) :
											TCopyOnWriteReferenceCountable(),
													mCopyProc(other.mCopyProc), mDisposeProc(other.mDisposeProc),
													mCount(other.mCount), mReference(0),
													mItemInfosCount(other.mItemInfosCount)
											{
												// Finish setup
												mItemInfos =
														(SSetItemInfo**)
																::calloc(mItemInfosCount, sizeof(SSetItemInfo*));

												for (UInt32 i = 0; i < mItemInfosCount; i++) {
													// Setup for this linked list
													SSetItemInfo*	itemInfo = other.mItemInfos[i];
													SSetItemInfo*	internalsItemInfo = nil;

													while (itemInfo != nil) {
														// Clone
														SSetItemInfo*	setItemInfo =
																				new SSetItemInfo(itemInfo->mHashValue,
																						(mCopyProc != nil) ?
																								*mCopyProc(itemInfo->mHashable) :
																								itemInfo->mHashable);

														// Check for first in the linked list
														if (internalsItemInfo == nil) {
															// First in this linked list
															mItemInfos[i] = setItemInfo;
															internalsItemInfo = mItemInfos[i];
														} else {
															// Next one in this linked list
															internalsItemInfo->mNextItemInfo = setItemInfo;
															internalsItemInfo = internalsItemInfo->mNextItemInfo;
														}
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

				Internals*				insert(const CHashable& hashable)
											{
												// Prepare for write
												Internals*	internals = prepareForWrite();

												// Setup
												UInt32			hashValue = CHasher::getValueForHashable(hashable);
												UInt32			index = hashValue & (internals->mItemInfosCount - 1);
												SSetItemInfo*	setItemInfo =
																		new SSetItemInfo(hashValue,
																				(mCopyProc != nil) ?
																						*mCopyProc(hashable) :
																						hashable);

												// Find
												SSetItemInfo*	previousItemInfo = nil;
												SSetItemInfo*	currentItemInfo = internals->mItemInfos[index];
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
														internals->mItemInfos[index] = setItemInfo;
													else
														// Add to the end
														previousItemInfo->mNextItemInfo = setItemInfo;

													// Update info
													internals->mCount++;
													internals->mReference++;
												}

												return internals;
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
				Internals*				remove(const CHashable& hashable)
											{
												// Prepare for write
												Internals*	internals = prepareForWrite();

												// Setup
												UInt32	hashValue = CHasher::getValueForHashable(hashable);
												UInt32	index = hashValue & (internals->mItemInfosCount - 1);

												// Find
												SSetItemInfo*	previousItemInfo = nil;
												SSetItemInfo*	currentItemInfo = internals->mItemInfos[index];
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
														internals->mItemInfos[index] =
																currentItemInfo->mNextItemInfo;
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
													internals->mCount--;
													internals->mReference++;
												}

												return internals;
											}
				Internals*				removeAll()
											{
												// Prepare for write
												Internals*	internals = prepareForWrite();

												// Iterate all item infos
												for (UInt32 i = 0; i < internals->mItemInfosCount; i++) {
													// Check if have an item info
													if (internals->mItemInfos[i] != nil) {
														// Remove this chain
														remove(internals->mItemInfos[i]);

														// Clear
														internals->mItemInfos[i] = nil;
													}
												}

												// Update info
												internals->mCount = 0;
												internals->mReference++;

												return internals;
											}
				void					remove(SSetItemInfo* itemInfo)
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

				TIteratorS<CHashable>	getIterator() const
											{
												// Setup
												IteratorInfo*	iteratorInfo = new IteratorInfo(*this, mReference);

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

												return TIteratorS<CHashable>(firstValue, iteratorAdvance,
														*iteratorInfo);
											}

		static	void*					iteratorAdvance(CIterator::Info& iteratorInfo)
											{
												// Setup
												IteratorInfo&	setIteratorInfo = (IteratorInfo&) iteratorInfo;

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

		CSet::CopyProc		mCopyProc;
		CSet::DisposeProc	mDisposeProc;

		CSet::ItemCount		mCount;
		UInt32				mReference;

		SSetItemInfo**		mItemInfos;
		UInt32				mItemInfosCount;
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
CHashable& CSet::copy(const CHashable& hashable) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (mInternals->mCopyProc != nil) ? *mInternals->mCopyProc(hashable) : (CHashable&) hashable;
}

//----------------------------------------------------------------------------------------------------------------------
CSet& CSet::insert(const CHashable& hashable)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add hashable
	mInternals = mInternals->insert(hashable);

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
