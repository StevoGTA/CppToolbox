//----------------------------------------------------------------------------------------------------------------------
//	CFileReader.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileMemoryMap

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
										CFileReader(const CFile& file);
										CFileReader(const CFileReader& other);
										~CFileReader();

										// Instance methods
				const	CFile&			getFile() const;

						OI<SError>		open(bool buffered = false);

						OI<SError>		readData(void* buffer, UInt64 byteCount) const;
						CData			readData(UInt64 byteCount, OI<SError>& outError) const;
						SInt8			readSInt8(OI<SError>& outError) const
											{
												// Read
												SInt8	value = 0;
												outError = readData(&value, sizeof(SInt8));

												return value;
											}
						SInt16			readSInt16(OI<SError>& outError) const
											{
												// Read
												SInt16	value = 0;
												outError = readData(&value, sizeof(SInt16));

												return value;
											}
						SInt32			readSInt32(OI<SError>& outError) const
											{
												// Read
												SInt32	value = 0;
												outError = readData(&value, sizeof(SInt32));

												return value;
											}
						SInt64			readSInt64(OI<SError>& outError) const
											{
												// Read
												SInt64	value = 0;
												outError = readData(&value, sizeof(SInt64));

												return value;
											}
						UInt8			readUInt8(OI<SError>& outError) const
											{
												// Read
												UInt8	value = 0;
												outError = readData(&value, sizeof(UInt8));

												return value;
											}
						UInt16			readUInt16(OI<SError>& outError) const
											{
												// Read
												UInt16	value = 0;
												outError = readData(&value, sizeof(UInt16));

												return value;
											}
						UInt32			readUInt32(OI<SError>& outError) const
											{
												// Read
												UInt32	value = 0;
												outError = readData(&value, sizeof(UInt32));

												return value;
											}
						UInt64			readUInt64(OI<SError>& outError) const
											{
												// Read
												UInt64	value = 0;
												outError = readData(&value, sizeof(UInt64));

												return value;
											}
						OSType			readOSType(OI<SError>& outError) const
											{
												// Read
												OSType	value = 0;
												outError = readData(&value, sizeof(OSType));

												return value;
											}

						SInt64			getPos() const;
						OI<SError>		setPos(Position position, SInt64 newPos) const;

						CFileMemoryMap	getFileMemoryMap(UInt64 byteOffset, UInt64 byteCount,
												OI<SError>& outError) const;

						OI<SError>		close() const;

										// Class methods
		static	CData					readData(const CFile& file, OI<SError>& outError);

	// Properties
	private:
		CFileReaderInternals*	mInternals;
};
