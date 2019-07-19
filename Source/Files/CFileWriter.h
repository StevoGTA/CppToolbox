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
		UError	open(bool append = false, bool buffered = false, bool removeIfNotClosed = false) const;

		UError	write(const void* buffer, UInt64 byteCount) const;
		UError	write(const CData& data) const
					{ return write(data.getBytePtr(), data.getSize()); }
		UError	write(const CString& string, EStringEncoding stringEncoding = kStringEncodingTextDefault) const
					{  return write(string.getData(stringEncoding)); }
		UError	write(SInt8 value) const
					{ return write(&value, sizeof(SInt8)); }
		UError	write(SInt16 value) const
					{ return write(&value, sizeof(SInt16)); }
		UError	write(SInt32 value) const
					{ return write(&value, sizeof(SInt32)); }
		UError	write(SInt64 value) const
					{ return write(&value, sizeof(SInt64)); }
		UError	write(UInt8 value) const
					{ return write(&value, sizeof(UInt8)); }
		UError	write(UInt16 value) const
					{ return write(&value, sizeof(UInt16)); }
		UError	write(UInt32 value) const
					{ return write(&value, sizeof(UInt32)); }
		UError	write(UInt64 value) const
					{ return write(&value, sizeof(UInt64)); }

		SInt64	getPos() const;
		UError	setPos(EFileWriterPositionMode mode, SInt64 newPos) const;
		UError	setSize(UInt64 newSize) const;

		UError	close() const;

	// Properties
	private:
		CFileWriterInternals*	mInternals;
};
