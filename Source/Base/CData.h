//----------------------------------------------------------------------------------------------------------------------
//	CData.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

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
										CData(ByteCount initialByteCount = 0);
										CData(const CData& other);
										CData(const void* buffer, ByteCount bufferByteCount,
												bool copySourceData = true);
										~CData();

										// Instance methods
						ByteCount		getByteCount() const;
						void			setByteCount(ByteCount byteCount);
						void			increaseByteCountBy(ByteCount byteCount);
						bool			isEmpty() const
											{ return getByteCount() == 0; }

				const	void*			getBytePtr() const;
						void*			getMutableBytePtr();
						void			copyBytes(void* destinationBuffer, ByteIndex startByteIndex,
												ByteCount byteCount) const;
						void			appendBytes(const void* buffer, ByteCount bufferByteCount);
						void			replaceBytes(ByteIndex startByteIndex, ByteCount byteCount, const void* buffer,
												ByteCount bufferByteCount);

						CString			getBase64String(bool prettyPrint = false) const;
						CString			getHexString(bool uppercase = false) const;

						CData			subData(ByteIndex byteIndex, const OV<ByteCount>& byteCount = OV<ByteCount>(),
												bool copySourceData = true) const;
						CData			subData(ByteIndex byteIndex, ByteCount byteCount, bool copySourceData = true)
												const
											{ return subData(byteIndex, OV<ByteCount>(byteCount), copySourceData); }
						CData			subData(const OV<ByteCount>& byteCount = OV<ByteCount>(),
												bool copySourceData = true) const
											{ return subData(0, byteCount, copySourceData); }
						OV<SRange64>	findSubData(const CData& subData, ByteIndex startIndex = 0,
												const OV<ByteCount>& byteCount = OV<ByteCount>()) const;

						CData&			operator=(const CData& other);
						bool			operator==(const CData& other) const;
						bool			operator!=(const CData& other) const
											{ return !operator==(other); }
						CData			operator+(const CData& other) const;
						CData&			operator+=(const CData& other)
											{ appendBytes(other.getBytePtr(), other.getByteCount()); return *this; }

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
