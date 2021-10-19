//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSource

class CFileDataSourceInternals;
class CFileDataSource : public CSeekableDataSource {
	// Methods
	public:
								// Lifecycle methods
								CFileDataSource(const CFile& file, bool buffered = false);
								~CFileDataSource();

								// CSeekableDataSource methods
		UInt64					getByteCount() const;

		OI<SError>				readData(UInt64 position, void* buffer, CData::ByteCount byteCount);

								// Class methods
		static	TIResult<CData>	readData(const CFile& file)
									{ return CFileDataSource(file).CSeekableDataSource::readData(); }

	// Properties
	private:
		CFileDataSourceInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSource

class CMappedFileDataSourceInternals;
class CMappedFileDataSource : public CSeekableDataSource {
	// Methods
	public:
					// Lifecycle methods
					CMappedFileDataSource(const CFile& file, UInt64 byteOffset, UInt64 byteCount);
					CMappedFileDataSource(const CFile& file);
					~CMappedFileDataSource();

					// CSeekableDataSource methods
		UInt64		getByteCount() const;

		OI<SError>	readData(UInt64 position, void* buffer, CData::ByteCount byteCount);

	// Properties
	private:
		CMappedFileDataSourceInternals*	mInternals;
};
