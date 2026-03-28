//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "TResult.h"

/*
Notes:
	readData(...) returns a CData object that may live after the Data 
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSource

class CDataSource {
	// Methods
	public:
								// Lifecycle methods
								CDataSource() {}
		virtual					~CDataSource() {}

								// Instance methods
		virtual	TVResult<CData>	readData() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CRandomAccessDataSource

class CRandomAccessDataSource : public CDataSource {
	// Methods
	public:
											// Lifecycle methods
											CRandomAccessDataSource() : CDataSource() {}

											// CDataSource methods
				TVResult<CData>				readData()
												{ return readData(0, getByteCount()); }

											// Instance methods
		virtual	UInt64						getByteCount() const = 0;

				bool						canRead(UInt64 position, CData::ByteCount byteCount)
												{ return (position + byteCount) <= getByteCount(); }

		virtual	OV<SError>					read(UInt64 position, void* buffer, UInt64 byteCount) = 0;
		virtual	TVResult<CData>				readData(UInt64 position, CData::ByteCount byteCount) = 0;
		virtual	TVResult<TBuffer<UInt8> >	readUInt8Buffer(UInt64 position, UInt64 byteCount) = 0;

	// Properties
	public:
		static	const	SError	mSetPosBeforeStartError;
		static	const	SError	mSetPosAfterEndError;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataDataSource

class CDataDataSource : public CRandomAccessDataSource {
	// Classes
	private:
		class Internals;

	// Methods
	public:
									// Lifecycle methods
									CDataDataSource(const CData& data);
									~CDataDataSource();

									// CRandomAccessDataSource methods
		UInt64						getByteCount() const;

		OV<SError>					read(UInt64 position, void* buffer, UInt64 byteCount);
		TVResult<CData>				readData(UInt64 position, CData::ByteCount byteCount);
		TVResult<TBuffer<UInt8> >	readUInt8Buffer(UInt64 position, UInt64 byteCount);

	// Properties
	private:
		Internals*	mInternals;
};
