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
					return OI<SError>(error);												\
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
					{}

	OI<SError>	write(const void* buffer, UInt64 byteCount)
					{
						AssertFailUnimplemented();
						return OI<SError>();
					}
	OI<SError>	close()
					{
						AssertFailUnimplemented();
						return OI<SError>();
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
OI<SError> CFileWriter::open(bool append, bool buffered, bool removeIfNotClosed) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mRemoveIfNotClosed = removeIfNotClosed;

	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::write(const void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	OI<SError>	error = mInternals->write(buffer, byteCount);
	if (!error.hasInstance())
		// Error
		CFileWriterReportErrorAndReturnError(*error, "writing");

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileWriter::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return 0;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::setPos(PositionMode positionMode, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::setSize(UInt64 newSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::flush() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->close();
}
