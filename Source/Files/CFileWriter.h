//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileWriter

class CFileWriterInternals;
class CFileWriter {
	// Enums
	public:
		enum Position {
			kPositionFromBeginning,
			kPositionFromCurrent,
			kPositionFromEnd,
		};

	// Methods
	public:
					// Lifecycle methods
					CFileWriter(const CFile& file);
					CFileWriter(const CFileWriter& other);
					~CFileWriter();

					// Instance methods
		OV<SError>	open(bool append = false, bool buffered = false, bool removeIfNotClosed = false) const;

		OV<SError>	write(const void* buffer, UInt64 byteCount) const;
		OV<SError>	write(const CData& data) const
						{ return write(data.getBytePtr(), data.getByteCount()); }
		OV<SError>	write(const CString& string, CString::Encoding stringEncoding = CString::kEncodingTextDefault) const
						{  return write(string.getData(stringEncoding)); }
		OV<SError>	write(SInt8 value) const
						{ return write(&value, sizeof(SInt8)); }
		OV<SError>	write(SInt16 value) const
						{ return write(&value, sizeof(SInt16)); }
		OV<SError>	write(SInt32 value) const
						{ return write(&value, sizeof(SInt32)); }
		OV<SError>	write(SInt64 value) const
						{ return write(&value, sizeof(SInt64)); }
		OV<SError>	write(UInt8 value) const
						{ return write(&value, sizeof(UInt8)); }
		OV<SError>	write(UInt16 value) const
						{ return write(&value, sizeof(UInt16)); }
		OV<SError>	write(UInt32 value) const
						{ return write(&value, sizeof(UInt32)); }
		OV<SError>	write(UInt64 value) const
						{ return write(&value, sizeof(UInt64)); }

		UInt64		getPos() const;
		OV<SError>	setPos(Position position, SInt64 newPos) const;
		OV<SError>	setByteCount(UInt64 byteCount) const;

		OV<SError>	flush() const;

		OV<SError>	close() const;

	// Properties
	private:
		CFileWriterInternals*	mInternals;
};
