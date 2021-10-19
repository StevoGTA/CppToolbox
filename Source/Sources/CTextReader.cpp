//----------------------------------------------------------------------------------------------------------------------
//	CTextReader.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTextReader.h"

#include "CDataSource.h"
#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTextReaderInternals

class CTextReaderInternals : public TReferenceCountable<CTextReaderInternals> {
	public:
		CTextReaderInternals(const I<CSeekableDataSource>& seekableDataSource) :
			TReferenceCountable(),
					mSeekableDataSource(seekableDataSource), mDataSourceOffset(0),
					mByteCount(mSeekableDataSource->getByteCount())
			{}

		I<CSeekableDataSource>	mSeekableDataSource;
		UInt64					mDataSourceOffset;
		UInt64					mByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTextReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTextReader::CTextReader(const I<CSeekableDataSource>& seekableDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CTextReaderInternals(seekableDataSource);
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
TIResult<CString> CTextReader::readStringToEOL()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	outString;

	bool	foundEnd = false;
	while (!foundEnd) {
		// First, read as much as we can
		UInt64	bytesRead = std::min<UInt64>(1024, mInternals->mByteCount - mInternals->mDataSourceOffset);
		if (bytesRead == 0)
			// EOF
			return outString.isEmpty() ? TIResult<CString>(SError::mEndOfData) : TIResult<CString>(outString);

		// Read
		TBuffer<char>	buffer((UInt32) bytesRead + 1);
		OI<SError>		error =
								mInternals->mSeekableDataSource->readData(mInternals->mDataSourceOffset, *buffer,
										bytesRead);
		ReturnValueIfError(error, TIResult<CString>(*error));

		mInternals->mDataSourceOffset += bytesRead;

		// Did we actually read anything?
		if (bytesRead > 0) {
			// Prepare for resetting file position
			SInt32	delta = -((SInt32) bytesRead);

			// Go through destBuffer, searching for \r and \n
			char*	p = *buffer;
			while ((bytesRead > 0) && (*p != '\r') && (*p != '\n')) {
				p++;
				bytesRead--;
				delta++;
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
			outString += CString(*buffer);
		} else
			foundEnd = true;
	}

	return TIResult<CString>(outString);
}
