//----------------------------------------------------------------------------------------------------------------------
//	CArray.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CArray.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SArraySortInfo

struct SArraySortInfo {
	// Properties
	CArray::CompareProc	mCompareProc;
	void*				mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CArray::IteratorInfo

class CArray::IteratorInfo : public CIterator::Info {
	// Methods
	public:
						// Lifecycle methods
						IteratorInfo(const Internals& internals, UInt32 initialReference) :
							CIterator::Info(),
									mInternals(internals), mInitialReference(initialReference), mCurrentIndex(0)
							{}

						// CIterator::Info methods
	CIterator::Info*	copy()
							{
								// Make copy
								IteratorInfo*	iteratorInfo = new IteratorInfo(mInternals, mInitialReference);
								iteratorInfo->mCurrentIndex = mCurrentIndex;

								return iteratorInfo;
							}

	// Properties
	const	Internals&	mInternals;
			UInt32		mInitialReference;
			UInt32		mCurrentIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CArray::Internals

class CArray::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
											Internals(CArray::ItemCount initialCapacity, CArray::CopyProc copyProc,
													CArray::DisposeProc disposeProc) :
												TCopyOnWriteReferenceCountable(),
														mCapacity(std::max(initialCapacity, (UInt32) 10)), mCount(0),
														mItemRefs(
																(CArray::ItemRef*)
																		::calloc(mCapacity, sizeof(CArray::ItemRef))),
														mCopyProc(copyProc),
														mDisposeProc(disposeProc), mReference(0)
												{}
											Internals(const Internals& other) :
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
											~Internals()
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

				void						append(const CArray::ItemRef* itemRefs, CArray::ItemCount count,
													CArray::CopyProc copyProc)
												{
													// Setup
													CArray::ItemCount	neededCount = mCount + count;

													// Check storage
													if (neededCount > mCapacity) {
														// Expand storage
														mCapacity = std::max(neededCount, mCapacity * 2);
														mItemRefs =
																(CArray::ItemRef*)
																		::realloc(mItemRefs,
																				mCapacity * sizeof(CArray::ItemRef));
													}

													// Check if have copy proc
													if (copyProc != nil) {
														// Copy each item
														for (CArray::ItemIndex i = 0; i < count; i++)
															// Copy item
															mItemRefs[mCount + i] = copyProc(itemRefs[i]);
													} else
														// Append itemRefs into place
														::memcpy(mItemRefs + mCount, itemRefs,
																count * sizeof(CArray::ItemRef));
													mCount = neededCount;

													// Update info
													mReference++;
												}
				void						insertAtIndex(const CArray::ItemRef itemRef, CArray::ItemIndex itemIndex,
													CArray::CopyProc copyProc)
												{
													// Setup
													CArray::ItemCount	neededCount = mCount + 1;

													// Check storage
													if (neededCount > mCapacity) {
														// Expand storage
														mCapacity = std::max(neededCount, mCapacity * 2);
														mItemRefs =
																(CArray::ItemRef*)
																		::realloc(mItemRefs,
																				mCapacity * sizeof(CArray::ItemRef));
													}

													// Move following itemRefs back
													::memmove(mItemRefs + itemIndex + 1, mItemRefs + itemIndex,
															(mCount - itemIndex) * sizeof(CArray::ItemRef));

													// Check if have copy proc
													if (copyProc != nil)
														// Copy item
														mItemRefs[itemIndex] = copyProc(itemRef);
													else
														// Store new itemRef
														mItemRefs[itemIndex] = itemRef;
													mCount++;

													// Update info
													mReference++;
												}
				void						removeAtIndex(CArray::ItemIndex itemIndex, bool performDispose)
												{
													// Check if owns items
													if (performDispose && (mDisposeProc != nil))
														// Dispose
														mDisposeProc(mItemRefs[itemIndex]);

													// Move following itemRefs forward
													::memmove(mItemRefs + itemIndex, mItemRefs + itemIndex + 1,
															(mCount - itemIndex - 1) * sizeof(CArray::ItemRef));

													// Update info
													mCount--;
													mReference++;
												}
				void						removeAll()
												{
													// Remove all
													removeAllInternal();

													// Update info
													mCount = 0;
													mReference++;
												}

				TIteratorS<CArray::ItemRef>	getIterator() const
												{
													// Setup
													IteratorInfo*	iteratorInfo = new IteratorInfo(*this, mReference);

													return TIteratorS<CArray::ItemRef>((mCount > 0) ? mItemRefs : nil,
															(CIterator::AdvanceProc) iteratorAdvance, *iteratorInfo);
												}

		static	void*						iteratorAdvance(IteratorInfo& arrayIteratorInfo)
												{
													return (++arrayIteratorInfo.mCurrentIndex <
																	arrayIteratorInfo.mInternals.mCount) ?
															arrayIteratorInfo.mInternals.mItemRefs +
																	arrayIteratorInfo.mCurrentIndex :
															nil;
												}

	private:
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
	mInternals = new Internals(initialCapacity, copyProc, disposeProc);
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
CArray& CArray::attach(const ItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Add item
	mInternals->append(&itemRef, 1, nil);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::add(const ItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(itemRef);

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Add item
	mInternals->append(&itemRef, 1, mInternals->mCopyProc);

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

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Add items
	mInternals->append(other.mInternals->mItemRefs, other.mInternals->mCount, mInternals->mCopyProc);

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

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Insert at index
	mInternals->insertAtIndex(itemRef, itemIndex, mInternals->mCopyProc);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::detach(const ItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	OV<ItemIndex>	itemIndex = getIndexOf(itemRef);

	// Check if itemRef was found
	if (itemIndex.hasValue()) {
		// Prepare for write
		Internals::prepareForWrite(&mInternals);

		// Remove
		mInternals->removeAtIndex(*itemIndex, false);
	}

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
		// Prepare for write
		Internals::prepareForWrite(&mInternals);

		// Remove
		mInternals->removeAtIndex(*itemIndex, false);

		// Append
		other.attach(itemRef);
	}

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::detachAtIndex(ItemIndex itemIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Remove
	mInternals->removeAtIndex(itemIndex, false);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::remove(const ItemRef itemRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get index of itemRef
	OV<ItemIndex>	itemIndex = getIndexOf(itemRef);

	// Check if itemRef was found
	if (itemIndex.hasValue()) {
		// Prepare for write
		Internals::prepareForWrite(&mInternals);

		// Remove
		mInternals->removeAtIndex(*itemIndex, true);
	}

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::removeAtIndex(ItemIndex itemIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(itemIndex >= mInternals->mCount);

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Remove
	mInternals->removeAtIndex(itemIndex, true);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CArray& CArray::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Remove all
	mInternals->removeAll();

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
	Internals::prepareForWrite(&mInternals);

	// Sort
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	// BSD platforms
	SArraySortInfo	sortInfo = {compareProc, userData};
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), &sortInfo, sSortProc);
#elif defined(TARGET_OS_LINUX)
	// GLibc platforms
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), sSortProc, userData);
#elif defined(TARGET_OS_WINDOWS)
	// Windows platforms
	SArraySortInfo	sortInfo = {compareProc, userData};
	qsort_s((void*) mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), sSortProc, &sortInfo);
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
CArray CArray::filtered(IsMatchProc isMatchProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CArray	array;

	// Iterate all item refs
	for (ItemIndex i = 0; i < mInternals->mCount; i++) {
		// Call proc
		if (isMatchProc(mInternals->mItemRefs[i], userData))
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
