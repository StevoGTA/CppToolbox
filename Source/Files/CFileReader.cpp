//----------------------------------------------------------------------------------------------------------------------
//	CFileReader.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileReader.h"

#include "CFileDataSource.h"
#include "CFilesystem.h"

#if defined(TARGET_OS_MACOS)
	#include <sys/xattr.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

#if defined(TARGET_OS_WINDOWS)
#pragma pack(push, 1)

// AFP_AfpInfo is a 60 byte block - see MS-FSCC "Macintosh Supported Stream Names".  Only its signature, which marks it
//	as one, and the Finder Info it carries are wanted; the rest is skipped.
struct SAFPInfo {
	// Methods
	public:
									// Instance methods
		bool						canDecode() const
										{ return EndianU32_BtoN(mSignature) == kSignature; }
		const CFile::FinderInfo&	getFinderInfo() const
										{ return mFinderInfo; }

	// Properties (in storage endian)
	private:
		static	const	UInt32				kSignature = 0x41465000;	// 'AFP\0'

						UInt32				mSignature;
						UInt32				mVersion;
						UInt32				mFileID;
						UInt32				mBackupTime;
						CFile::FinderInfo	mFinderInfo;
						UInt8				mProDOSInfo[6];
						UInt8				mReserved[6];
};

#pragma pack(pop)
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs

//----------------------------------------------------------------------------------------------------------------------
static TArray<CFile> sGetCandidateMACOSXDotUnderscoreFiles(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// A macOS-created zip stores AppleDouble data under a "__MACOSX" folder at the archive root, mirroring the
	//	archive's tree below it - so the companion for "Root/A/B/Name" is "Root/__MACOSX/A/B/._Name", not a sibling.
	//	The archive root is wherever the files were unpacked, which is not knowable from here, so every ancestor is
	//	tried: "__MACOSX" is inserted below it and the tail down to the file mirrored.  A wrong level yields a path
	//	that simply will not exist, so the caller's read discards it - false positives are effectively impossible.
	CFilesystemPath		filesystemPath = file.getFilesystemPath();
	TArray<CString>		components = filesystemPath.getComponents();
	CArray::ItemCount	componentCount = components.getCount();
	if (componentCount < 2)
		// Nothing above the file that could hold a "__MACOSX" folder
		return TNArray<CFile>();

	CString	macOSXComponent(OSSTR("__MACOSX"));
	CString	dotUnderscoreFilename = CString(OSSTR("._")) + components.getLast();

	// Walk from the file's own folder up to the root, inserting "__MACOSX" at each level where that folder exists
	TNArray<CFile>	files;
	CFilesystemPath	ancestorPath = filesystemPath.deletingLastComponent();
	for (CArray::ItemIndex insertAt = (CArray::ItemIndex) (componentCount - 1); insertAt > 0; insertAt--) {
		// Ask whether the "__MACOSX" folder is even there before composing the mirrored tail below it
		CFilesystemPath	macOSXPath = ancestorPath.appendingComponent(macOSXComponent);
		if (CFolder(macOSXPath).doesExist()) {
			// Compose "{ancestor}/__MACOSX/{components between ancestor and file}/._{filename}"
			CFilesystemPath	path = macOSXPath;
			for (CArray::ItemIndex i = insertAt; i < (CArray::ItemIndex) (componentCount - 1); i++)
				// Add component
				path = path.appendingComponent(components[i]);

			// Add file
			files += CFile(path.appendingComponent(dotUnderscoreFilename));
		}

		// Move up one level
		ancestorPath = ancestorPath.deletingLastComponent();
	}

	return files;
}

#if defined(TARGET_OS_WINDOWS)
//----------------------------------------------------------------------------------------------------------------------
static OV<CData> sReadAlternateDataStream(const CFile& file, const CString& streamName)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose the stream path
	CString	path = file.getFilesystemPath().getString() + CString(OSSTR(":")) + streamName;

	// Open.  CreateFile2() accepts a stream path, but GetFileAttributes*() does not - which is why CFile::doesExist()
	//	and CFile::getByteCount() cannot be used to probe for one, and why this opens rather than asks.
	CREATEFILE2_EXTENDED_PARAMETERS	extendedParameters = {0};
	extendedParameters.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
	extendedParameters.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

	HANDLE	fileHandle =
					::CreateFile2(path.getOSString(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING,
							&extendedParameters);
	if (fileHandle == INVALID_HANDLE_VALUE)
		// No such stream.  Overwhelmingly the common case, and not worth reporting as an error.
		return OV<CData>();

	OV<CData>		data;
	LARGE_INTEGER	byteCount;
	if (::GetFileSizeEx(fileHandle, &byteCount) && (byteCount.QuadPart > 0)) {
		// Read a bounded prefix - a fork at the start of a padded stream is still recovered, the tail left unread
		CData::ByteCount	readByteCount =
									(CData::ByteCount) std::min<UInt64>((UInt64) byteCount.QuadPart,
											CFile::mMaximumResourceForkByteCount * 2);
		CData	streamData(readByteCount);
		DWORD	bytesRead = 0;
		if (::ReadFile(fileHandle, *streamData.getMutableBuffer(readByteCount), (DWORD) readByteCount, &bytesRead,
						NULL) &&
				(bytesRead == (DWORD) readByteCount))
			// Success
			data.setValue(streamData);
	}

	// Close
	::CloseHandle(fileHandle);

	return data;
}

