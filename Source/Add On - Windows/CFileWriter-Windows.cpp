//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileWriterReportErrorAndReturnError(error, message)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
																							\
					return error;															\
				}
#define	CFileWriterReportErrorAndReturnValue(error, message, value)							\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
																							\
					return value;															\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileWriterInternals

class CFileWriterInternals : public TReferenceCountable<CFileWriterInternals> {
public:
	CFileWriterInternals(const CFile& file) :
		TReferenceCountable(), mFile(file), mRemoveIfNotClosed(false)
	{}
	~CFileWriterInternals()
	{
	}

	UError	write(const void* buffer, UInt64 byteCount)
	{
		AssertFailUnimplemented();
		return kNoError;
	}
	UError	close()
	{
		AssertFailUnimplemented();
		return kNoError;
	}

	CFile	mFile;
	UInt32	mReferenceCount;

	bool	mRemoveIfNotClosed;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileWriter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileWriter::CFileWriter(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFileWriterInternals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CFileWriter::CFileWriter(const CFileWriter& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFileWriter::~CFileWriter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::open(bool append, bool buffered, bool removeIfNotClosed) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mRemoveIfNotClosed = removeIfNotClosed;

	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::write(const void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	UError	error = mInternals->write(buffer, byteCount);
	if (error == kNoError)
		// Success
		return kNoError;
	else
		// Error
		CFileWriterReportErrorAndReturnError(error, "writing");
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileWriter::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return 0;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::setPos(EFileWriterPositionMode mode, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::setSize(UInt64 newSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::flush() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->close();
}
