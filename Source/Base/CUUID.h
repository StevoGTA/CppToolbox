//----------------------------------------------------------------------------------------------------------------------
//	CUUID.h			©2009 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Overview
//	UUIDs are Universally Unique 16 byte codes that can be used for numerous purposes.  Typically they are represented
//		as hex strings broken into 5 segments, but can also be represented by a base 64 string that is shorter and can
//		work great for internal stuffs.
// 	Example UUID as hex string: 6C43D03C-D09C-43D1-948F-DBC00F0EDC06
//	Example UUID as base 64 string:

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CUUID

class CUUIDInternals;
class CUUID : public CHashable {
	// Structs
	public:
		struct Bytes {
			// Properties
			UInt8	mBytes[16];
		};

	// Methods
	public:
						// Lifecycle methods
						CUUID();
						CUUID(const Bytes& bytes);
						CUUID(const CData& data);
						CUUID(const CString& string);
						CUUID(const CUUID& other);
						~CUUID();

						// CEquatable methods
				bool	operator==(const CEquatable& other) const
							{ return equals((const CUUID&) other); }

						// CHashable methods
				void	hashInto(CHasher& hasher) const
							{ getBase64String().hashInto(hasher); }

						// Instance methods
				CData	getData() const;
				CString	getHexString() const;
				CString	getBase64String() const
							{ return getData().getBase64String(); }
				Bytes	getBytes();

				bool	equals(const CUUID& other) const;

	// Properties
	public:
		static	CUUID			mZero;

	private:
				CUUIDInternals*	mInternals;
};
