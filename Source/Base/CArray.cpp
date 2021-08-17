//----------------------------------------------------------------------------------------------------------------------
//	CArray.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CArray.h"

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SArraySortInfo

struct SArraySortInfo {
	// Properties
	CArray::CompareProc	mCompareProc;
	void*				mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CArrayIteratorInfo

class CArrayIteratorInfo : public CIterator::Info {
	// Methods
	public:
						// Lifecycle methods
						CArrayIteratorInfo(const CArrayInternals& internals, UInt32 initialReference) :
							CIterator::Info(),
									mInternals(internals), mInitialReference(initialReference), mCurrentIndex(0)
							{}

						// CIterator::Info methods
	CIterator::Info*	copy()
							{
								// Make copy
								CArrayIteratorInfo*	iteratorInfo =
															new CArrayIteratorInfo(mInternals, mInitialReference);
								iteratorInfo->mCurrentIndex = mCurrentIndex;

								return iteratorInfo;
							}

	// Properties
	const	CArrayInternals&	mInternals;
			UInt32				mInitialReference;
			UInt32				mCurrentIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CArrayInternals

class CArrayInternals : public TCopyOnWriteReferenceCountable<CArrayInternals> {
	public:
											CArrayInternals(CArray::ItemCount initialCapacity,
													CArray::CopyProc copyProc, CArray::DisposeProc disposeProc) :
												TCopyOnWriteReferenceCountable(),
														mCapacity(std::max(initialCapacity, (UInt32) 10)), mCount(0),
														mItemRefs(
																(CArray::ItemRef*)
																		::calloc(mCapacity, sizeof(CArray::ItemRef))),
														mCopyProc(copyProc),
														mDisposeProc(disposeProc), mReference(0)
												{}
											CArrayInternals(const CArrayInternals& other) :
												TCopyOnWriteReferenceCountable(),
														mCapacity(other.mCount), mCount(other.mCount),
														mItemRefs(
																(CArray::ItemRef*)
																		::calloc(mCapacity, sizeof(CArray::ItemRef))),
														mCopyProc(other.mCopyProc), mDisposeProc(other.mDisposeProc),
														mReference(0)
												{
													// Check if have copy proc
													if (mCopyProc != nil) {
														// Copy each item
														for (CArray::ItemIndex i = 0; i < mCount; i++)
															// Copy item
															mItemRefs[i] = mCopyProc(other.mItemRefs[i]);
													} else
														// Copy item refs
														::memcpy(mItemRefs, other.mItemRefs,
																mCount * sizeof(CArray::ItemRef));
												}
											~CArrayInternals()
												{
													// Remove all
													removeAllInternal();

													// Cleanup
													::free(mItemRefs);
												}

				OV<CArray::ItemIndex>		getIndexOf(const CArray::ItemRef itemRef) const
												{
													// Scan
													CArray::ItemIndex	itemIndex = 0;
													for (CArray::ItemRef* testItemRef = mItemRefs; itemIndex < mCount;
															itemIndex++, testItemRef++) {
														// Check test item ref
														if (*testItemRef == itemRef)
															// Found
															return OV<CArray::ItemIndex>(itemIndex);
													}

													return OV<CArray::ItemIndex>();
												}

