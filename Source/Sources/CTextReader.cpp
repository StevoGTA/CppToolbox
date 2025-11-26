//----------------------------------------------------------------------------------------------------------------------
//	CTextReader.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTextReader.h"

#include "CDataSource.h"
#include "CReferenceCountable.h"
#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTextReader::Internals

class CTextReader::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const I<CRandomAccessDataSource>& randomAccessDataSource, CString::Encoding stringEncoding) :
			TReferenceCountableAutoDelete(),
					mRandomAccessDataSource(randomAccessDataSource), mStringEncoding(stringEncoding),
					mDataSourceOffset(0), mByteCount(mRandomAccessDataSource->getByteCount())
			{}

		I<CRandomAccessDataSource>	mRandomAccessDataSource;
		CString::Encoding			mStringEncoding;
		UInt64						mDataSourceOffset;
		UInt64						mByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTextReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTextReader::CTextReader(const I<CRandomAccessDataSource>& randomAccessDataSource, CString::Encoding stringEncoding)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(randomAccessDataSource, stringEncoding);
}

//----------------------------------------------------------------------------------------------------------------------
CTextReader::CTextReader(const CTextReader& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CTextReader::~CTextReader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CTextReader::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CString> CTextReader::readStringToEOL()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	outString;

	bool	foundEnd = false;
	while (!foundEnd) {
		// Figure how many bytes to read
		UInt64	bytesRead = std::min<UInt64>(1024, mInternals->mByteCount - mInternals->mDataSourceOffset);
		if (bytesRead == 0)
			// EOF
			return outString.isEmpty() ? TVResult<CString>(SError::mEndOfData) : TVResult<CString>(outString);

		// Read bytes
		TBuffer<char>	buffer((UInt32) bytesRead + 1);
		OV<SError>		error =
								mInternals->mRandomAccessDataSource->readData(mInternals->mDataSourceOffset, *buffer,
										bytesRead);
		ReturnValueIfError(error, TVResult<CString>(*error));

		// Update
		mInternals->mDataSourceOffset += bytesRead;

		// Prepare for resetting file position
		SInt32	delta = -((SInt32) bytesRead);

		// Go through destBuffer, searching for \r and \n
		char*			p = *buffer;
		CString::Length	stringLength = 0;
		while ((bytesRead > 0) && (*p != '\r') && (*p != '\n')) {
			p++;
			bytesRead--;
			delta++;
			stringLength++;
		}

		// Did we find any end of line chars?
		if (bytesRead > 0) {
			// Yes
			foundEnd = true;

			// End string
			*p = 0;
			p++;
			delta++;

			// Skip the rest of the end of line chars we find
			while ((bytesRead > 0) && ((*p == '\r') || (*p == '\n'))) {
				p++;
				bytesRead--;
				delta++;
			}
		}

		// Seek to beginning of the next line
		mInternals->mDataSourceOffset += delta;

		// Append the chars we found to the return string
		outString += CString(*buffer, stringLength, mInternals->mStringEncoding);
	}

	return TVResult<CString>(outString);
}
