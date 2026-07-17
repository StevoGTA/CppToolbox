//----------------------------------------------------------------------------------------------------------------------
//	CData-ZIPExtensions.cpp			©2013 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CData-ZIPExtensions.h"

#define ZLIB_CONST
#include <zlib.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CData_ZIPExtensions"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs

//----------------------------------------------------------------------------------------------------------------------
static SError sZIPError(const CString& context, int zLibStatus, const char* msg)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prefer the stream's specific message, fall back to the status text
	const	char*	text = (msg != nil) ? msg : zError(zLibStatus);

	return SError(sErrorDomain, (SInt32) zLibStatus,
			context + CString(OSSTR(": ")) + CString(text, (UInt32) ::strlen(text), CString::kEncodingASCII));
}

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CData_ZIPExtensions

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CData_ZIPExtensions::uncompressDataAsZIP(const CData& data,
		OV<CData::ByteCount> uncompressedDataByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup write buffer
	CData::ByteCount	sourceByteCount = data.getByteCount();
	CData::ByteCount	capacity =
								uncompressedDataByteCount.hasValue() ?
										*uncompressedDataByteCount : sourceByteCount + sourceByteCount / 2;

	CData				decompressedData(capacity);
	TBuffer<UInt8>		outBuffer = decompressedData.getMutableBuffer(capacity);

	// Initialize
	z_stream		strm = {0};
	strm.next_in = *data.getUInt8Buffer();
	strm.avail_in = (uInt) sourceByteCount;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	int	zLibStatus = inflateInit(&strm);
	if (zLibStatus != Z_OK)
		// inflateInit failed
		return TVResult<CData>(sZIPError(CString(OSSTR("inflateInit")), zLibStatus, strm.msg));

	// Inflate
	while (zLibStatus == Z_OK) {
		// Aim at the unused tail of the output buffer
		strm.next_out = (Bytef*) *outBuffer + strm.total_out;
		strm.avail_out = (uInt) (capacity - strm.total_out);

		// Inflate another chunk
		zLibStatus = inflate(&strm, Z_SYNC_FLUSH);

		// Check if out of available space
		if (strm.avail_out == 0) {
			// Grow if the output buffer filled (reallocate preserves existing contents)
			capacity += sourceByteCount / 2;
			outBuffer = decompressedData.getMutableBuffer(capacity);
		}
	}

	// Check for failure
	if (zLibStatus != Z_STREAM_END) {
		// Decompression failed (compose the error before inflateEnd releases the stream)
		SError	error = sZIPError(CString(OSSTR("inflate")), zLibStatus, strm.msg);
		inflateEnd(&strm);

		return TVResult<CData>(error);
	}

	// Finalize
	zLibStatus = inflateEnd(&strm);
	if (zLibStatus != Z_OK)
		// inflateEnd failed
		return TVResult<CData>(sZIPError(CString(OSSTR("inflateEnd")), zLibStatus, strm.msg));

	// Trim to the actual decompressed size
	decompressedData.getMutableBuffer((CData::ByteCount) strm.total_out);

	return TVResult<CData>(decompressedData);
}
