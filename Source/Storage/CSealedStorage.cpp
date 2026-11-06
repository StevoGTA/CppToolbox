//----------------------------------------------------------------------------------------------------------------------
//	CSealedStorage.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSealedStorage.h"

#include "CCoreServices.h"
#include "CFileDataSource.h"
#include "CFileWriter.h"
#include "CLogServices.h"
#include "SHMAC.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

//----------------------------------------------------------------------------------------------------------------------
static CString sComposeSeal(const CString& keyName, const CString& contents, const CData& sealKey)
//----------------------------------------------------------------------------------------------------------------------
{
	return SHMAC::sha256((keyName + CString::mVerticalBar + contents).getUTF8Data(), sealKey).getHexString();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSealedStorage

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CData CSealedStorage::composeSealKey(const CString& secret, const OV<SVersionInfo>& versionInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup - the version, when given, is part of the secret
	CData	secretData = secret.getUTF8Data();
	if (versionInfo.hasValue()) {
		// Add version
		UInt8	versionBytes[] =
						{
							versionInfo->getMajorVersion(),
							versionInfo->getMinorVersion(),
							versionInfo->getPatchVersion(),
						};
		secretData = CData(versionBytes, sizeof(versionBytes)) + secretData;
	}

	return SHMAC::sha256(CCoreServices::getMachineUUIDString().getUTF8Data(), secretData);
}

//----------------------------------------------------------------------------------------------------------------------
CSealedStorage::Value CSealedStorage::read(const CString& keyName, const CCredentialStore& credentialStore,
		const FileInfo& fileInfo, UInt32 componentCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read credential store copy
	OV<CString>		storeString = credentialStore.getString(keyName);
	TArray<CString>	storeComponents =
							storeString.hasValue() ?
									storeString->components(CString::mVerticalBar) : TNArray<CString>();
	bool			storeIsValid = storeComponents.getCount() == componentCount;

	// Read sealed file
	TVResult<CData>	fileData = CFileDataSource::readData(fileInfo.getFile());
	OV<CString>		fileString =
							!fileData.hasError() ?
									OV<CString>(CString(*fileData, CString::kEncodingUTF8)) : OV<CString>();
	TArray<CString>	fileComponents =
							fileString.hasValue() ? fileString->components(CString::mVerticalBar) : TNArray<CString>();
	bool			fileIsValid =
							(fileComponents.getCount() == (componentCount + 1)) &&
									(fileComponents.getLast() ==
											sComposeSeal(keyName,
													fileString->getSubString(0,
															fileString->getLength() -
																	fileComponents.getLast().getLength() - 1),
													fileInfo.getSealKey()));

	// Check if present
	if (!storeString.hasValue() && !fileString.hasValue())
		// Not present
		return Value(OV<TArray<CString> >(), false);

	// Both copies must agree
	if (storeIsValid && fileIsValid) {
		// Compare
		for (CArray::ItemIndex i = 0; i < componentCount; i++) {
			// Check component
			if (storeComponents[i] != fileComponents[i])
				// Differ
				return Value(OV<TArray<CString> >(), true);
		}

		return Value(OV<TArray<CString> >(storeComponents), false);
	} else
		// A copy is missing or has been tampered with
		return Value(OV<TArray<CString> >(), true);
}

//----------------------------------------------------------------------------------------------------------------------
void CSealedStorage::write(const CString& keyName, const TArray<CString>& components, CCredentialStore& credentialStore,
		const FileInfo& fileInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	contents = CString(components, CString::mVerticalBar);

	// Write credential store copy
	OV<SError>	storeError = credentialStore.set(keyName, contents);
	LogIfError(storeError, CString(OSSTR("writing credential store copy")));

	// Write sealed file
	OV<SError>	folderError = fileInfo.getFile().getFolder().create(true);
	LogIfError(folderError, CString(OSSTR("creating sealed file folder")));

	OV<SError>	fileError =
						CFileWriter::write(fileInfo.getFile(),
								(contents + CString::mVerticalBar +
										sComposeSeal(keyName, contents, fileInfo.getSealKey())).getUTF8Data());
	LogIfError(fileError, CString(OSSTR("writing sealed file")));
}

//----------------------------------------------------------------------------------------------------------------------
void CSealedStorage::remove(const CString& keyName, CCredentialStore& credentialStore, const FileInfo& fileInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove credential store copy
	OV<SError>	storeError = credentialStore.remove(keyName);
	LogIfError(storeError, CString(OSSTR("removing credential store copy")));

	// Remove sealed file
	OV<SError>	fileError = fileInfo.getFile().remove();
	LogIfError(fileError, CString(OSSTR("removing sealed file")));
}
