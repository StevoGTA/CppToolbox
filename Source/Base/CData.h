//----------------------------------------------------------------------------------------------------------------------
//	CData.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TBuffer.h"
#include "TRange.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CData

class CString;

class CData {
	// Types
	public:
		typedef	UInt64	ByteCount;
		typedef	UInt64	ByteIndex;

	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CData(ByteCount preallocatedByteCount = 1024);
										CData(ByteCount byteCount, UInt8 fillValue);
										CData(const void* buffer, ByteCount bufferByteCount,
												bool copySourceData = true);
										CData(const CData& other);
										~CData();

										// Instance methods
						ByteCount		getByteCount() const;
						bool			isEmpty() const
											{ return getByteCount() == 0; }

				const	void*			getBytePtr() const;
						void			copyBytes(void* destinationBuffer, ByteIndex startByteIndex,
												ByteCount byteCount) const;

						CString			getBase64String(bool prettyPrint = false) const;
						CString			getHexString(bool uppercase = false) const;

						CData			subData(ByteIndex byteIndex, ByteCount byteCount) const;
						CData			subData(ByteIndex byteIndex) const;
						OV<SRange64>	findSubData(const CData& subData, ByteIndex startIndex = 0,
												const OV<ByteCount>& byteCount = OV<ByteCount>()) const;

						TBuffer<UInt8>	getMutableBuffer(ByteIndex byteIndex, ByteCount byteCount);
						TBuffer<UInt8>	getMutableBuffer(ByteCount byteCount);
						CData&			append(const void* buffer, ByteCount bufferByteCount);
						CData&			append(const CData& data)
											{ return append(data.getBytePtr(), data.getByteCount()); }
						CData&			append(SInt16 value)
											{ return append(&value, sizeof(SInt16)); }
						CData&			append(UInt16 value)
											{ return append(&value, sizeof(UInt16)); }
						CData&			append(UInt32 value)
											{ return append(&value, sizeof(UInt32)); }
						CData&			replace(ByteIndex startByteIndex, ByteCount byteCount, const void* buffer,
												ByteCount bufferByteCount);
						CData&			replace(ByteIndex startByteIndex, const CData& data)
											{ return replace(startByteIndex, data.getByteCount(),
													data.getBytePtr(), data.getByteCount()); }
						CData&			replace(ByteIndex startByteIndex, UInt32 value)
											{ return replace(startByteIndex, sizeof(UInt32), &value,
													sizeof(UInt32)); }

						CData&			operator=(const CData& other);
						bool			operator==(const CData& other) const;
						bool			operator!=(const CData& other) const
											{ return !operator==(other); }
						CData			operator+(const CData& other) const;
						CData&			operator+=(const CData& other)
											{ return append(other.getBytePtr(), other.getByteCount()); }

										// Class methods
		static			CData			fromBase64String(const CString& base64String);
		static			CData			storing(SInt32 value, bool copyValue = true)
											{ return CData(&value, sizeof(SInt32), copyValue); }
		static			CData			storing(SInt8 value, bool copyValue = true)
											{ return CData(&value, sizeof(SInt8), copyValue); }
		static			CData			storing(UInt8 value, bool copyValue = true)
											{ return CData(&value, sizeof(UInt8), copyValue); }
		static			CData			storing(UInt16 value, bool copyValue = true)
											{ return CData(&value, sizeof(UInt16), copyValue); }
		static			CData			storing(UInt32 value, bool copyValue = true)
											{ return CData(&value, sizeof(UInt32), copyValue); }

	// Properties
	public:
		static	const	CData		mEmpty;
		static	const	CData		mZeroByte;

	private:
						Internals*	mInternals;
};
