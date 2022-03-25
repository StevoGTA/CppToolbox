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
		virtual	UInt64			getByteCount() const = 0;

				bool			canReadData(UInt64 position, CData::ByteCount byteCount)
									{ return (position + byteCount) <= getByteCount(); }

		virtual	OI<SError>		readData(UInt64 position, void* buffer, CData::ByteCount byteCount) = 0;
		virtual	TIResult<CData>	readData(UInt64 position, CData::ByteCount byteCount) = 0;

	// Properties
	public:
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
		UInt64			getByteCount() const;

		OI<SError>		readData(UInt64 position, void* buffer, CData::ByteCount byteCount);
		TIResult<CData>	readData(UInt64 position, CData::ByteCount byteCount);

	// Properties
	private:
		CDataDataSourceInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CStreamableDataSource

class CStreamableDataSource : public CDataSource {
	// Methods
	public:
								// Lifecycle methods
								CStreamableDataSource() : CDataSource() {}

								// CDataSource methods
				TIResult<CData>	readData();

								// Instance methods

	// Properties
};
