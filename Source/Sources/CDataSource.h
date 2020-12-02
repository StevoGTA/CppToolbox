//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Position mode

enum EDataSourcePosition {
	kDataSourcePositionFromBeginning,
	kDataSourcePositionFromCurrent,
	kDataSourcePositionFromEnd,
};

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

		virtual	OI<SError>		readData(void* buffer, UInt64 byteCount) = 0;

		virtual	SInt64			getPos() const = 0;
		virtual	OI<SError>		setPos(EDataSourcePosition position, SInt64 newPos) = 0;

		virtual	CDataSource*	clone() const = 0;

		virtual	void			reset() = 0;

	// Properties
	protected:
		static	SError	mSetPosBeforeStartError;
		static	SError	mSetPosAfterEndError;
};