#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileReader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CFileReader::AppleMetadataInfo CFileReader::readAppleMetadata(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CFile::AppleMetadata>	appleMetadatas;
	UInt64							maximumReadByteCount = CFile::mMaximumResourceForkByteCount * 2;
	UInt64							resourceByteCount = 0;

#if defined(TARGET_OS_MACOS)
	// The Finder Info here is the live, authoritative identity, so it is taken before any companion that might also
	//	claim one.  It is an extended attribute, not a file of its own, so it does not add to the resource byte total.
	CString::C			path = file.getFilesystemPath().getString().getUTF8String();
	CFile::FinderInfo	finderInfo;
	if ((::getxattr(*path, XATTR_FINDERINFO_NAME, &finderInfo, sizeof(finderInfo), 0, 0) ==
					(ssize_t) sizeof(finderInfo)) &&
			finderInfo.hasIdentity())
		// Identity only
		appleMetadatas += CFile::AppleMetadata(finderInfo.getIdentity());

	// Try to read the resource fork
	CFile	resourceForkFile = CFilesystem::getResourceForkFile(file);
	if (resourceForkFile.doesExist()) {
		// Get info
		CFileDataSource	dataSource(resourceForkFile);
		UInt64			byteCount = dataSource.getByteCount();
		if (byteCount > 0) {
			// Count the full size on disk, read a bounded prefix
			resourceByteCount += byteCount;

			TVResult<CData>	data = dataSource.readData(0, (CData::ByteCount) std::min(byteCount, maximumReadByteCount));
			if (data.hasValue())
				// Add AppleMetadata
				appleMetadatas += CFile::AppleMetadata(*data);
		}
	}
#endif

#if defined(TARGET_OS_WINDOWS)
	// NTFS keeps Macintosh forks in alternate data streams.  The identity is the live, authoritative one, so
	//	AFP_AfpInfo is taken before any companion.
	OV<CData>	afpInfoData = sReadAlternateDataStream(file, CString(OSSTR("AFP_AfpInfo")));
	if (afpInfoData.hasValue() && (afpInfoData->getByteCount() >= sizeof(SAFPInfo))) {
		// The AFP info block is the identity, not a fork - like the macOS Finder Info attribute it does not count
		const	SAFPInfo&	afpInfo = *((const SAFPInfo*) *afpInfoData->getUInt8Buffer());
		if (afpInfo.canDecode() && afpInfo.getFinderInfo().hasIdentity())
			// Identity only
			appleMetadatas += CFile::AppleMetadata(afpInfo.getFinderInfo().getIdentity());
	}

	// AFP_Resource holds either a raw fork or an AppleDouble-wrapped one depending on the producer, so sniff the
	//	AppleDouble magic rather than assume - exactly as the .rsrc sibling does below
	OV<CData>	afpResource = sReadAlternateDataStream(file, CString(OSSTR("AFP_Resource")));
	if (afpResource.hasValue()) {
		// Update resource byte count
		resourceByteCount += afpResource->getByteCount();

		// Add either translated AppleMetadata or raw data
		OV<CFile::AppleMetadata>	appleMetadata = CFile::AppleMetadata::fromAppleDouble(*afpResource);
		appleMetadatas += appleMetadata.hasValue() ? *appleMetadata : CFile::AppleMetadata(*afpResource);
	}
#endif

	// Find and process AppleDouble files
	TNArray<CFile>	appleDoubleFiles;
	appleDoubleFiles += CFilesystem::getDotUnderscoreFile(file);
	appleDoubleFiles += sGetCandidateMACOSXDotUnderscoreFiles(file);
	for (TArray<CFile>::Iterator iterator = appleDoubleFiles.getIterator(); iterator; iterator++) {
		// Check if exists
		if (!iterator->doesExist())
			continue;

		// Setup data source
		CFileDataSource	dataSource(*iterator);
		UInt64			byteCount = dataSource.getByteCount();
		if (byteCount == 0)
			continue;

		// Update resource byte count
		resourceByteCount += byteCount;

		// Read data
		TVResult<CData>	data = dataSource.readData(0, (CData::ByteCount) std::min(byteCount, maximumReadByteCount));
		if (!data.hasValue())
			continue;

		// Convert to CFile::AppleMetadata
		OV<CFile::AppleMetadata>	appleMetadata = CFile::AppleMetadata::fromAppleDouble(*data);
		if (appleMetadata.hasValue())
			// Add
			appleMetadatas += *appleMetadata;
	}

	// Check .rsrc sibling file
	CFile	rsrcFile(file.getFilesystemPath().appendingExtension(CString(OSSTR("rsrc"))));
	if (rsrcFile.doesExist()) {
		// Setup data source
		CFileDataSource	dataSource(rsrcFile);
		UInt64			byteCount = dataSource.getByteCount();
		if (byteCount > 0) {
			// Has bytes
			resourceByteCount += byteCount;

			// Read data
			TVResult<CData>	data = dataSource.readData(0, (CData::ByteCount) std::min(byteCount, maximumReadByteCount));
			if (data.hasValue()) {
				// Try to convert to CFile::AppleMetadata via AppleDouble
				OV<CFile::AppleMetadata>	appleMetadata = CFile::AppleMetadata::fromAppleDouble(*data);

				// Add either translated AppleMetadata or raw data
				appleMetadatas += appleMetadata.hasValue() ? *appleMetadata : CFile::AppleMetadata(*data);
			}
		}
	}

	return AppleMetadataInfo(appleMetadatas, resourceByteCount);
}
