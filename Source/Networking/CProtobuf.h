//----------------------------------------------------------------------------------------------------------------------
//	CProtobuf.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProtobuf

class CProtobuf {
	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CProtobuf();
								CProtobuf(const CData& data);
								CProtobuf(const CProtobuf& other);
								~CProtobuf();

								// Instance methods
				CProtobuf&		addVarint(UInt32 fieldNumber, UInt64 value);
				CProtobuf&		addBytes(UInt32 fieldNumber, const CData& data);
				CProtobuf&		addString(UInt32 fieldNumber, const CString& string);
				CProtobuf&		addMessage(UInt32 fieldNumber, const CProtobuf& message);

				OV<UInt64>		getVarint(UInt32 fieldNumber) const;
				OV<CData>		getBytes(UInt32 fieldNumber) const;
				OV<CString>		getString(UInt32 fieldNumber) const;
				OV<CProtobuf>	getMessage(UInt32 fieldNumber) const;

		const	CData&			getData() const;

				CProtobuf&		operator=(const CProtobuf& other);

	// Properties
	private:
		Internals*	mInternals;
};
