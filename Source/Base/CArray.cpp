//----------------------------------------------------------------------------------------------------------------------
//	CArray.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CArray.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CArray::IteratorInfo:

class CArray::IteratorInfo {
	public:
		IteratorInfo(const Internals& internals, UInt32 initialReference) :
			mInternals(internals),
					mInitialReference(initialReference), mCurrentIndex(0)
			{}

		const	Internals&	mInternals;

				UInt32		mInitialReference;
				UInt32		mCurrentIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CArray::Internals

class CArray::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
		struct SortInfo {
			public:
				CompareProc	mCompareProc;
				void*		mUserData;
		};

	public:
								Internals(ItemCount initialCapacity, CopyProc copyProc,
										DisposeProc disposeProc) :
									TCopyOnWriteReferenceCountable(),
											mCapacity(std::max(initialCapacity, (UInt32) 10)), mCount(0),
											mItemRefs((ItemRef*) ::calloc(mCapacity, sizeof(ItemRef))),
											mCopyProc(copyProc), mDisposeProc(disposeProc), mReference(0)
									{}
								Internals(const Internals& other) :
									TCopyOnWriteReferenceCountable(),
											mCapacity(other.mCount), mCount(other.mCount),
											mItemRefs((ItemRef*) ::calloc(mCapacity, sizeof(ItemRef))),
											mCopyProc(other.mCopyProc), mDisposeProc(other.mDisposeProc),
											mReference(0)
									{
										// Check if have copy proc
										if (mCopyProc != nil) {
											// Copy each item
											for (ItemIndex i = 0; i < mCount; i++)
												// Copy item
												mItemRefs[i] = mCopyProc(other.mItemRefs[i]);
										} else
											// Copy item refs
											::memcpy(mItemRefs, other.mItemRefs, mCount * sizeof(ItemRef));
									}
								~Internals()
									{
										// Remove all
										removeAllInternal();

										// Cleanup
										::free(mItemRefs);
									}

				OV<ItemIndex>	getIndexOf(const ItemRef itemRef) const
									{
										// Scan
										ItemIndex	itemIndex = 0;
										for (ItemRef* testItemRef = mItemRefs; itemIndex < mCount;
												itemIndex++, testItemRef++) {
											// Check test item ref
											if (*testItemRef == itemRef)
												// Found
												return OV<ItemIndex>(itemIndex);
										}

										return OV<ItemIndex>();
									}

				void			append(const ItemRef* itemRefs, ItemCount count, CopyProc copyProc)
									{
										// Setup
										ItemCount	neededCount = mCount + count;

										// Check storage
										if (neededCount > mCapacity) {
											// Expand storage
											mCapacity = std::max(neededCount, mCapacity * 2);
											mItemRefs = (ItemRef*) ::realloc(mItemRefs, mCapacity * sizeof(ItemRef));
										}

										// Check if have copy proc
										if (copyProc != nil) {
											// Copy each item
											for (ItemIndex i = 0; i < count; i++)
												// Copy item
												mItemRefs[mCount + i] = copyProc(itemRefs[i]);
										} else
											// Append itemRefs into place
											::memcpy(mItemRefs + mCount, itemRefs, count * sizeof(ItemRef));

										// Update info
										mCount = neededCount;
										mReference++;
									}
				void			insertAtIndex(const ItemRef itemRef, ItemIndex itemIndex, CopyProc copyProc)
									{
										// Setup
										ItemCount	neededCount = mCount + 1;

										// Check storage
										if (neededCount > mCapacity) {
											// Expand storage
											mCapacity = std::max(neededCount, mCapacity * 2);
											mItemRefs = (ItemRef*) ::realloc(mItemRefs, mCapacity * sizeof(ItemRef));
										}

										// Move following itemRefs back
										::memmove(mItemRefs + itemIndex + 1, mItemRefs + itemIndex,
												(mCount - itemIndex) * sizeof(ItemRef));

										// Update info
										mCount++;
										mItemRefs[itemIndex] = (copyProc != nil) ? copyProc(itemRef) : itemRef;
										mReference++;
									}
				void			removeAtIndex(ItemIndex itemIndex, bool performDispose)
									{
										// Check if owns items
										if (performDispose && (mDisposeProc != nil))
											// Dispose
											mDisposeProc(mItemRefs[itemIndex]);

										// Move following itemRefs forward
										::memmove(mItemRefs + itemIndex, mItemRefs + itemIndex + 1,
												(mCount - itemIndex - 1) * sizeof(ItemRef));

										// Update info
										mCount--;
										mReference++;
									}
				void			removeAll()
									{
										// Remove all
										removeAllInternal();

										// Update info
										mCount = 0;
										mReference++;
									}

		static	int				sort(void* info, const void* itemRef1, const void* itemRef2)
									{
										SortInfo*	sortInfo = (SortInfo*) info;

										return sortInfo->mCompareProc(*((ItemRef*) itemRef1), *((ItemRef*) itemRef2),
														sortInfo->mUserData) ?
												-1 : 1;
									}

	private:
				void			removeAllInternal()
									{
										// Check if have item dispose proc
										if (mDisposeProc != nil) {
											// Dispose each item
											for (ItemIndex i = 0; i < mCount; i++) {
												// Dispose
												mDisposeProc(mItemRefs[i]);
											}
										}
									}

	public:
		ItemCount	mCapacity;
		ItemCount	mCount;
		ItemRef*	mItemRefs;
		CopyProc	mCopyProc;
		DisposeProc	mDisposeProc;
		UInt32		mReference;
};

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
I<CArray::IteratorInfo> CArray::getIteratorInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	return I<IteratorInfo>(new IteratorInfo(*mInternals, mInternals->mReference));
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
	Internals::SortInfo	sortInfo = {compareProc, userData};
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), &sortInfo, Internals::sort);
#elif defined(TARGET_OS_LINUX)
	// GLibc platforms
	qsort_r(mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), Internals::sort, userData);
#elif defined(TARGET_OS_WINDOWS)
	// Windows platforms
	Internals::SortInfo	sortInfo = {compareProc, userData};
	qsort_s((void*) mInternals->mItemRefs, mInternals->mCount, sizeof(ItemRef), Internals::sort, &sortInfo);
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

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CArray::iteratorGetCurrentIndex(const I<IteratorInfo>& iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	return iteratorInfo->mCurrentIndex;
}

//----------------------------------------------------------------------------------------------------------------------
CArray::ItemRef CArray::iteratorAdvance(const I<IteratorInfo>& iteratorInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Integrity check
	AssertFailIf(iteratorInfo->mInitialReference != iteratorInfo->mInternals.mReference);

	if (iteratorInfo->mCurrentIndex < iteratorInfo->mInternals.mCount)
		// Increement index
		iteratorInfo->mCurrentIndex++;

	return (iteratorInfo->mCurrentIndex < iteratorInfo->mInternals.mCount) ?
			iteratorInfo->mInternals.mItemRefs[iteratorInfo->mCurrentIndex] : nil;
}
