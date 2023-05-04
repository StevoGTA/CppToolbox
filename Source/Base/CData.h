//----------------------------------------------------------------------------------------------------------------------
//	CData.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

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
							CData(const void* buffer, ByteCount bufferByteCount, bool copySourceData = true);
							CData(const CString& base64String);
							CData(SInt8 value);
							CData(UInt8 value);
							~CData();

							// Instance methods
				ByteCount	getByteCount() const;
				void		setByteCount(ByteCount byteCount);
				void		increaseByteCountBy(ByteCount byteCount);
				bool		isEmpty() const
								{ return getByteCount() == 0; }

		const	void*		getBytePtr() const;
				void*		getMutableBytePtr();
				void		copyBytes(void* destinationBuffer, ByteIndex startByteIndex = 0,
									OV<ByteCount> byteCount = OV<ByteCount>()) const;
				void		appendBytes(const void* buffer, ByteCount bufferByteCount);
				void		replaceBytes(ByteIndex startByteIndex, ByteCount byteCount, const void* buffer,
									ByteCount bufferByteCount);

				CString		getBase64String(bool prettyPrint = false) const;

				CData		subData(ByteIndex byteIndex, const OV<ByteCount>& byteCount = OV<ByteCount>(),
									bool copySourceData = true) const;

				CData&		operator=(const CData& other);
				bool		operator==(const CData& other) const;
				bool		operator!=(const CData& other) const
								{ return !operator==(other); }
				CData		operator+(const CData& other) const;
				CData&		operator+=(const CData& other)
								{ appendBytes(other.getBytePtr(), other.getByteCount()); return *this; }

	// Properties
	public:
		static	const	CData		mEmpty;
		static	const	CData		mZeroByte;

	private:
						Internals*	mInternals;
};
