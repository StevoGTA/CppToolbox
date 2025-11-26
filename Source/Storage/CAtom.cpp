//----------------------------------------------------------------------------------------------------------------------
//	CAtom.cpp			©2025 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAtom.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAtom

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAtom::CAtom(OSType type) : CData(sizeof(UInt32) + sizeof(OSType))
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32*	bytePtr = ((UInt32*) getMutableBytePtr());

	// Set data
	*(bytePtr++) = EndianU32_NtoB((UInt32) getByteCount());
	*bytePtr = EndianU32_NtoB(type);
}

//----------------------------------------------------------------------------------------------------------------------
CAtom::CAtom(OSType type, const CData& payload) : CData(sizeof(UInt32) + sizeof(OSType))
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32*	bytePtr = ((UInt32*) getMutableBytePtr());

	// Set data
	*(bytePtr++) = EndianU32_NtoB((UInt32) (getByteCount() + payload.getByteCount()));
	*bytePtr = EndianU32_NtoB(type);

	// Add payload
	((CData&) *this) += payload;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CAtom& CAtom::operator+=(const CAtom& atom)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add atom
	((CData&) *this) += atom;

	// Update byte count
	UInt32*	bytePtr = ((UInt32*) getMutableBytePtr());
	*bytePtr = EndianU32_NtoB((UInt32) getByteCount());

	return *this;
}
