//----------------------------------------------------------------------------------------------------------------------
//	CByteParceller.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CDataSource.h"
#include "TInstance.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CByteParceller

class CByteParcellerInternals;
class CByteParceller {
	// Methods
	public:
					// Lifecycle methods
					CByteParceller(const I<CDataSource>& dataSource, bool isBigEndian);
					CByteParceller(const I<CDataSource>& dataSource, UInt64 offset, UInt64 size, bool isBigEndian);
					CByteParceller(const CByteParceller& other);
					~CByteParceller();

					// Instance methods
		UInt64		getSize() const;

		SInt64		getPos() const;
		OI<SError>	setPos(CDataSource::Position position, SInt64 newPos) const;

		OI<SError>	readData(void* buffer, UInt64 byteCount) const;
		OI<CData>	readData(UInt64 byteCount, OI<SError>& outError) const;

		OV<SInt8>	readSInt8(OI<SError>& outError) const;
		OV<SInt16>	readSInt16(OI<SError>& outError) const;
		OV<SInt32>	readSInt32(OI<SError>& outError) const;
		OV<SInt64>	readSInt64(OI<SError>& outError) const;
		OV<UInt8>	readUInt8(OI<SError>& outError) const;
		OV<UInt16>	readUInt16(OI<SError>& outError) const;
		OV<UInt32>	readUInt32(OI<SError>& outError) const;
		OV<UInt64>	readUInt64(OI<SError>& outError) const;
		OV<OSType>	readOSType(OI<SError>& outError) const;

		void		reset() const;

	// Properties
	private:
		CByteParcellerInternals*	mInternals;
};
