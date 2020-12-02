//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Position mode

enum EFileWriterPositionMode {
	kFileWriterPositionModeFromBeginning,
	kFileWriterPositionModeFromCurrent,
	kFileWriterPositionModeFromEnd,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileWriter

class CFileWriterInternals;
class CFileWriter {
	// Methods
	public:
					// Lifecycle methods
					CFileWriter(const CFile& file);
					CFileWriter(const CFileWriter& other);
					~CFileWriter();

					// Instance methods
		OI<SError>	open(bool append = false, bool buffered = false, bool removeIfNotClosed = false) const;

		OI<SError>	write(const void* buffer, UInt64 byteCount) const;
		OI<SError>	write(const CData& data) const
						{ return write(data.getBytePtr(), data.getSize()); }
		OI<SError>	write(const CString& string, EStringEncoding stringEncoding = kStringEncodingTextDefault) const
						{  return write(string.getData(stringEncoding)); }
		OI<SError>	write(SInt8 value) const
						{ return write(&value, sizeof(SInt8)); }
		OI<SError>	write(SInt16 value) const
						{ return write(&value, sizeof(SInt16)); }
		OI<SError>	write(SInt32 value) const
						{ return write(&value, sizeof(SInt32)); }
		OI<SError>	write(SInt64 value) const
						{ return write(&value, sizeof(SInt64)); }
		OI<SError>	write(UInt8 value) const
						{ return write(&value, sizeof(UInt8)); }
		OI<SError>	write(UInt16 value) const
						{ return write(&value, sizeof(UInt16)); }
		OI<SError>	write(UInt32 value) const
						{ return write(&value, sizeof(UInt32)); }
		OI<SError>	write(UInt64 value) const
						{ return write(&value, sizeof(UInt64)); }

		SInt64		getPos() const;
		OI<SError>	setPos(EFileWriterPositionMode mode, SInt64 newPos) const;
		OI<SError>	setSize(UInt64 newSize) const;

		OI<SError>	flush() const;

		OI<SError>	close() const;

	// Properties
	private:
		CFileWriterInternals*	mInternals;
};
