//----------------------------------------------------------------------------------------------------------------------
//	CAtom.h			©2025 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAtom

class CAtom : public CData {
	// Header
	public:
		struct Header {
			// Methods
			public:
						// Instance methods
				UInt32	getLength() const
							{ return EndianU32_BtoN(mLength); }
				OSType	getType() const
							{ return EndianU32_BtoN(mType); }

			// Properties (in storage endian)
			private:
				UInt32	mLength;
				OSType	mType;
		};

	// Methods
	public:
				// Lifecycle methods
				CAtom(OSType type);
				CAtom(OSType type, const CData& payload);

				// Instance methods
		CAtom&	operator+=(const CAtom& atom);
};
