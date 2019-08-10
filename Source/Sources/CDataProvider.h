//----------------------------------------------------------------------------------------------------------------------
//	CDataProvider.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Position mode

enum EDataProviderPosition {
	kDataProviderPositionFromBeginning,
	kDataProviderPositionFromCurrent,
	kDataProviderPositionFromEnd,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Errors

const	UErrorDomain	kDataProviderErrorDomain	= MAKE_OSTYPE('D', 'a', 'P', 'r');

const	UError	kDataProviderReadBeyondEndError		= MAKE_UError(kDataProviderErrorDomain, 1);
const	UError	kDataProviderSetPosBeforeStartError	= MAKE_UError(kDataProviderErrorDomain, 2);
const	UError	kDataProviderSetPosAfterEndError	= MAKE_UError(kDataProviderErrorDomain, 3);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataProvider

class CDataProvider {
	// Methods
	public:
						// Lifecycle methods
						CDataProvider() {}
		virtual			~CDataProvider() {}

						// Instance methods
		virtual	UInt64	getSize() const = 0;

		virtual	UError	readData(void* buffer, UInt64 byteCount) const = 0;

		virtual	SInt64	getPos() const = 0;
		virtual	UError	setPos(EDataProviderPosition position, SInt64 newPos) const = 0;

		virtual	void	reset() const = 0;
};
