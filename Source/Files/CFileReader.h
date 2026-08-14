//----------------------------------------------------------------------------------------------------------------------
//	CFileReader.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileReader

class CFileReader {
	// AppleMetadataInfo
	public:
		struct AppleMetadataInfo : public TV2<FileAppleMetadatas, UInt64> {
			// Methods
			public:
											// Lifecycle methods
											AppleMetadataInfo(const FileAppleMetadatas& appleMetadatas,
													UInt64 resourceByteCount) :
												TV2(appleMetadatas, resourceByteCount)
												{}
											AppleMetadataInfo(const AppleMetadataInfo& other) : TV2(other) {}

											// Instance methods
				const	FileAppleMetadatas&	getAppleMetadatas() const
												{ return getA(); }
						UInt64				getResourceByteCount() const
												{ return getB(); }
		};

	// Methods
	public:
									// Class methods
		static	AppleMetadataInfo	readAppleMetadata(const CFile& file);
};
