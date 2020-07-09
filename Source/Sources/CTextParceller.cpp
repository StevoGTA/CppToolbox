//----------------------------------------------------------------------------------------------------------------------
//	CTextParceller.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTextParceller.h"

#include "CDataSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTextParcellerInternals

class CTextParcellerInternals : public TReferenceCountable<CTextParcellerInternals> {
	public:
		CTextParcellerInternals(const CDataSource* dataSource) : TReferenceCountable(), mDataSource(dataSource) {}
		~CTextParcellerInternals()
			{ Delete(mDataSource); }

		const	CDataSource*	mDataSource;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTextParceller

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTextParceller::CTextParceller(const CDataSource* dataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CTextParcellerInternals(dataSource);
}

//----------------------------------------------------------------------------------------------------------------------
CTextParceller::CTextParceller(const CTextParceller& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CTextParceller::~CTextParceller()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CTextParceller::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataSource->getSize();
}

//----------------------------------------------------------------------------------------------------------------------
CString CTextParceller::readStringToEOL(UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	outString;

	bool	foundEnd = false;
	while (!foundEnd) {
		// First, read as much as we can
		UInt64	bytesRead = std::min<UInt64>(1024, getSize() - mInternals->mDataSource->getPos());
		if (bytesRead == 0) {
			// EOF
			outError = outString.isEmpty() ? kDataProviderReadBeyondEndError : kNoError;

			return outString;
		}

		char	buffer[bytesRead + 1];
		outError = mInternals->mDataSource->readData(buffer, bytesRead);
		if (outError != kNoError)
			// Error
			return CString::mEmpty;

		// Add NULL to end of end
		buffer[bytesRead] = 0;

		// Did we actually read anything?
		if (bytesRead > 0) {
			// Prepare for resetting file position
			SInt32	delta = -((SInt32) bytesRead);

			// Go through destBuffer, searching for \r and \n
			char*	p = buffer;
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

			// Reset the file's position to the beginning of the next line
			outError = mInternals->mDataSource->setPos(kDataSourcePositionFromCurrent, delta);
			ReturnValueIfError(outError, outString);

			// Append the chars we found to the return string
			outString += CString(buffer);
		} else
			foundEnd = true;
	}

	return outString;
}

//----------------------------------------------------------------------------------------------------------------------
void CTextParceller::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mDataSource->reset();
}
