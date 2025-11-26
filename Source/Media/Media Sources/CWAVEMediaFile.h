//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CChunkReader.h"
#include "SMediaSource.h"
#include "SWAVEInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaFile

class CWAVEMediaFile {
	// Methods
	public:
												// Lifecycle methods
		virtual									~CWAVEMediaFile() {}

												// Class methods
		static	I<CWAVEMediaFile>				create();
		static	I<SMediaSource::ImportResult>	import(const SMediaSource::ImportSetup& importSetup);

	protected:
												// Lifecycle methods
												CWAVEMediaFile() {}

												// Instance methods
		virtual	OI<CChunkReader>				createChunkReader(
														const I<CRandomAccessDataSource>& randomAccessDataSource);
		virtual	CChunkReader::ChunkInfo			preprocessChunk(const CChunkReader::ChunkInfo& chunkInfo) const
													{ return chunkInfo; }
		virtual	bool							isFormatChunk(const CChunkReader::ChunkInfo& chunkInfo) const
													{ return chunkInfo.getID() == kWAVEFormatChunkID; }
		virtual	bool							isDataChunk(const CChunkReader::ChunkInfo& chunkInfo) const
													{ return chunkInfo.getID() == kWAVEDataChunkID; }
		virtual	I<SMediaSource::ImportResult>	import(CChunkReader& chunkReader,
														const CChunkReader::ChunkInfo& formatChunkInfo,
														UInt64 dataChunkPosition, UInt64 dataChunkByteCount,
														const TArray<CChunkReader::ChunkInfo>& otherChunkInfos,
														UInt32 options) const;

	// Properties
	public:
		static	CString	mErrorDomain;
		static	SError	mInvalidWAVEFileError;
};
