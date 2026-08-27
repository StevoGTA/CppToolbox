//----------------------------------------------------------------------------------------------------------------------
//	CFile.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "CReferenceCountable.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CFile"));

/*
	The AppleDouble container, as specified by RFC 1740 and Apple's AppleSingle/AppleDouble Developer Note.  This is
	what a "._" file holds, what a macOS-created zip puts under "__MACOSX", and what some servers put in an NTFS
	alternate data stream.

		offset  size    field
		0       4       magic - 0x00051607 for AppleDouble
		4       4       version - 0x00020000
		8       16      filler - zeros per spec, but macOS writes "Mac OS X        ", so it is ignored
		24      2       entry count
		26      12 * n  entry descriptors - each is ID, byte offset, byte count

	All fields are big endian.  Everything past the descriptors is payload, located only by the descriptors - so the
	layout varies by producer and nothing may be assumed about ordering, count, or adjacency.  macOS extends the Finder
	Info entry past its 32 bytes with an extended attributes region introduced by an 'ATTR' magic; only the leading 32
	bytes are Finder Info.
*/

#pragma pack(push, 1)

struct SAppleDoubleHeader {
	// Methods
	public:
				// Lifecycle methods
				SAppleDoubleHeader(UInt16 entryCount) :
					mMagic(EndianU32_NtoB(kMagic)), mVersion(EndianU32_NtoB(kVersion)), mFiller{},
							mEntryCount(EndianU16_NtoB(entryCount))
					{}

				// Instance methods
		bool	canDecode() const
					{ return (EndianU32_BtoN(mMagic) == kMagic) && (EndianU32_BtoN(mVersion) == kVersion); }
		UInt16	getEntryCount() const
					{ return EndianU16_BtoN(mEntryCount); }

		CData	getData() const
					{ return CData(this, sizeof(SAppleDoubleHeader)); }

	// Properties (in storage endian)
	private:
		static	const	UInt32	kMagic = 0x00051607;
		static	const	UInt32	kVersion = 0x00020000;

						UInt32	mMagic;
						UInt32	mVersion;
						UInt8	mFiller[16];
						UInt16	mEntryCount;
};

struct SAppleDoubleEntryDescriptor {
	// ID
	public:
		enum ID : UInt32 {
			kIDResourceFork	= 2,
			kIDFinderInfo	= 9,
		};

	// Methods
	public:
				// Lifecycle methods
				SAppleDoubleEntryDescriptor(ID id, UInt32 byteOffset, UInt32 byteCount) :
					mID(EndianU32_NtoB((UInt32) id)), mByteOffset(EndianU32_NtoB(byteOffset)),
							mByteCount(EndianU32_NtoB(byteCount))
					{}

				// Instance methods
		ID		getID() const
					{ return (ID) EndianU32_BtoN(mID); }
		UInt32	getByteOffset() const
					{ return EndianU32_BtoN(mByteOffset); }
		UInt32	getByteCount() const
					{ return EndianU32_BtoN(mByteCount); }

		CData	getData() const
					{ return CData(this, sizeof(SAppleDoubleEntryDescriptor)); }

	// Properties (in storage endian)
	private:
		UInt32	mID;
		UInt32	mByteOffset;
		UInt32	mByteCount;
};

#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs

//----------------------------------------------------------------------------------------------------------------------
static	CFilesystemPath	sGetFilesystemPath(const CFile& file, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	return file.getFilesystemPath();
}

//----------------------------------------------------------------------------------------------------------------------
static	CString	sGetFilesystemPathString(const CFile& file, CFilesystemPath::Style* filesystemPathStyle)
//----------------------------------------------------------------------------------------------------------------------
{
	return file.getFilesystemPath().getString(*filesystemPathStyle);
}

