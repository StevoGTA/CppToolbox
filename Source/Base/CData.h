//----------------------------------------------------------------------------------------------------------------------
//	CData.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CData

class CDataInternals;
class CData {
	// Types
	public:
		typedef	UInt64	Size;
		typedef	UInt64	ByteIndex;

	// Methods
	public:
						// Lifecycle methods
						CData(Size initialSize = 0);
						CData(const CData& other);
						CData(const void* buffer, Size bufferSize, bool copySourceData = true);
						CData(const CString& base64String);
						CData(SInt8 value);
						CData(UInt8 value);
						~CData();

						// Instance methods
				Size	getSize() const;
				void	setSize(Size size);
				void	increaseSizeBy(Size size);
				bool	isEmpty() const
							{ return getSize() == 0; }

		const	void*	getBytePtr() const;
				void*	getMutableBytePtr();
				void	copyBytes(void* destinationBuffer, ByteIndex startByte = 0, OV<Size> count = OV<Size>()) const;
				void	appendBytes(const void* buffer, Size bufferSize);
				void	replaceBytes(ByteIndex startByte, Size byteCount, const void* buffer, Size bufferSize);

				CString	getBase64String(bool prettyPrint = false) const;

				CData&	operator=(const CData& other);
				bool	operator==(const CData& other) const;
				bool	operator!=(const CData& other) const
							{ return !operator==(other); }
				CData	operator+(const CData& other) const;
				CData&	operator+=(const CData& other)
							{ appendBytes(other.getBytePtr(), other.getSize()); return *this; }

	// Properties
	public:
		static	CData			mEmpty;
		static	CData			mZeroByte;

	private:
				CDataInternals*	mInternals;
};
