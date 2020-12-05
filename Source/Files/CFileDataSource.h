//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSource

class CFileDataSourceInternals;
class CFileDataSource : public CDataSource {
	// Methods
	public:
						// Lifecycle methods
						CFileDataSource(const CFile& file);
						CFileDataSource(const CFileDataSource& other);
						~CFileDataSource();

						// CDataSource methods
		UInt64			getSize() const;

		OI<SError>		readData(void* buffer, UInt64 byteCount);

		SInt64			getPos() const;
		OI<SError>		setPos(Position position, SInt64 newPos);

		CDataSource*	clone() const
							{ return new CFileDataSource(*this); }

		void			reset();

	// Properties
	private:
		CFileDataSourceInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSource

class CMappedFileDataSourceInternals;
class CMappedFileDataSource : public CDataSource {
	// Methods
	public:
						// Lifecycle methods
						CMappedFileDataSource(const CFile& file);
						CMappedFileDataSource(const CMappedFileDataSource& other);
						~CMappedFileDataSource();

						// CDataSource methods
		UInt64			getSize() const;

		OI<SError>		readData(void* buffer, UInt64 byteCount);

		SInt64			getPos() const;
		OI<SError>		setPos(Position position, SInt64 newPos);

		CDataSource*	clone() const
							{ return new CMappedFileDataSource(*this); }

		void			reset();

	// Properties
	private:
		CMappedFileDataSourceInternals*	mInternals;
};
