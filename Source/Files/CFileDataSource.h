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
				UInt64						getByteCount() const;

				OV<SError>					read(UInt64 position, void* buffer, UInt64 byteCount);
				TVResult<CData>				readData(UInt64 position, CData::ByteCount byteCount)
												{
													// Read data
													CData		data(byteCount);
													OV<SError>	error =
																		read(position,
																				*data.getMutableBuffer(byteCount),
																				byteCount);

													return !error.hasValue() ?
															TVResult<CData>(data) : TVResult<CData>(*error);
												}
				TVResult<TBuffer<UInt8> >	readUInt8Buffer(UInt64 position, UInt64 byteCount)
												{
													// Read buffer
													TBuffer<UInt8>	buffer(byteCount);
													OV<SError>		error = read(position, *buffer, byteCount);

													return !error.hasValue() ?
															TVResult<TBuffer<UInt8> >(buffer) :
															TVResult<TBuffer<UInt8> >(*error);
												}

											// Class methods
		static	TVResult<CData>				readData(const CFile& file)
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
		UInt64						getByteCount() const;

		OV<SError>					read(UInt64 position, void* buffer, UInt64 byteCount);
		TVResult<CData>				readData(UInt64 position, CData::ByteCount byteCount);
		TVResult<TBuffer<UInt8> >	readUInt8Buffer(UInt64 position, UInt64 byteCount);

	// Properties
	private:
		Internals*	mInternals;
};
