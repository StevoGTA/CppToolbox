//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileWriterReportErrorAndReturnError(error, message)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
																							\
					return OV<SError>(error);												\
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

class CFileWriterInternals : public TReferenceCountableAutoDelete<CFileWriterInternals> {
public:
				CFileWriterInternals(const CFile& file) :
					TReferenceCountableAutoDelete(),
							mFile(file), mRemoveIfNotClosed(false)
					{}
				~CFileWriterInternals()
					{}

	OV<SError>	write(const void* buffer, UInt64 byteCount)
					{
						AssertFailUnimplemented();
						return OV<SError>();
					}
	OV<SError>	close()
					{
						AssertFailUnimplemented();
						return OV<SError>();
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
OV<SError> CFileWriter::open(bool append, bool buffered, bool removeIfNotClosed) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mRemoveIfNotClosed = removeIfNotClosed;

	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::write(const void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	OV<SError>	error = mInternals->write(buffer, byteCount);
	if (!error.hasInstance())
		// Error
		CFileWriterReportErrorAndReturnError(*error, "writing");

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileWriter::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return 0;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::setPos(Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::setByteCount(UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::flush() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->close();
}