//----------------------------------------------------------------------------------------------------------------------
static bool sDoesFileExist(const CFile& file, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	return file.doesExist();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile::AppleMetadata

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CData CFile::AppleMetadata::toAppleDouble() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt16	entryCount = (mIdentity.hasValue() ? 1 : 0) + (mResourceForkData.hasValue() ? 1 : 0);
	CData	data = SAppleDoubleHeader(entryCount).getData();

	// Add headers
	CData::ByteCount	payloadByteOffset =
								data.getByteCount() +
										(CData::ByteCount) entryCount * sizeof(SAppleDoubleEntryDescriptor);
	if (mIdentity.hasValue()) {
		// Finder Info descriptor
		data +=
				SAppleDoubleEntryDescriptor(SAppleDoubleEntryDescriptor::kIDFinderInfo, (UInt32) payloadByteOffset,
						(UInt32) sizeof(FinderInfo)).getData();
		payloadByteOffset += sizeof(FinderInfo);
	}
	if (mResourceForkData.hasValue()) {
		// Resource Fork descriptor
		data +=
				SAppleDoubleEntryDescriptor(SAppleDoubleEntryDescriptor::kIDResourceFork, (UInt32) payloadByteOffset,
						(UInt32) mResourceForkData->getByteCount()).getData();
		payloadByteOffset += mResourceForkData->getByteCount();
	}

	// Add Payloads
	if (mIdentity.hasValue())
		// Finder Info - type and creator, with the rest of FInfo and all of FXInfo zeroed
		data += FinderInfo(mIdentity->getA(), mIdentity->getB().getValue(0)).getData();
	if (mResourceForkData.hasValue())
		// Resource Fork
		data += *mResourceForkData;

	return data;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<CFile::AppleMetadata> CFile::AppleMetadata::fromAppleDouble(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check header - the magic and version are the whole of what makes these bytes an AppleDouble
	if (data.getByteCount() < sizeof(SAppleDoubleHeader))
		// Too small to be one
		return OV<AppleMetadata>();

	const	SAppleDoubleHeader&	header = *((const SAppleDoubleHeader*) *data.getUInt8Buffer());
	if (!header.canDecode())
		// Not an AppleDouble we know how to read
		return OV<AppleMetadata>();

	// Check the entry descriptors are all there
	UInt16				entryCount = header.getEntryCount();
	CData::ByteCount	descriptorsByteCount = (CData::ByteCount) entryCount * sizeof(SAppleDoubleEntryDescriptor);
	if (data.getByteCount() < (sizeof(SAppleDoubleHeader) + descriptorsByteCount))
		// Entry count overruns the data
		return OV<AppleMetadata>();

	// Collect the entries of interest.  Each is taken independently so that a malformed descriptor for one does not
	//	discard the other - a file carrying only usable Finder Info is still worth reporting.  The first usable one of
	//	each kind wins, so a later malformed duplicate cannot displace what has already been read.
	const	SAppleDoubleEntryDescriptor*	entryDescriptors =
													(const SAppleDoubleEntryDescriptor*)
															(*data.getUInt8Buffer() + sizeof(SAppleDoubleHeader));
			OV<CData>						resourceForkData;
			OV<Identity>					identity;
	for (UInt16 i = 0; i < entryCount; i++) {
		// Setup
		const	SAppleDoubleEntryDescriptor&	entryDescriptor = entryDescriptors[i];
				CData::ByteCount				byteOffset = entryDescriptor.getByteOffset();
				CData::ByteCount				byteCount = entryDescriptor.getByteCount();
		if ((byteOffset + byteCount) > data.getByteCount())
			// Entry overruns the data
			continue;

		// Check entry ID
		switch (entryDescriptor.getID()) {
			case SAppleDoubleEntryDescriptor::kIDResourceFork:
				// Resource fork
				if (resourceForkData.hasValue())
					// Already have one
					break;

				if (byteCount >= sizeof(UInt32) * 4)
					// Store
					resourceForkData.setValue(data.subData(byteOffset, byteCount));
				break;

			case SAppleDoubleEntryDescriptor::kIDFinderInfo:
				// Finder Info
				if (identity.hasValue())
					// Already have one
					break;

				if (byteCount >= sizeof(FinderInfo)) {
					// Read the Finder Info
					const	FinderInfo&	finderInfo = *((const FinderInfo*) *data.getUInt8Buffer(byteOffset, byteCount));
					if (finderInfo.hasIdentity())
						// Store
						identity.setValue(finderInfo.getIdentity());
				}
				break;

			default:
				// A kind we do not read
				break;
		}
	}

	return (resourceForkData.hasValue() || identity.hasValue()) ?
			OV<AppleMetadata>(AppleMetadata(resourceForkData, identity)) : OV<AppleMetadata>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile::Internals

class CFile::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
		Internals(const CFilesystemPath& filesystemPath) :
			TCopyOnWriteReferenceCountable(),
					mFilesystemPath(filesystemPath)
			{}
		Internals(const Internals& other) :
			TCopyOnWriteReferenceCountable(),
					mFilesystemPath(other.mFilesystemPath)
			{}

		CFilesystemPath	mFilesystemPath;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Properties

const	UInt64	CFile::mMaximumResourceForkByteCount = 16 * 1024 * 1024;

const	SError	CFile::mDoesNotExistError(sErrorDomain, 1, CString(OSSTR("Does Not Exist")));
const	SError	CFile::mIsOpenError(sErrorDomain, 2, CString(OSSTR("Is Open")));
const	SError	CFile::mNotOpenError(sErrorDomain, 3, CString(OSSTR("Is Not Open")));
const	SError	CFile::mNotFoundError(sErrorDomain, 4, CString(OSSTR("Is Not Found")));
const	SError	CFile::mUnableToRevealInFinderError(sErrorDomain, 5, CString(OSSTR("Unable to reveal in Finder")));
const	SError	CFile::mUnableToReadError(sErrorDomain, 6, CString(OSSTR("Unable to read")));
const	SError	CFile::mUnableToWriteError(sErrorDomain, 7, CString(OSSTR("Unable to write")));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFile::CFile(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(filesystemPath);
}

//----------------------------------------------------------------------------------------------------------------------
CFile::CFile(const CFile& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFile::~CFile()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CFilesystemPath& CFile::getFilesystemPath() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath;
}

//----------------------------------------------------------------------------------------------------------------------
CString CFile::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->mFilesystemPath.getLastComponent();
}

//----------------------------------------------------------------------------------------------------------------------
CString CFile::getNameDeletingExtension() const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->mFilesystemPath.getLastComponentDeletingExtension();
}

