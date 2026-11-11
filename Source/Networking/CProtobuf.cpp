//----------------------------------------------------------------------------------------------------------------------
//	CProtobuf.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CProtobuf.h"

#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProtobuf::Internals

class CProtobuf::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
		enum WireType {
			kWireTypeVarint			= 0,
			kWireType64Bit			= 1,
			kWireTypeLengthPrefixed	= 2,
			kWireType32Bit			= 5,
		};

							Internals(const CData& data) : TCopyOnWriteReferenceCountable(), mData(data) {}
							Internals(const Internals& other) : TCopyOnWriteReferenceCountable(), mData(other.mData) {}

				void		appendKey(UInt32 fieldNumber, WireType wireType)
								{ appendVarint(((UInt64) fieldNumber << 3) | wireType); }
				void		appendVarint(UInt64 value)
								{
									// Write 7 bits per byte, low bits first, with the high bit set on all but the last
									while (value >= 0x80) {
										// Write byte
										mData.append((UInt8) ((value & 0x7F) | 0x80));
										value >>= 7;
									}
									mData.append((UInt8) value);
								}

				bool		find(UInt32 fieldNumber, WireType wireType, UInt64& value, CData::ByteIndex& byteIndex)
									const
								{
									// Setup
									TBuffer<const UInt8>	bytes = mData.getUInt8Buffer();
									CData::ByteCount		byteCount = mData.getByteCount();
									CData::ByteIndex		index = 0;
									bool					found = false;

									// Iterate fields
									while (index < byteCount) {
										// Read key
										OV<UInt64>	key = readVarint(bytes, byteCount, index);
										if (!key.hasValue())
											// Malformed
											return false;

										// Read field
										UInt64				fieldValue = 0;
										CData::ByteIndex	fieldByteIndex = index;
										switch (*key & 0x07) {
											case kWireTypeVarint: {
												// Varint
												OV<UInt64>	varint = readVarint(bytes, byteCount, index);
												if (!varint.hasValue())
													// Malformed
													return false;
												fieldValue = *varint;
												} break;

											case kWireType64Bit:
												// 64 bits
												index += 8;
												break;

											case kWireTypeLengthPrefixed: {
												// Length-prefixed
												OV<UInt64>	length = readVarint(bytes, byteCount, index);
												if (!length.hasValue() || (*length > (byteCount - index)))
													// Malformed
													return false;
												fieldValue = *length;
												fieldByteIndex = index;
												index += *length;
												} break;

											case kWireType32Bit:
												// 32 bits
												index += 4;
												break;

											default:
												// Malformed
												return false;
										}

										// Check if match
										if (((*key >> 3) == fieldNumber) && ((*key & 0x07) == wireType)) {
											// Found
											value = fieldValue;
											byteIndex = fieldByteIndex;
											found = true;
										}
									}

									return found;
								}

		static	OV<UInt64>	readVarint(const TBuffer<const UInt8>& bytes, CData::ByteCount byteCount,
									CData::ByteIndex& index)
								{
									// Read 7 bits per byte until a byte without the high bit set
									UInt64	value = 0;
									for (UInt32 shift = 0; (shift < 64) && (index < byteCount); shift += 7) {
										// Read byte
										UInt8	byte = bytes[index++];
										value |= (UInt64) (byte & 0x7F) << shift;
										if ((byte & 0x80) == 0)
											// Done
											return OV<UInt64>(value);
									}

									return OV<UInt64>();
								}

		CData	mData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProtobuf

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CProtobuf::CProtobuf()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(CData((CData::ByteCount) 0, 0));
}

//----------------------------------------------------------------------------------------------------------------------
CProtobuf::CProtobuf(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(data);
}

//----------------------------------------------------------------------------------------------------------------------
CProtobuf::CProtobuf(const CProtobuf& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CProtobuf::~CProtobuf()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CProtobuf& CProtobuf::addVarint(UInt32 fieldNumber, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Append
	mInternals->appendKey(fieldNumber, Internals::kWireTypeVarint);
	mInternals->appendVarint(value);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CProtobuf& CProtobuf::addBytes(UInt32 fieldNumber, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Append
	mInternals->appendKey(fieldNumber, Internals::kWireTypeLengthPrefixed);
	mInternals->appendVarint(data.getByteCount());
	mInternals->mData.append(data);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CProtobuf& CProtobuf::addString(UInt32 fieldNumber, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	return addBytes(fieldNumber, *string.getData(CString::kEncodingUTF8));
}

//----------------------------------------------------------------------------------------------------------------------
CProtobuf& CProtobuf::addMessage(UInt32 fieldNumber, const CProtobuf& message)
//----------------------------------------------------------------------------------------------------------------------
{
	return addBytes(fieldNumber, message.getData());
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt64> CProtobuf::getVarint(UInt32 fieldNumber) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Find
	UInt64				value;
	CData::ByteIndex	byteIndex;

	return mInternals->find(fieldNumber, Internals::kWireTypeVarint, value, byteIndex) ?
			OV<UInt64>(value) : OV<UInt64>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CProtobuf::getBytes(UInt32 fieldNumber) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Find
	UInt64				byteCount;
	CData::ByteIndex	byteIndex;

	return mInternals->find(fieldNumber, Internals::kWireTypeLengthPrefixed, byteCount, byteIndex) ?
			OV<CData>(mInternals->mData.subData(byteIndex, byteCount)) : OV<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CProtobuf::getString(UInt32 fieldNumber) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get bytes
	OV<CData>	data = getBytes(fieldNumber);

	return data.hasValue() ? OV<CString>(CString(*data, CString::kEncodingUTF8)) : OV<CString>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CProtobuf> CProtobuf::getMessage(UInt32 fieldNumber) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get bytes
	OV<CData>	data = getBytes(fieldNumber);

	return data.hasValue() ? OV<CProtobuf>(CProtobuf(*data)) : OV<CProtobuf>();
}

//----------------------------------------------------------------------------------------------------------------------
const CData& CProtobuf::getData() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mData;
}

//----------------------------------------------------------------------------------------------------------------------
CProtobuf& CProtobuf::operator=(const CProtobuf& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if assignment to self
	if (this == &other)
		return *this;

	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
}
