//----------------------------------------------------------------------------------------------------------------------
//	CFileDataProvider.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataProvider.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataProvider

class CFileDataProviderInternals;
class CFileDataProvider : public CDataProvider {
	// Methods
	public:
				// Lifecycle methods
				CFileDataProvider(const CFile& file);
				~CFileDataProvider();

				// CDataProvider methods
		UInt64	getSize() const;

		UError	readData(void* buffer, UInt64 byteCount) const;

		SInt64	getPos() const;
		UError	setPos(EDataProviderPosition position, SInt64 newPos) const;

		void	reset() const;

	// Properties
	private:
		CFileDataProviderInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataProvider

class CMappedFileDataProviderInternals;
class CMappedFileDataProvider : public CDataProvider {
	// Methods
	public:
				// Lifecycle methods
				CMappedFileDataProvider(const CFile& file);
				~CMappedFileDataProvider();

				// CDataProvider methods
		UInt64	getSize() const;

		UError	readData(void* buffer, UInt64 byteCount) const;

		SInt64	getPos() const;
		UError	setPos(EDataProviderPosition position, SInt64 newPos) const;

		void	reset() const;

	// Properties
	private:
		CMappedFileDataProviderInternals*	mInternals;
};
