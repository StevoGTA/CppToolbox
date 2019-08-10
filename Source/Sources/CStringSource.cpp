//----------------------------------------------------------------------------------------------------------------------
//	CStringSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CStringSource.h"

#include "CDataProvider.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CStringSourceInternals

class CStringSourceInternals {
	public:
								CStringSourceInternals(const CDataProvider* dataProvider) :
									mDataProvider(dataProvider), mReferenceCount(1)
									{}
								~CStringSourceInternals()
									{
										DisposeOf(mDataProvider);
									}

		CStringSourceInternals*	addReference()
									{ mReferenceCount++; return this; }
		void					removeReference()
									{
										// Decrement reference count and check if we are the last one
										if (--mReferenceCount == 0) {
											// We going away
											CStringSourceInternals*	THIS = this;
											DisposeOf(THIS);
										}
									}

		const	CDataProvider*	mDataProvider;
				UInt32			mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CStringSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CStringSource::CStringSource(const CDataProvider* dataProvider)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringSourceInternals(dataProvider);
}

//----------------------------------------------------------------------------------------------------------------------
CStringSource::CStringSource(const CStringSource& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CStringSource::~CStringSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CStringSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataProvider->getSize();
}

//----------------------------------------------------------------------------------------------------------------------
CString CStringSource::readStringToEOL(UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	outString;

	bool	foundEnd = false;
	while (!foundEnd) {
		// First, read as much as we can
		UInt64	bytesRead = std::min<UInt64>(1024, getSize() - mInternals->mDataProvider->getPos());
		if (bytesRead == 0) {
			// EOF
			outError = kDataProviderReadBeyondEndError;

			return outString;
		}

		char	buffer[bytesRead + 1];
		outError = mInternals->mDataProvider->readData(buffer, bytesRead);
		if (outError != kNoError)
			// Error
			return CString::mEmpty;

		// Add NULL to end of end
		buffer[bytesRead] = 0;

		// Did we actually read anything?
		if (bytesRead > 0) {
			// Prepare for resetting file position
			SInt32	delta = (SInt32) -bytesRead;

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
			outError = mInternals->mDataProvider->setPos(kDataProviderPositionFromCurrent, delta);
			ReturnValueIfError(outError, outString);

			// Append the chars we found to the return string
			outString += CString(buffer);
		} else
			foundEnd = true;
	}

	return outString;
}

//----------------------------------------------------------------------------------------------------------------------
void CStringSource::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mDataProvider->reset();
}