//----------------------------------------------------------------------------------------------------------------------
CString CFile::getNameForDisplay() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath.getLastComponentForDisplay();
}

//----------------------------------------------------------------------------------------------------------------------
CFolder CFile::getFolder() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFolder(mInternals->mFilesystemPath.deletingLastComponent());
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::equals(const CFile& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFilesystemPath == other.mInternals->mFilesystemPath;
}

//----------------------------------------------------------------------------------------------------------------------
CFile& CFile::operator=(const CFile& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if assignment to self
	if (this == &other)
		return *this;

	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
void CFile::update(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Update
	mInternals->mFilesystemPath = filesystemPath;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CFile> CFile::getExistingFiles(const TArray<CFile>& files)
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<CFile>(files, sDoesFileExist);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CFilesystemPath> CFile::getFilesystemPaths(const TArray<CFile>& files)
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<CFilesystemPath>(files, (TNArray<CFilesystemPath>::MapProc) sGetFilesystemPath);
}

//----------------------------------------------------------------------------------------------------------------------
CString CFile::getFilesystemPathsForDisplay(const TArray<CFile>& files, CFilesystemPath::Style filesystemPathStyle)
//----------------------------------------------------------------------------------------------------------------------
{
	return CString(
			TNArray<CString>(files, (TNArray<CString>::MapProc) sGetFilesystemPathString,
					(void*) &filesystemPathStyle));
}
