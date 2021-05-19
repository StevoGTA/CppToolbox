//----------------------------------------------------------------------------------------------------------------------
//	CBitParceller.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "TInstance.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBitParceller

class CBitParcellerInternals;
class CBitParceller {
	// Methods
	public:
					// Lifecycle methods
					CBitParceller(const I<CDataSource>& dataSource, bool isBigEndian);
					CBitParceller(const I<CDataSource>& dataSource, UInt64 offset, UInt64 size, bool isBigEndian);
					CBitParceller(const CBitParceller& other);
					~CBitParceller();

					// Instance methods
		UInt64		getSize() const;

		SInt64		getPos() const;	// Will return next byte pos if bits still to read in current byte
		OI<SError>	setPos(CDataSource::Position position, SInt64 newPos) const;	// kPositionFromCurrent of 0 advances to next byte boundary

		OI<SError>	readData(void* buffer, UInt64 byteCount) const;
		OI<CData>	readData(UInt64 byteCount, OI<SError>& outError) const;

		OV<SInt8>	readSInt8(OI<SError>& outError) const;	// Will ignore bits remaining in the current byte
		OV<SInt8>	readSInt8(UInt8 bitCount, OI<SError>& outError) const;
		OV<SInt16>	readSInt16(OI<SError>& outError) const;	// Will ignore bits remaining in the current byte
		OV<SInt16>	readSInt16(UInt8 bitCount, OI<SError>& outError) const;
		OV<SInt32>	readSInt32(OI<SError>& outError) const;	// Will ignore bits remaining in the current byte
		OV<SInt32>	readSInt32(UInt8 bitCount, OI<SError>& outError) const;
		OV<SInt64>	readSInt64(OI<SError>& outError) const;	// Will ignore bits remaining in the current byte
		OV<SInt64>	readSInt64(UInt8 bitCount, OI<SError>& outError) const;
		OV<UInt8>	readUInt8(OI<SError>& outError) const;	// Will ignore bits remaining in the current byte
		OV<UInt8>	readUInt8(UInt8 bitCount, OI<SError>& outError) const;
		OV<UInt16>	readUInt16(OI<SError>& outError) const;	// Will ignore bits remaining in the current byte
		OV<UInt16>	readUInt16(UInt8 bitCount, OI<SError>& outError) const;
		OV<UInt32>	readUInt32(OI<SError>& outError) const;	// Will ignore bits remaining in the current byte
		OV<UInt32>	readUInt32(UInt8 bitCount, OI<SError>& outError) const;
		OV<UInt64>	readUInt64(OI<SError>& outError) const;	// Will ignore bits remaining in the current byte
		OV<UInt64>	readUInt64(UInt8 bitCount, OI<SError>& outError) const;

		OV<UInt32>	readUEColumbusCode(OI<SError>& outError) const;

		void		reset() const;

	// Properties
	private:
		CBitParcellerInternals*	mInternals;
};
