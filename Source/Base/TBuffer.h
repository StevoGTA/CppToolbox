//----------------------------------------------------------------------------------------------------------------------
//	TBuffer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TBuffer

template <typename T> struct TBuffer {
			// Lifecycle methods
			TBuffer(UInt32 count) :
				mStorage((T*) ::calloc(count, sizeof(T))), mOwnsStorage(true), mByteCount(count * sizeof(T))
				{}
			TBuffer(T* buffer, UInt32 size) : mStorage(buffer), mOwnsStorage(false), mByteCount(size) {}
			~TBuffer() { if (mOwnsStorage) ::free(mStorage); }

			// Instamce methods
	T*		operator*() const
				{ return mStorage; }
	T&		operator[](UInt32 index) const
				{ return mStorage[index]; }
	UInt32	getByteCount() const
				{ return mByteCount; }

	// Properties
	private:
		T*		mStorage;
		bool	mOwnsStorage;
		UInt32	mByteCount;
};
