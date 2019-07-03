//----------------------------------------------------------------------------------------------------------------------
//	CArray.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CArray.h"

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: SArraySortInfo

struct SArraySortInfo {
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
// MARK: - CArray2Internals

class CArrayInternals {
	public:
											CArrayInternals(CArrayItemCount initialCapacity,
													CArrayItemCopyProc itemCopyProc,
													CArrayItemDisposeProc itemDisposeProc) :
												mIsSorted(true), mCount(0), mItemCopyProc(itemCopyProc),
														mItemDisposeProc(itemDisposeProc), mReferenceCount(1),
														mReference(0)
												{
													// Allocate at least some minimum
													mCapacity = std::max(initialCapacity, (UInt32) 10);
													mItemRefs =
															(CArrayItemRef*)
																	::malloc(mCapacity * sizeof(CArrayItemRef));
												}
											~CArrayInternals()
												{
													// Remove all
													removeAllInternal();

													// Cleanup
													::free(mItemRefs);
												}

				CArrayInternals*			addReference() { mReferenceCount++; return this; }
				void						removeReference()
												{
													// Decrement reference count and check if we are the last one
													if (--mReferenceCount == 0) {
														// We going away
														CArrayInternals*	THIS = this;
														DisposeOf(THIS);
													}
												}

				CArrayInternals*			prepareForWrite()
												{
													// Check reference count.  If there is more than 1 reference, we
													//	implement a "copy on write".  So we will clone ourselves so we
													//	have a personal buffer that can be changed while leaving the
													//	exiting buffer as-is for the other references.
													if (mReferenceCount > 1) {
														// Multiple references, copy
														CArrayInternals*	arrayInternals =
																					new CArrayInternals(mCount,
																							mItemCopyProc,
																							mItemDisposeProc);
														arrayInternals->mCount = mCount;
														arrayInternals->mIsSorted = mIsSorted;
														if (mItemCopyProc != nil) {
															// Copy each item
															for (CArrayItemIndex i = 0; i < mCount; i++)
																// Copy item
																arrayInternals->mItemRefs[i] =
																		mItemCopyProc(mItemRefs[i]);
														} else
															// Copy item refs
															::memcpy(arrayInternals->mItemRefs, mItemRefs,
																	mCount * sizeof(CArrayItemRef));

														// One less reference here
														mReferenceCount--;

														return arrayInternals;
													} else
														// Only a single reference
														return this;
												}

				CArrayInternals*			append(const CArrayItemRef itemRefs, CArrayItemCount count,
													bool avoidDuplicates)
												{
													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													// Setup
													CArrayItemCount	neededCount = arrayInternals->mCount + count;

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

													// Append itemRefs into place
													::memcpy(arrayInternals->mItemRefs + arrayInternals->mCount,
															itemRefs, count * sizeof(CFArrayRef));
													arrayInternals->mCount = neededCount;

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
				CArrayInternals*			removeAtIndex(CArrayItemIndex itemIndex)
												{
													// Prepare for write
													CArrayInternals*	arrayInternals = prepareForWrite();

													// Check if owns items
													if (mItemDisposeProc != nil)
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

		bool					mIsSorted;
		CArrayItemCount			mCapacity;
		CArrayItemCount			mCount;
		CArrayItemRef*			mItemRefs;
		CArrayItemCopyProc		mItemCopyProc;
		CArrayItemDisposeProc	mItemDisposeProc;
		UInt32					mReferenceCount;
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
CArray& CArray::add(const CArrayItemRef itemRef, bool avoidDuplicates)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);
	if (itemRef == nil)
		return *this;

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
	if (itemRef == nil)
		return false;

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
CArrayItemIndex CArray::getIndexOf(const CArrayItemRef itemRef) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);
	if (itemRef == nil)
		return false;

	// Scan
	CArrayItemIndex	itemIndex = 0;
	for (CArrayItemRef* testItemRef = mInternals->mItemRefs; itemIndex < mInternals->mCount;
			itemIndex++, testItemRef++) {
		// Check test item ref
		if (*testItemRef == itemRef)
			// Found
			return itemIndex;
	}

	return kCArrayItemIndexNotFound;
}

//----------------------------------------------------------------------------------------------------------------------
CArrayItemRef CArray::getItemAt(CArrayItemIndex itemIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);
	if (itemIndex >= mInternals->mCount)
		return nil;

	return mInternals->mItemRefs[itemIndex];
}

//----------------------------------------------------------------------------------------------------------------------
CArrayItemRef CArray::getLast() const
//----------------------------------------------------------------------------------------------------------------------
{
	return (mInternals->mCount > 0) ? mInternals->mItemRefs[mInternals->mCount - 1] : nil;
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
	if (itemRef == nil)
		return *this;

	AssertFailIf(itemIndex > mInternals->mCount);
	if (itemIndex > mInternals->mCount)
		return *this;

	// Insert at index
	mInternals = mInternals->insertAtIndex(itemRef, itemIndex);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::remove(const CArrayItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	CArrayItemIndex	itemIndex = getIndexOf(itemRef);

	// Check if itemRef was found
	if (itemIndex != kCArrayItemIndexNotFound)
		// Remove
		mInternals = mInternals->removeAtIndex(itemIndex);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::removeAtIndex(CArrayItemIndex itemIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);
	if (itemIndex >= mInternals->mCount)
		return *this;

	// Remove
	mInternals = mInternals->removeAtIndex(itemIndex);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::removeFrom(const CArray& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all in the other
	for (CArrayItemIndex otherItemIndex = 0; otherItemIndex < other.mInternals->mCount; otherItemIndex++) {
		// Check if other item is in local storage
		CArrayItemRef	otherItemRef = other.mInternals->mItemRefs[otherItemIndex];
		CArrayItemIndex	itemIndex = getIndexOf(otherItemRef);
		if (itemIndex != kCArrayItemIndexNotFound)
			// Remove
			mInternals = mInternals->removeAtIndex(itemIndex);
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
	// Check if sorted
	if (mInternals->mIsSorted)
		return *this;
		
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Sort
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	// BSD platforms
	SArraySortInfo	sortInfo = {compareProc, userData};
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(CArrayItemRef), &sortInfo, sSortProc);
#elif defined(TARGET_OS_LINUX)
	// GLibc platforms
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(CArrayItemRef), sSortProc, userData);
#elif defined(TARGET_OS_WINDOWS)
	// Windows platforms
	SArraySortInfo	sortInfo = {compareProc, userData};
	qsort_s(*mInternals->mItemRefs, mInternals->mCount, sizeof(CArrayItemRef), sSortProc, &sortInfo);
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
