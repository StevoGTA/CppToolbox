//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CFile.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileWriter

class CFileWriter {
	// Enums
	public:
		enum Mode {
			kModeCreate,
			kModeReplace,
			kModeAppend,
		};

		enum Position {
			kPositionFromBeginning,
			kPositionFromCurrent,
			kPositionFromEnd,
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
		explicit							CFileWriter(const CFile& file);
											CFileWriter(const CFileWriter& other);
		virtual								~CFileWriter();

											// Instance methods
					const	CFile&			getFile() const;

							bool			isOpen() const;
							OV<SError>		open(Mode mode = kModeCreate, bool buffered = false,
													bool removeIfNotClosed = false) const;

							TVResult<CData>	read(CData::ByteCount byteCount) const;

							OV<SError>		write(const CData& data) const
												{ return write(*data.getUInt8Buffer(), data.getByteCount()); }

											template <typename T>
							OV<SError>		write(const TBuffer<T>& buffer) const
												{ return write(*buffer, buffer.getByteCount()); }

							OV<SError>		write(const CString& string) const
												{ return write(string.getUTF8Data()); }
							OV<SError>		write(SInt8 value) const
												{ return write(&value, sizeof(SInt8)); }
							OV<SError>		write(SInt16 value) const
												{ return write(&value, sizeof(SInt16)); }
							OV<SError>		write(SInt32 value) const
												{ return write(&value, sizeof(SInt32)); }
							OV<SError>		write(SInt64 value) const
												{ return write(&value, sizeof(SInt64)); }
							OV<SError>		write(UInt8 value) const
												{ return write(&value, sizeof(UInt8)); }
							OV<SError>		write(UInt16 value) const
												{ return write(&value, sizeof(UInt16)); }
							OV<SError>		write(UInt32 value) const
												{ return write(&value, sizeof(UInt32)); }
							OV<SError>		write(UInt64 value) const
												{ return write(&value, sizeof(UInt64)); }
							OV<SError>		write(const void* buffer, UInt64 byteCount) const;

							UInt64			getPosition() const;
							OV<SError>		setPosition(Position position, SInt64 newPos) const;
							OV<SError>		setByteCount(UInt64 byteCount) const;

							OV<SError>		flush() const;

							OV<SError>		close() const;

							CFileWriter&	operator=(const CFileWriter& other);

											// Class methods
		static				OV<SError>		write(const CFile& file, const CData& data);
		static				OV<SError>		write(const CFile& file, const CFile::AppleMetadata& appleMetadata);

	// Properties
	private:
		Internals*	mInternals;
};
