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
					CByteParceller(const I<CDataSource>& dataSource);
					CByteParceller(const CByteParceller& other, UInt64 offset, UInt64 size);	// "Sub" CByteParceller
					CByteParceller(const CByteParceller& other);
					~CByteParceller();

					// Instance methods
		UInt64		getSize() const;

		OI<SError>	readData(void* buffer, UInt64 byteCount) const;
		OI<CData>	readData(UInt64 byteCount, OI<SError>& outError) const;
		OI<CData>	readData(OI<SError>& outError) const;
		OV<SInt8>	readSInt8(OI<SError>& outError) const
						{
							// Read
							SInt8	value = 0;
							outError = readData(&value, sizeof(SInt8));
							ReturnValueIfError(outError, OV<SInt8>());

							return OV<SInt8>(value);
						}
		OV<SInt16>	readSInt16(OI<SError>& outError) const
						{
							// Read
							SInt16	value = 0;
							outError = readData(&value, sizeof(SInt16));
							ReturnValueIfError(outError, OV<SInt16>());

							return OV<SInt16>(value);
						}
		OV<SInt32>	readSInt32(OI<SError>& outError) const
						{
							// Read
							SInt32	value = 0;
							outError = readData(&value, sizeof(SInt32));
							ReturnValueIfError(outError, OV<SInt32>());

							return OV<SInt32>(value);
						}
		OV<SInt64>	readSInt64(OI<SError>& outError) const
						{
							// Read
							SInt64	value = 0;
							outError = readData(&value, sizeof(SInt64));
							ReturnValueIfError(outError, OV<SInt64>());

							return OV<SInt64>(value);
						}
		OV<UInt8>	readUInt8(OI<SError>& outError) const
						{
							// Read
							UInt8	value = 0;
							outError = readData(&value, sizeof(UInt8));
							ReturnValueIfError(outError, OV<UInt8>());

							return OV<UInt8>(value);
						}
		OV<UInt16>	readUInt16(OI<SError>& outError) const
						{
							// Read
							UInt16	value = 0;
							outError = readData(&value, sizeof(UInt16));
							ReturnValueIfError(outError, OV<UInt16>());

							return OV<UInt16>(value);
						}
		OV<UInt32>	readUInt32(OI<SError>& outError) const
						{
							// Read
							UInt32	value = 0;
							outError = readData(&value, sizeof(UInt32));
							ReturnValueIfError(outError, OV<UInt32>());

							return OV<UInt32>(value);
						}
		OV<UInt64>	readUInt64(OI<SError>& outError) const
						{
							// Read
							UInt64	value = 0;
							outError = readData(&value, sizeof(UInt64));
							ReturnValueIfError(outError, OV<UInt64>());

							return OV<UInt64>(value);
						}
		OV<OSType>	readOSType(OI<SError>& outError) const
						{
							// Read
							OSType	value = 0;
							outError = readData(&value, sizeof(OSType));
							ReturnValueIfError(outError, OV<OSType>());

							return OV<OSType>(value);
						}

		SInt64		getPos() const;
		OI<SError>	setPos(CDataSource::Position position, SInt64 newPos) const;

		void		reset() const;

	// Properties
	private:
		CByteParcellerInternals*	mInternals;
};
