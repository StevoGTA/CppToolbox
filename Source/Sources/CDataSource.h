//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Position mode

enum EDataSourcePosition {
	kDataSourcePositionFromBeginning,
	kDataSourcePositionFromCurrent,
	kDataSourcePositionFromEnd,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Errors

const	UErrorDomain	kDataProviderErrorDomain	= MAKE_OSTYPE('D', 'a', 'P', 'r');

const	UError	kDataProviderReadBeyondEndError		= MAKE_UError(kDataProviderErrorDomain, 1);
const	UError	kDataProviderSetPosBeforeStartError	= MAKE_UError(kDataProviderErrorDomain, 2);
const	UError	kDataProviderSetPosAfterEndError	= MAKE_UError(kDataProviderErrorDomain, 3);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataSource

class CDataSource {
	// Methods
	public:
								// Lifecycle methods
								CDataSource() {}
		virtual					~CDataSource() {}

								// Instance methods
		virtual	UInt64			getSize() const = 0;

		virtual	UError			readData(void* buffer, UInt64 byteCount) = 0;

		virtual	SInt64			getPos() const = 0;
		virtual	UError			setPos(EDataSourcePosition position, SInt64 newPos) = 0;

		virtual	CDataSource*	clone() const = 0;

		virtual	void			reset() = 0;
};
