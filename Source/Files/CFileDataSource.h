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
				~CFileDataSource();

				// CDataSource methods
		UInt64	getSize() const;

		UError	readData(void* buffer, UInt64 byteCount) const;

		SInt64	getPos() const;
		UError	setPos(EDataSourcePosition position, SInt64 newPos) const;

		void	reset() const;

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
				~CMappedFileDataSource();

				// CDataSource methods
		UInt64	getSize() const;

		UError	readData(void* buffer, UInt64 byteCount) const;

		SInt64	getPos() const;
		UError	setPos(EDataSourcePosition position, SInt64 newPos) const;

		void	reset() const;

	// Properties
	private:
		CMappedFileDataSourceInternals*	mInternals;
};
