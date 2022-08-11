//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CChunkReader.h"
#include "CMediaSourceRegistry.h"
#include "SWAVEInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaFile

class CWAVEMediaFile {
	// Methods
	public:
											// Lifecycle methods
		virtual								~CWAVEMediaFile() {}

											// Class methods
		static	I<CWAVEMediaFile>			create();
		static	SMediaSource::ImportResult	import(const I<CRandomAccessDataSource>& randomAccessDataSource,
														const OI<CAppleResourceManager>& appleResourceManager,
														SMediaSource::Options options);

	protected:
											// Lifecycle methods
											CWAVEMediaFile() {}

											// Instance methods
		virtual	OI<CChunkReader>			createChunkReader(const I<CRandomAccessDataSource>& randomAccessDataSource);
		virtual	bool						isFormatChunk(const CChunkReader::ChunkInfo& chunkInfo) const
												{ return chunkInfo.getID() == kWAVEFormatChunkID; }
		virtual	bool						isDataChunk(const CChunkReader::ChunkInfo& chunkInfo) const
												{ return chunkInfo.getID() == kWAVEDataChunkID; }
		virtual	SMediaSource::ImportResult	import(CChunkReader& chunkReader,
													const CChunkReader::ChunkInfo& formatChunkInfo,
													const CChunkReader::ChunkInfo& dataChunkInfo,
													const TArray<CChunkReader::ChunkInfo>& otherChunkInfos,
													SMediaSource::Options options) const;

	// Properties
	protected:
		static	CString	mErrorDomain;
		static	SError	mInvalidWAVEFileError;
};