				CArrayInternals*			append(const CArray::ItemRef* itemRefs, CArray::ItemCount count)
												{
													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													// Setup
													CArray::ItemCount	neededCount = arrayInternals->mCount + count;

													// Check storage
													if (neededCount > arrayInternals->mCapacity) {
														// Expand storage
														arrayInternals->mCapacity =
																std::max(neededCount, arrayInternals->mCapacity * 2);
														arrayInternals->mItemRefs =
																(CArray::ItemRef*)
																		::realloc(arrayInternals->mItemRefs,
																				arrayInternals->mCapacity *
																						sizeof(CArray::ItemRef));
													}

													// Append itemRefs into place
													::memcpy(arrayInternals->mItemRefs + arrayInternals->mCount,
															itemRefs, count * sizeof(CArray::ItemRef));
													arrayInternals->mCount = neededCount;

													// Update info
													arrayInternals->mReference++;

													return arrayInternals;
												}
				CArrayInternals*			insertAtIndex(const CArray::ItemRef itemRef, CArray::ItemIndex itemIndex)
												{
													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													// Setup
													CArray::ItemCount	neededCount = arrayInternals->mCount + 1;

													// Check storage
													if (neededCount > arrayInternals->mCapacity) {
														// Expand storage
														arrayInternals->mCapacity =
																std::max(neededCount, arrayInternals->mCapacity * 2);
														arrayInternals->mItemRefs =
																(CArray::ItemRef*)
																		::realloc(mItemRefs,
																				arrayInternals->mCapacity *
																						sizeof(CArray::ItemRef));
													}

													// Move following itemRefs back
													::memmove(arrayInternals->mItemRefs + itemIndex + 1,
															arrayInternals->mItemRefs + itemIndex,
															(arrayInternals->mCount - itemIndex) *
																	sizeof(CArray::ItemRef));

													// Store new itemRef
													arrayInternals->mItemRefs[itemIndex] = itemRef;
													arrayInternals->mCount++;

													// Update info
													arrayInternals->mReference++;

													return arrayInternals;
												}
				CArrayInternals*			removeAtIndex(CArray::ItemIndex itemIndex, bool performDispose)
												{
													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													// Check if owns items
													if (performDispose && (mDisposeProc != nil))
														// Dispose
														mDisposeProc(arrayInternals->mItemRefs[itemIndex]);

													// Move following itemRefs forward
													::memmove(arrayInternals->mItemRefs + itemIndex,
															arrayInternals->mItemRefs + itemIndex + 1,
															(arrayInternals->mCount - itemIndex - 1) *
																	sizeof(CArray::ItemRef));

													// Update info
													arrayInternals->mCount--;
													arrayInternals->mReference++;

													return arrayInternals;
												}
				CArrayInternals*			removeAll()
												{
													// Check if empty
													if (mCount == 0)
														// Nothing to remove
														return this;

													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													// Remove all
													arrayInternals->removeAllInternal();

													// Update info
													arrayInternals->mCount = 0;
													arrayInternals->mReference++;

													return arrayInternals;
												}
				void						removeAllInternal()
												{
													// Check if have item dispose proc
													if (mDisposeProc != nil) {
														// Dispose each item
														for (CArray::ItemIndex i = 0; i < mCount; i++) {
															// Dispose
															mDisposeProc(mItemRefs[i]);
														}
													}
												}

				TIteratorS<CArray::ItemRef>	getIterator() const
												{
													// Setup
													CArrayIteratorInfo*	iteratorInfo =
																				new CArrayIteratorInfo(*this,
																						mReference);

													return TIteratorS<CArray::ItemRef>((mCount > 0) ? mItemRefs : nil,
															(CIterator::AdvanceProc) iteratorAdvance, *iteratorInfo);
												}

		static	void*						iteratorAdvance(CArrayIteratorInfo& arrayIteratorInfo)
												{
													return (++arrayIteratorInfo.mCurrentIndex <
																	arrayIteratorInfo.mInternals.mCount) ?
															arrayIteratorInfo.mInternals.mItemRefs +
																	arrayIteratorInfo.mCurrentIndex :
															nil;
												}

	public:
		CArray::ItemCount	mCapacity;
		CArray::ItemCount	mCount;
		CArray::ItemRef*	mItemRefs;
		CArray::CopyProc	mCopyProc;
		CArray::DisposeProc	mDisposeProc;
		UInt32				mReference;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	int sSortProc(void* info, const void* itemRef1, const void* itemRef2);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CArray

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CArray::CArray(ItemCount initialCapacity, CopyProc copyProc, DisposeProc disposeProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CArrayInternals(initialCapacity, copyProc, disposeProc);
}

//----------------------------------------------------------------------------------------------------------------------
CArray::CArray(const CArray& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CArray::~CArray()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CArray::ItemRef CArray::copy(const ItemRef itemRef) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (mInternals->mCopyProc != nil) ? mInternals->mCopyProc(itemRef) : itemRef;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::add(const ItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);

	// Add item
	mInternals = mInternals->append(&itemRef, 1);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::addFrom(const CArray& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	if (other.mInternals->mCount == 0)
		// Nothing to add
		return *this;

	// Add items
	mInternals = mInternals->append(other.mInternals->mItemRefs, other.mInternals->mCount);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
bool CArray::contains(const ItemRef itemRef) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);

	// Scan
	ItemIndex	itemIndex = 0;
	for (ItemRef* testItemRef = mInternals->mItemRefs; itemIndex < mInternals->mCount; itemIndex++, testItemRef++) {
		// Check test item ref
		if (*testItemRef == itemRef)
			// Found
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CArray::ItemIndex> CArray::getIndexOf(const ItemRef itemRef) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);

	return mInternals->getIndexOf(itemRef);
}

//----------------------------------------------------------------------------------------------------------------------
CArray::ItemRef CArray::getItemAt(ItemIndex itemIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);

