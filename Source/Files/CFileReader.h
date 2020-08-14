//----------------------------------------------------------------------------------------------------------------------
//	CFileReader.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Position mode

enum EFileReaderPositionMode {
	kFileReaderPositionModeFromBeginning,
	kFileReaderPositionModeFromCurrent,
	kFileReaderPositionModeFromEnd,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileMemoryMap

struct SFileMemoryMapSetupInfo;

class CFileMemoryMapInternals;
class CFileMemoryMap {
	// Methods
	public:
						// Lifecycle methods
						CFileMemoryMap(SFileMemoryMapSetupInfo& fileMemoryMapSetupInfo);
						CFileMemoryMap(const CFileMemoryMap& other);
						~CFileMemoryMap();

						// Instance methods
		const	void*	getBytePtr() const;
				UInt64	getByteCount() const;

				bool	isValid() const
							{ return getBytePtr() != nil; }

	// Properties
	private:
		CFileMemoryMapInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileReader

class CFileReaderInternals;
class CFileReader {
	// Methods
	public:
								// Lifecycle methods
								CFileReader(const CFile& file);
								CFileReader(const CFileReader& other);
								~CFileReader();

								// Instance methods
		const	CFile&			getFile() const;

				UError			open(bool buffered = false);

				UError			readData(void* buffer, UInt64 byteCount) const;
				CData			readData(UInt64 byteCount, UError& outError) const;
				SInt8			readSInt8(UError& outError) const
									{
										// Read
										SInt8	value = 0;
										outError = readData(&value, sizeof(SInt8));

										return value;
									}
				SInt16			readSInt16(UError& outError) const
									{
										// Read
										SInt16	value = 0;
										outError = readData(&value, sizeof(SInt16));

										return value;
									}
				SInt32			readSInt32(UError& outError) const
									{
										// Read
										SInt32	value = 0;
										outError = readData(&value, sizeof(SInt32));

										return value;
									}
				SInt64			readSInt64(UError& outError) const
									{
										// Read
										SInt64	value = 0;
										outError = readData(&value, sizeof(SInt64));

										return value;
									}
				UInt8			readUInt8(UError& outError) const
									{
										// Read
										UInt8	value = 0;
										outError = readData(&value, sizeof(UInt8));

										return value;
									}
				UInt16			readUInt16(UError& outError) const
									{
										// Read
										UInt16	value = 0;
										outError = readData(&value, sizeof(UInt16));

										return value;
									}
				UInt32			readUInt32(UError& outError) const
									{
										// Read
										UInt32	value = 0;
										outError = readData(&value, sizeof(UInt32));

										return value;
									}
				UInt64			readUInt64(UError& outError) const
									{
										// Read
										UInt64	value = 0;
										outError = readData(&value, sizeof(UInt64));

										return value;
									}
				OSType			readOSType(UError& outError) const
									{
										// Read
										OSType	value = 0;
										outError = readData(&value, sizeof(OSType));

										return value;
									}

				SInt64			getPos() const;
				UError			setPos(EFileReaderPositionMode mode, SInt64 newPos) const;

				CFileMemoryMap	getFileMemoryMap(UInt64 byteOffset, UInt64 byteCount, UError& outError) const;

				UError			close() const;

	// Properties
	private:
		CFileReaderInternals*	mInternals;
};
