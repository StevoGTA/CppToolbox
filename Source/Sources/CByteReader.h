//----------------------------------------------------------------------------------------------------------------------
//	CByteReader.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CUUID.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CByteReader

class CByteReaderInternals;
class CByteReader {
	// Position
	public:
		enum Position {
			kPositionFromBeginning,
			kPositionFromCurrent,
			kPositionFromEnd,
		};

	// Methods
	public:
							// Lifecycle methods
							CByteReader(const I<CRandomAccessDataSource>& randomAccessDataSource, bool isBigEndian);
							CByteReader(const I<CRandomAccessDataSource>& randomAccessDataSource, UInt64 offset,
									UInt64 size, bool isBigEndian);
							CByteReader(const CByteReader& other);
							~CByteReader();

							// Instance methods
		UInt64				getByteCount() const;

		UInt64				getPos() const;
		OI<SError>			setPos(Position position, SInt64 newPos) const;

		OI<SError>			readData(void* buffer, UInt64 byteCount) const;
		TIResult<CData>		readData(CData::ByteCount byteCount) const;

		TVResult<SInt8>		readSInt8() const;
		TVResult<SInt16>	readSInt16() const;
		TVResult<SInt32>	readSInt32() const;
		TVResult<SInt64>	readSInt64() const;
		TVResult<UInt8>		readUInt8() const;
		TVResult<UInt16>	readUInt16() const;
		TVResult<UInt32>	readUInt32() const;
		TVResult<UInt64>	readUInt64() const;
		TVResult<OSType>	readOSType() const;
		TIResult<CUUID>		readUUID() const;

	// Properties
	private:
		CByteReaderInternals*	mInternals;
};
