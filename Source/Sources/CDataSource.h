//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSource

class CDataSource {
	// Methods
	public:
								// Lifecycle methods
								CDataSource() {}
		virtual					~CDataSource() {}

								// Instance methods
		virtual	TIResult<CData>	readData() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableDataSource

class CSeekableDataSource : public CDataSource {
	// Methods
	public:
								// Lifecycle methods
								CSeekableDataSource() : CDataSource() {}

								// CDataSource methods
				TIResult<CData>	readData();

								// Instance methods
		virtual	UInt64			getSize() const = 0;

		virtual	OI<SError>		readData(UInt64 position, void* buffer, CData::Size byteCount) = 0;

	// Properties
	protected:
		static	SError	mSetPosBeforeStartError;
		static	SError	mSetPosAfterEndError;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataDataSource

class CDataDataSourceInternals;
class CDataDataSource : public CSeekableDataSource {
	// Methods
	public:
					// Lifecycle methods
					CDataDataSource(const CData& data);
					~CDataDataSource();

					// CSeekableDataSource methods
		UInt64		getSize() const;

		OI<SError>	readData(UInt64 position, void* buffer, CData::Size byteCount);

	// Properties
	private:
		CDataDataSourceInternals*	mInternals;
};
