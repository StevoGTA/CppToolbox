//----------------------------------------------------------------------------------------------------------------------
//	CData.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Size

typedef	UInt32	CDataSize;

const	CDataSize	kDataBytesAll = 0;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Byte Index

typedef	UInt32	CDataByteIndex;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CData

class CDataInternals;
class CData {
	// Methods
	public:
							// Lifecycle methods
							CData(CDataSize initialSize = 0);
							CData(const CData& other);
							CData(const void* buffer, CDataSize bufferSize, bool copySourceData = true);
							CData(const CString& base64String);
							~CData();

							// Instance methods
				CDataSize	getSize() const;
				void		setSize(CDataSize size);
				void		increaseSizeBy(CDataSize size);
				bool		isEmpty() const
								{ return getSize() == 0; }

		const	void*		getBytePtr() const;
				void*		getMutableBytePtr();
				void		copyBytes(void* destinationBuffer, CDataByteIndex startByte = 0,
									CDataSize byteCount = kDataBytesAll) const;
				void		appendBytes(const void* buffer, CDataSize bufferSize);
				void		replaceBytes(CDataByteIndex startByte, CDataSize byteCount, const void* buffer,
									CDataSize bufferSize);

				CString		getBase64String(bool prettyPrint = false) const;

				CData&		operator=(const CData& other);
				bool		operator==(const CData& other) const;
				bool		operator!=(const CData& other) const
								{ return !operator==(other); }
				CData		operator+(const CData& other) const;
				CData&		operator+=(const CData& other)
								{ appendBytes(other.getBytePtr(), other.getSize()); return *this; }

	// Properties
	public:
		static	CData			mEmpty;
		static	CData			mZeroByte;

	private:
				CDataInternals*	mInternals;
};
