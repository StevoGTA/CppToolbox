//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSource

class CDataSource {
	// Enums
	public:
		enum Position {
			kPositionFromBeginning,
			kPositionFromCurrent,
			kPositionFromEnd,
		};

	// Methods
	public:
							// Lifecycle methods
							CDataSource() {}
		virtual				~CDataSource() {}

							// Instance methods
		virtual	UInt64		getSize() const = 0;

		virtual	OI<SError>	readData(void* buffer, UInt64 byteCount) = 0;
				OI<CData>	readData(UInt64 byteCount, OI<SError>& outError);
				OI<CData>	readData(OI<SError>& outError);

		virtual	SInt64		getPos() const = 0;
		virtual	OI<SError>	setPos(Position position, SInt64 newPos) = 0;

		virtual	void		reset() = 0;

	// Properties
	protected:
		static	SError	mSetPosBeforeStartError;
		static	SError	mSetPosAfterEndError;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataDataSource

class CDataDataSourceInternals;
class CDataDataSource : public CDataSource {
	// Methods
	public:
					// Lifecycle methods
					CDataDataSource(const CData& data);
					~CDataDataSource();

					// CDataSource methods
		UInt64		getSize() const;

		OI<SError>	readData(void* buffer, UInt64 byteCount);

		SInt64		getPos() const;
		OI<SError>	setPos(Position position, SInt64 newPos);

		void		reset();

	// Properties
	private:
		CDataDataSourceInternals*	mInternals;
};
