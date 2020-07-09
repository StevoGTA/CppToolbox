//----------------------------------------------------------------------------------------------------------------------
//	TBuffer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: TBuffer

template <typename T> struct TBuffer {
		// Lifecycle methods
		TBuffer(UInt64 count) : mStorage((T*) ::calloc(count, sizeof(T))) {}
		~TBuffer() { free(mStorage); }

		// Instamce methods
	T*	operator*() const { return mStorage; }
	T	operator[](UInt64 index) const { return mStorage[index]; }

	// Properties
	private:
		T*	mStorage;
};
