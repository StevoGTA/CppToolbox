//----------------------------------------------------------------------------------------------------------------------
//	CFile.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CFolder.h"
#include "TBuffer.h"
#include "TimeAndDate.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFile

class CFile : public CHashable {
	// AppleMetadata
	public:
		struct AppleMetadata {
			// Identigy
			public:
				struct Identity : public TV2<OSType, OV<OSType> > {
					// Methods
					public:
											// Lifecycle methods
											Identity(OSType fileType, const OV<OSType>& fileCreator) :
												TV2(fileType, fileCreator)
												{}
											Identity(const Identity& other) : TV2(other) {}

											// Instance methods
								OSType		getFileType() const
												{ return getA(); }
						const	OV<OSType>&	getFileCreator() const
												{ return getB(); }
				};

			// Methods
			public:
											// Lifecycle methods
											AppleMetadata(const CData& resourceForkData, OSType fileType,
													OSType fileCreator) :
												mResourceForkData(resourceForkData),
													mIdentity(Identity(fileType, OV<OSType>(fileCreator)))
												{}
											AppleMetadata(const CData& resourceForkData, OSType fileType) :
												mResourceForkData(resourceForkData),
													mIdentity(Identity(fileType, OV<OSType>()))
												{}
											AppleMetadata(const CData& resourceForkData) :
												mResourceForkData(resourceForkData)
												{}
											AppleMetadata(const Identity& identity) : mIdentity(identity) {}
											AppleMetadata(const AppleMetadata& other) :
												mResourceForkData(other.mResourceForkData), mIdentity(other.mIdentity)
												{}

											// Instance methods
				const	OV<CData>&			getResourceForkData() const
												{ return mResourceForkData; }
				const	OV<Identity>&		getIdentity() const
												{ return mIdentity; }

						CData				toAppleDouble() const;

											// Class methods
				static	OV<AppleMetadata>	fromAppleDouble(const CData& data);

			private:
											// Lifecycle methods
											AppleMetadata(const OV<CData>& resourceForkData,
													const OV<Identity>& identity) :
												mResourceForkData(resourceForkData), mIdentity(identity)
												{}

			// Properties
			private:
				OV<CData>		mResourceForkData;
				OV<Identity>	mIdentity;
		};

	// FinderInfo
	public:
#pragma pack(push, 1)
		struct FinderInfo {
			// Methods
			public:
										// Lifecycle methods
										FinderInfo(OSType fileType, OSType fileCreator) :
											mFileType(EndianU32_NtoB(fileType)),
													mFileCreator(EndianU32_NtoB(fileCreator)), mFlags(0), mLocationV(0),
													mLocationH(0), mFolder(0), mIconID(0), mReserved{}, mScript(0),
													mXFlags(0), mCommentID(0), mPutAwayFolderID(0)
											{}
										FinderInfo() :
											mFileType(0), mFileCreator(0), mFlags(0), mLocationV(0), mLocationH(0),
													mFolder(0), mIconID(0), mReserved{}, mScript(0), mXFlags(0),
													mCommentID(0), mPutAwayFolderID(0)
											{}

										// Instance methods
				OSType					getFileType() const
											{ return EndianU32_BtoN(mFileType); }
				void					setFileType(OSType fileType)
											{ mFileType = EndianU32_NtoB(fileType); }
				OSType					getFileCreator() const
											{ return EndianU32_BtoN(mFileCreator); }
				void					setFileCreator(OSType fileCreator)
											{ mFileCreator = EndianU32_NtoB(fileCreator); }

				bool					hasIdentity() const
											{ return mFileType != 0; }
				AppleMetadata::Identity	getIdentity() const
											{ return CFile::AppleMetadata::Identity(getFileType(),
													(mFileCreator != 0) ?
															OV<OSType>(EndianU32_BtoN(mFileCreator)) : OV<OSType>()); }
				CData					getData() const
											{ return CData(this, sizeof(FinderInfo)); }

			// Properties (in storage endian)
			private:
				// FInfo
				OSType	mFileType;
				OSType	mFileCreator;
				UInt16	mFlags;
				SInt16	mLocationV;
				SInt16	mLocationH;
				SInt16	mFolder;

				// FXInfo
				SInt16	mIconID;
				SInt16	mReserved[3];
				SInt8	mScript;
				SInt8	mXFlags;
				SInt16	mCommentID;
				SInt32	mPutAwayFolderID;
		};
#pragma pack(pop)

	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											CFile(const CFilesystemPath& filesystemPath);
											CFile(const CFile& other);
											~CFile();

											// CEquatable methods
						bool				operator==(const CEquatable& other) const
												{ return equals((const CFile&) other); }

											// CHashable methods
						void				hashInto(CHashable::HashCollector& hashableHashCollector) const
												{ getFilesystemPath().hashInto(hashableHashCollector); }

											// Instance methods
				const	CFilesystemPath&	getFilesystemPath() const;

						CString				getName() const;
						CString				getNameDeletingExtension() const;
						CString				getNameForDisplay() const;
						OV<SError>			rename(const CString& name);

						UInt64				getByteCount() const;

						OV<SError>			remove() const;
						bool				doesExist() const;

						CFolder				getFolder() const;
						bool				isHidden() const;

						bool				getLocked() const;
						OV<SError>			setLocked(bool lockFile) const;

						UniversalTime		getCreationUniversalTime() const;
						UniversalTime		getModificationUniversalTime() const;

						bool				equals(const CFile& other) const;

						CFile&				operator=(const CFile& other);

#if defined(TARGET_OS_MACOS) || defined(TARGET_OS_LINUX)
						UInt16				getPermissions() const;
						OV<SError>			setPermissions(UInt16 permissions) const;
#endif

#if defined(TARGET_OS_MACOS)
						bool				isAlias() const;

						OV<CString>			getComments() const;
						OV<SError>			setComments(const CString& string) const;
#endif

											// Class methods
		static			TArray<CFile>		getExistingFiles(const TArray<CFile>& files);

		static			TArray<CString>		getFilesystemPaths(const TArray<CFile>& files,
													CFilesystemPath::Style filesystemPathStyle =
															CFilesystemPath::kStylePlatformDefault);
		static			CString				getFilesystemPathsForDisplay(const TArray<CFile>& files,
													CFilesystemPath::Style filesystemPathStyle =
															CFilesystemPath::kStylePlatformDefault)
												{ return CString(getFilesystemPaths(files, filesystemPathStyle)); }

	private:
											// Instance methods
						void				update(const CFilesystemPath& filesystemPath);

	// Properties
	public:
		static	const	UInt64		mMaximumResourceForkByteCount;

		static	const	SError		mDoesNotExistError;
		static	const	SError		mIsOpenError;
		static	const	SError		mNotOpenError;
		static	const	SError		mNotFoundError;
		static	const	SError		mUnableToRevealInFinderError;
		static	const	SError		mUnableToReadError;
		static	const	SError		mUnableToWriteError;

	private:
						Internals*	mInternals;
};

using FileAppleMetadatas = TArray<CFile::AppleMetadata>;
