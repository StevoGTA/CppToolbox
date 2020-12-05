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
		CData		readData(UInt64 byteCount, OI<SError>& outError) const;
		CData		readData(OI<SError>& outError) const;
		SInt8		readSInt8(OI<SError>& outError) const
						{
							// Read
							SInt8	value = 0;
							outError = readData(&value, sizeof(SInt8));

							return value;
						}
		SInt16		readSInt16(OI<SError>& outError) const
						{
							// Read
							SInt16	value = 0;
							outError = readData(&value, sizeof(SInt16));

							return value;
						}
		SInt32		readSInt32(OI<SError>& outError) const
						{
							// Read
							SInt32	value = 0;
							outError = readData(&value, sizeof(SInt32));

							return value;
						}
		SInt64		readSInt64(OI<SError>& outError) const
						{
							// Read
							SInt64	value = 0;
							outError = readData(&value, sizeof(SInt64));

							return value;
						}
		UInt8		readUInt8(OI<SError>& outError) const
						{
							// Read
							UInt8	value = 0;
							outError = readData(&value, sizeof(UInt8));

							return value;
						}
		UInt16		readUInt16(OI<SError>& outError) const
						{
							// Read
							UInt16	value = 0;
							outError = readData(&value, sizeof(UInt16));

							return value;
						}
		UInt32		readUInt32(OI<SError>& outError) const
						{
							// Read
							UInt32	value = 0;
							outError = readData(&value, sizeof(UInt32));

							return value;
						}
		UInt64		readUInt64(OI<SError>& outError) const
						{
							// Read
							UInt64	value = 0;
							outError = readData(&value, sizeof(UInt64));

							return value;
						}
		OSType		readOSType(OI<SError>& outError) const
						{
							// Read
							OSType	value = 0;
							outError = readData(&value, sizeof(OSType));

							return value;
						}

		SInt64		getPos() const;
		OI<SError>	setPos(CDataSource::Position position, SInt64 newPos) const;

		void		reset() const;

	// Properties
	private:
		CByteParcellerInternals*	mInternals;
};
