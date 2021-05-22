//----------------------------------------------------------------------------------------------------------------------
//	CArray.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CArray.h"

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: SArraySortInfo

struct SArraySortInfo {
	// Properties
	CArrayCompareProc	mCompareProc;
	void*				mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CArrayIteratorInfo

class CArrayIteratorInfo : public CIteratorInfo {
	// Methods
	public:
					// Lifecycle methods
					CArrayIteratorInfo(const CArrayInternals& internals, UInt32 initialReference) :
						CIteratorInfo(), mInternals(internals), mInitialReference(initialReference), mCurrentIndex(0)
						{}

					// CIteratorInfo methods
	CIteratorInfo*	copy()
						{
							// Make copy
							CArrayIteratorInfo*	iteratorInfo = new CArrayIteratorInfo(mInternals, mInitialReference);
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
											CArrayInternals(CArrayItemCount initialCapacity,
													CArrayItemCopyProc itemCopyProc,
													CArrayItemDisposeProc itemDisposeProc) :
												TCopyOnWriteReferenceCountable(),
														mIsSorted(true),
														mCapacity(std::max(initialCapacity, (UInt32) 10)), mCount(0),
														mItemRefs(
																(CArrayItemRef*)
																		::calloc(mCapacity, sizeof(CArrayItemRef))),
														mItemCopyProc(itemCopyProc),
														mItemDisposeProc(itemDisposeProc), mReference(0)
												{}
											CArrayInternals(const CArrayInternals& other) :
												TCopyOnWriteReferenceCountable(),
														mIsSorted(other.mIsSorted), mCapacity(other.mCount),
														mCount(other.mCount),
														mItemRefs(
																(CArrayItemRef*)
																		::calloc(mCapacity, sizeof(CArrayItemRef))),
														mItemCopyProc(other.mItemCopyProc),
														mItemDisposeProc(other.mItemDisposeProc), mReference(0)
												{
													// Check if have item copy proc
													if (mItemCopyProc != nil) {
														// Copy each item
														for (CArrayItemIndex i = 0; i < mCount; i++)
															// Copy item
															mItemRefs[i] = mItemCopyProc(other.mItemRefs[i]);
													} else
														// Copy item refs
														::memcpy(mItemRefs, other.mItemRefs,
																mCount * sizeof(CArrayItemRef));
												}
											~CArrayInternals()
												{
													// Remove all
													removeAllInternal();

													// Cleanup
													::free(mItemRefs);
												}

				OV<CArrayItemIndex>			getIndexOf(const CArrayItemRef itemRef) const
												{
													// Scan
													CArrayItemIndex	itemIndex = 0;
													for (CArrayItemRef* testItemRef = mItemRefs; itemIndex < mCount;
															itemIndex++, testItemRef++) {
														// Check test item ref
														if (*testItemRef == itemRef)
															// Found
															return OV<CArrayItemIndex>(itemIndex);
													}

													return OV<CArrayItemIndex>();
												}

				CArrayInternals*			append(const CArrayItemRef* itemRefs, CArrayItemCount count,
													bool avoidDuplicates)
												{
													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													if (!avoidDuplicates)
														// General case
														arrayInternals->append(itemRefs, count);
													else {
														// Must ensure we are not adding an itemRef we already have
														for (CArrayItemCount i = 0; i < count; i++) {
															// Get itemRef
															CArrayItemRef	itemRef = itemRefs[i];

															// Check if found
															OV<CArrayItemIndex>	index =
																						arrayInternals->getIndexOf(
																								itemRef);

															if (!index.hasValue())
																// Not found
																arrayInternals->append(&itemRef, 1);
														}
													}

													// Update info
													arrayInternals->mIsSorted = false;
													arrayInternals->mReference++;

													return arrayInternals;
												}
				CArrayInternals*			insertAtIndex(const CArrayItemRef itemRef, CArrayItemIndex itemIndex)
												{
													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													// Setup
													CArrayItemCount	neededCount = arrayInternals->mCount + 1;

													// Check storage
													if (neededCount > arrayInternals->mCapacity) {
														// Expand storage
														arrayInternals->mCapacity =
																std::max(neededCount, arrayInternals->mCapacity * 2);
														arrayInternals->mItemRefs =
																(CArrayItemRef*)
																		::realloc(mItemRefs,
																				arrayInternals->mCapacity *
																						sizeof(CArrayItemRef));
													}

													// Move following itemRefs back
													::memmove(arrayInternals->mItemRefs + itemIndex + 1,
															arrayInternals->mItemRefs + itemIndex,
															(arrayInternals->mCount - itemIndex) *
																	sizeof(CArrayItemRef));

													// Store new itemRef
													arrayInternals->mItemRefs[itemIndex] = itemRef;
													arrayInternals->mCount++;

													// Update info
													arrayInternals->mIsSorted = false;
													arrayInternals->mReference++;

													return arrayInternals;
												}
				CArrayInternals*			removeAtIndex(CArrayItemIndex itemIndex, bool performDispose)
												{
													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													// Check if owns items
													if (performDispose && (mItemDisposeProc != nil))
														// Dispose
														mItemDisposeProc(arrayInternals->mItemRefs[itemIndex]);

													// Move following itemRefs forward
													::memmove(arrayInternals->mItemRefs + itemIndex,
															arrayInternals->mItemRefs + itemIndex + 1,
															(arrayInternals->mCount - itemIndex - 1) *
																	sizeof(CArrayItemRef));

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
													if (mItemDisposeProc != nil) {
														// Dispose each item
														for (CArrayItemIndex i = 0; i < mCount; i++) {
															// Dispose
															mItemDisposeProc(mItemRefs[i]);
														}
													}
												}

				TIteratorS<CArrayItemRef>	getIterator() const
												{
													// Setup
													CArrayIteratorInfo*	iteratorInfo =
																				new CArrayIteratorInfo(*this,
																						mReference);

													return TIteratorS<CArrayItemRef>((mCount > 0) ? mItemRefs : nil,
															iteratorAdvance, *iteratorInfo);
												}

		static	void*						iteratorAdvance(CIteratorInfo& iteratorInfo)
												{
													// Setup
													CArrayIteratorInfo&	arrayIteratorInfo =
																				(CArrayIteratorInfo&) iteratorInfo;

													return (++arrayIteratorInfo.mCurrentIndex <
																	arrayIteratorInfo.mInternals.mCount) ?
															arrayIteratorInfo.mInternals.mItemRefs +
																	arrayIteratorInfo.mCurrentIndex :
															nil;

												}

	private:
				void						append(const CArrayItemRef itemRefs, CArrayItemCount count)
												{
													// Setup
													CArrayItemCount	neededCount = mCount + count;

													// Check storage
													if (neededCount > mCapacity) {
														// Expand storage
														mCapacity = std::max(neededCount, mCapacity * 2);
														mItemRefs =
																(CArrayItemRef*)
																		::realloc(mItemRefs,
																				mCapacity * sizeof(CArrayItemRef));
													}

													// Append itemRefs into place
													::memcpy(mItemRefs + mCount, itemRefs, count * sizeof(CArrayItemRef));
													mCount = neededCount;
												}

	public:
		bool					mIsSorted;
		CArrayItemCount			mCapacity;
		CArrayItemCount			mCount;
		CArrayItemRef*			mItemRefs;
		CArrayItemCopyProc		mItemCopyProc;
		CArrayItemDisposeProc	mItemDisposeProc;
		UInt32					mReference;
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
CArray::CArray(CArrayItemCount initialCapacity, CArrayItemCopyProc itemCopyProc, CArrayItemDisposeProc itemDisposeProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CArrayInternals(initialCapacity, itemCopyProc, itemDisposeProc);
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
CArrayItemRef CArray::copy(const CArrayItemRef itemRef) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (mInternals->mItemCopyProc != nil) ? mInternals->mItemCopyProc(itemRef) : itemRef;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::add(const CArrayItemRef itemRef, bool avoidDuplicates)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);

	// Add item
	mInternals = mInternals->append(&itemRef, 1, avoidDuplicates);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::addFrom(const CArray& other, bool avoidDuplicates)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	if (other.mInternals->mCount == 0)
		// Nothing to add
		return *this;

	// Add items
	mInternals = mInternals->append(other.mInternals->mItemRefs, other.mInternals->mCount, avoidDuplicates);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
bool CArray::contains(const CArrayItemRef itemRef) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);

