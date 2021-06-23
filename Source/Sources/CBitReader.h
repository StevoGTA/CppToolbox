//----------------------------------------------------------------------------------------------------------------------
//	CBitReader.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBitReader

class CBitReaderInternals;
class CBitReader {
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
							CBitReader(const I<CSeekableDataSource>& seekableDataSource, bool isBigEndian);
							CBitReader(const CBitReader& other);
							~CBitReader();

							// Instance methods
		UInt64				getSize() const;

		UInt64				getPos() const;	// Will return next byte pos if bits still to read in current byte
		OI<SError>			setPos(Position position, SInt64 newPos) const;	// kPositionFromCurrent of 0 advances to next byte boundary

		OI<SError>			readData(void* buffer, UInt64 byteCount) const;
		TIResult<CData>		readData(CData::Size byteCount) const;

		TVResult<SInt8>		readSInt8() const;	// Will ignore bits remaining in the current byte
		TVResult<SInt16>	readSInt16() const;	// Will ignore bits remaining in the current byte
		TVResult<SInt32>	readSInt32() const;	// Will ignore bits remaining in the current byte
		TVResult<SInt64>	readSInt64() const;	// Will ignore bits remaining in the current byte
		TVResult<UInt8>		readUInt8() const;	// Will ignore bits remaining in the current byte
		TVResult<UInt8>		readUInt8(UInt8 bitCount) const;
		TVResult<UInt16>	readUInt16() const;	// Will ignore bits remaining in the current byte
		TVResult<UInt32>	readUInt32() const;	// Will ignore bits remaining in the current byte
		TVResult<UInt32>	readUInt32(UInt8 bitCount) const;
		TVResult<UInt64>	readUInt64() const;	// Will ignore bits remaining in the current byte

		TVResult<UInt32>	readUEColumbusCode() const;

	// Properties
	private:
		CBitReaderInternals*	mInternals;
};
