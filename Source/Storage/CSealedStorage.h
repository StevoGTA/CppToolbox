//----------------------------------------------------------------------------------------------------------------------
//	CSealedStorage.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCredentialStore.h"
#include "CFile.h"
#include "SVersionInfo.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSealedStorage

class CSealedStorage {
	// FileINfo
	public:
		struct FileInfo : public TV2<CFile, CData> {
			// Methods
			public:
								// Lifecycle methods
								FileInfo(const CFile& file, const CData& sealKey) : TV2(file, sealKey) {}

								// Instance methods
				const	CFile&	getFile() const
									{ return getA(); }
				const	CData&	getSealKey() const
									{ return getB(); }
		};

	// Value
	public:
		struct Value : public TV2<OV<TArray<CString> >, bool> {
			// Methods
			public:
												// Lifecycle methods
												Value(const OV<TArray<CString> >& components, bool isDamaged) :
													TV2(components, isDamaged)
													{}

												// Instance methods
				const	OV<TArray<CString> >&	getComponents() const
													{ return getA(); }
						bool					isDamaged() const
													{ return getB(); }
		};

	// Methods
	public:
						// Class methods
		static	CData	composeSealKey(const CString& secret, const OV<SVersionInfo>& versionInfo);

		static	Value	read(const CString& keyName, const CCredentialStore& credentialStore, const FileInfo& fileInfo,
								UInt32 componentCount);
		static	void	write(const CString& keyName, const TArray<CString>& components,
								CCredentialStore& credentialStore, const FileInfo& fileInfo);
		static	void	remove(const CString& keyName, CCredentialStore& credentialStore, const FileInfo& fileInfo);
};