	// Scan
	CArrayItemIndex	itemIndex = 0;
	for (CArrayItemRef* testItemRef = mInternals->mItemRefs; itemIndex < mInternals->mCount;
			itemIndex++, testItemRef++) {
		// Check test item ref
		if (*testItemRef == itemRef)
			// Found
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CArrayItemIndex> CArray::getIndexOf(const CArrayItemRef itemRef) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);

	return mInternals->getIndexOf(itemRef);
}

//----------------------------------------------------------------------------------------------------------------------
CArrayItemRef CArray::getItemAt(CArrayItemIndex itemIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);

	return mInternals->mItemRefs[itemIndex];
}

//----------------------------------------------------------------------------------------------------------------------
CArrayItemRef CArray::getLast() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Sanity check
	AssertFailIf(mInternals->mCount == 0);

	return mInternals->mItemRefs[mInternals->mCount - 1];
}

//----------------------------------------------------------------------------------------------------------------------
CArrayItemCount CArray::getCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCount;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::insertAtIndex(const CArrayItemRef itemRef, CArrayItemIndex itemIndex)
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
CArray& CArray::detach(const CArrayItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	OV<CArrayItemIndex>	itemIndex = getIndexOf(itemRef);

	// Check if itemRef was found
	if (itemIndex.hasValue())
		// Remove
		mInternals = mInternals->removeAtIndex(*itemIndex, false);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::move(const CArrayItemRef itemRef, CArray& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	OV<CArrayItemIndex>	itemIndex = getIndexOf(itemRef);

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
CArray& CArray::detachAtIndex(CArrayItemIndex itemIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);

	// Remove
	mInternals = mInternals->removeAtIndex(itemIndex, false);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::remove(const CArrayItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	OV<CArrayItemIndex>	itemIndex = getIndexOf(itemRef);

	// Check if itemRef was found
	if (itemIndex.hasValue())
		// Remove
		mInternals = mInternals->removeAtIndex(*itemIndex, true);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::removeAtIndex(CArrayItemIndex itemIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);

	// Remove
	mInternals = mInternals->removeAtIndex(itemIndex, true);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::removeFrom(const CArray& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all in the other
	for (CArrayItemIndex otherItemIndex = 0; otherItemIndex < other.mInternals->mCount; otherItemIndex++) {
		// Check if other item is in local storage
		CArrayItemRef		otherItemRef = other.mInternals->mItemRefs[otherItemIndex];
		OV<CArrayItemIndex>	itemIndex = getIndexOf(otherItemRef);
		if (itemIndex.hasValue())
			// Remove
			mInternals = mInternals->removeAtIndex(*itemIndex, true);
	}

	return *this;
}

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
			(::memcmp(mInternals->mItemRefs, other.mInternals->mItemRefs,
					mInternals->mCount * sizeof(CArrayItemRef)) == 0);
}

//----------------------------------------------------------------------------------------------------------------------
TIteratorS<CArrayItemRef> CArray::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getIterator();
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::apply(CArrayApplyProc applyProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all item refs
	for (CArrayItemIndex i = 0; i < mInternals->mCount; i++)
		// Call proc
		applyProc(mInternals->mItemRefs[i], userData);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
bool CArray::isSorted() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsSorted;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::sort(CArrayCompareProc compareProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
//	// Check if sorted
//	if (mInternals->mIsSorted)
//		return *this;
		
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Sort
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	// BSD platforms
	SArraySortInfo	sortInfo = {compareProc, userData};
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(CArrayItemRef), &sortInfo, sSortProc);
#elif TARGET_OS_LINUX
	// GLibc platforms
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(CArrayItemRef), sSortProc, userData);
#elif TARGET_OS_WINDOWS
	// Windows platforms
	SArraySortInfo	sortInfo = {compareProc, userData};
	qsort_s((void*) *mInternals->mItemRefs, mInternals->mCount, sizeof(CArrayItemRef), sSortProc, &sortInfo);
#else
	// Unknown platform
	AssertFailWith(kUnimplementedError);
#endif

	// Update
	mInternals->mIsSorted = true;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray CArray::sorted(CArrayCompareProc compareProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CArray	array = *this;

	// Sort
	array.sort(compareProc, userData);

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
CArray CArray::filtered(CArrayItemIsIncludedProc isIncludedProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CArray	array;

	// Iterate all item refs
	for (CArrayItemIndex i = 0; i < mInternals->mCount; i++) {
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

	return (int) sortInfo->mCompareProc(*((CArrayItemRef*) itemRef1), *((CArrayItemRef*) itemRef2),
			sortInfo->mUserData);
}