	return mInternals->mItemRefs[itemIndex];
}

//----------------------------------------------------------------------------------------------------------------------
CArray::ItemRef CArray::getLast() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Sanity check
	AssertFailIf(mInternals->mCount == 0);

	return mInternals->mItemRefs[mInternals->mCount - 1];
}

//----------------------------------------------------------------------------------------------------------------------
CArray::ItemCount CArray::getCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCount;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::insertAtIndex(const ItemRef itemRef, ItemIndex itemIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);
	AssertFailIf(itemIndex > mInternals->mCount);

	// Insert at index
	mInternals = mInternals->insertAtIndex(itemRef, itemIndex);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::detach(const ItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	OV<ItemIndex>	itemIndex = getIndexOf(itemRef);

	// Check if itemRef was found
	if (itemIndex.hasValue())
		// Remove
		mInternals = mInternals->removeAtIndex(*itemIndex, false);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::move(const ItemRef itemRef, CArray& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	OV<ItemIndex>	itemIndex = getIndexOf(itemRef);

	// Check if itemRef was found
	if (itemIndex.hasValue()) {
		// Remove
		mInternals = mInternals->removeAtIndex(*itemIndex, false);

		// Append
		other.add(itemRef);
	}

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::detachAtIndex(ItemIndex itemIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);

	// Remove
	mInternals = mInternals->removeAtIndex(itemIndex, false);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::remove(const ItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	OV<ItemIndex>	itemIndex = getIndexOf(itemRef);

	// Check if itemRef was found
	if (itemIndex.hasValue())
		// Remove
		mInternals = mInternals->removeAtIndex(*itemIndex, true);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::removeAtIndex(ItemIndex itemIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);

	// Remove
	mInternals = mInternals->removeAtIndex(itemIndex, true);

	return *this;
}

////----------------------------------------------------------------------------------------------------------------------
//CArray& CArray::removeFrom(const CArray& other)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Iterate all in the other
//	for (ItemIndex otherItemIndex = 0; otherItemIndex < other.mInternals->mCount; otherItemIndex++) {
//		// Check if other item is in local storage
//		ItemRef			otherItemRef = other.mInternals->mItemRefs[otherItemIndex];
//		OV<ItemIndex>	itemIndex = getIndexOf(otherItemRef);
//		if (itemIndex.hasValue())
//			// Remove
//			mInternals = mInternals->removeAtIndex(*itemIndex, true);
//	}
//
//	return *this;
//}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove all
	mInternals = mInternals->removeAll();

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
bool CArray::equals(const CArray& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compare
	return (mInternals->mCount == other.mInternals->mCount) &&
			(::memcmp(mInternals->mItemRefs, other.mInternals->mItemRefs, mInternals->mCount * sizeof(ItemRef)) == 0);
}

//----------------------------------------------------------------------------------------------------------------------
TIteratorS<CArray::ItemRef> CArray::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getIterator();
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::apply(ApplyProc applyProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all item refs
	for (ItemIndex i = 0; i < mInternals->mCount; i++)
		// Call proc
		applyProc(mInternals->mItemRefs[i], userData);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::sort(CompareProc compareProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Sort
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	// BSD platforms
	SArraySortInfo	sortInfo = {compareProc, userData};
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), &sortInfo, sSortProc);
#elif TARGET_OS_LINUX
	// GLibc platforms
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), sSortProc, userData);
#elif TARGET_OS_WINDOWS
	// Windows platforms
	SArraySortInfo	sortInfo = {compareProc, userData};
	qsort_s((void*) *mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), sSortProc, &sortInfo);
#else
	// Unknown platform
	AssertFailUnimplemented();
#endif

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray CArray::sorted(CompareProc compareProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CArray	array = *this;

	// Sort
	array.sort(compareProc, userData);

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
CArray CArray::filtered(IsIncludedProc isIncludedProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CArray	array;

	// Iterate all item refs
	for (ItemIndex i = 0; i < mInternals->mCount; i++) {
		// Call proc
		if (isIncludedProc(mInternals->mItemRefs[i], userData))
			// Included
			array.add(mInternals->mItemRefs[i]);
	}

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::operator=(const CArray& other)
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
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
int sSortProc(void* info, const void* itemRef1, const void* itemRef2)
//----------------------------------------------------------------------------------------------------------------------
{
	SArraySortInfo*	sortInfo = (SArraySortInfo*) info;

	return sortInfo->mCompareProc(*((CArray::ItemRef*) itemRef1), *((CArray::ItemRef*) itemRef2), sortInfo->mUserData) ?
			-1 : 1;
}
