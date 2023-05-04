//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSource

class CFileDataSource : public CRandomAccessDataSource {
	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CFileDataSource(const CFile& file, bool buffered = false);
								~CFileDataSource();

								// CRandomAccessDataSource methods
		UInt64					getByteCount() const;

		OV<SError>				readData(UInt64 position, void* buffer, CData::ByteCount byteCount);
		TVResult<CData>			readData(UInt64 position, CData::ByteCount byteCount)
									{
										// Read data
										CData		data(byteCount);
										OV<SError>	error = readData(position, data.getMutableBytePtr(), byteCount);

										return !error.hasValue() ? TVResult<CData>(data) : TVResult<CData>(*error);
									}

								// Class methods
		static	TVResult<CData>	readData(const CFile& file)
									{ return CFileDataSource(file).CRandomAccessDataSource::readData(); }

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSource

class CMappedFileDataSource : public CRandomAccessDataSource {
	// Classes
	private:
		class Internals;

	// Methods
	public:
						// Lifecycle methods
						CMappedFileDataSource(const CFile& file, UInt64 byteOffset, UInt64 byteCount);
						CMappedFileDataSource(const CFile& file);
						~CMappedFileDataSource();

						// CRandomAccessDataSource methods
		UInt64			getByteCount() const;

		OV<SError>		readData(UInt64 position, void* buffer, CData::ByteCount byteCount);
		TVResult<CData>	readData(UInt64 position, CData::ByteCount byteCount);

	// Properties
	private:
		Internals*	mInternals;
};
