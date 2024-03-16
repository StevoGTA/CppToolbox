//----------------------------------------------------------------------------------------------------------------------
//	SNumber.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CHashable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Byte swapping

typedef UInt32	StoredFloat32;
typedef union {
	Float32			mValue;
	StoredFloat32	mStoredValue;
} SwappableFloat32;

static	inline	force_inline	StoredFloat32 EndianF32_NtoB(Float32 value) {
	SwappableFloat32	swappableFloat32;
	swappableFloat32.mValue = value;

	return EndianU32_NtoB(swappableFloat32.mStoredValue);
}

static	inline	force_inline	StoredFloat32 EndianF32_NtoL(Float32 value) {
	SwappableFloat32	swappableFloat32;
	swappableFloat32.mValue = value;

	return EndianU32_NtoL(swappableFloat32.mStoredValue);
}

static	inline	force_inline	Float32 EndianF32_BtoN(StoredFloat32 value) {
	SwappableFloat32	swappableFloat32;
	swappableFloat32.mStoredValue = EndianU32_BtoN(value);

    return swappableFloat32.mValue;
}

static	inline	force_inline	Float32 EndianF32_LtoN(StoredFloat32 value) {
	SwappableFloat32	swappableFloat32;
	swappableFloat32.mStoredValue = EndianU32_LtoN(value);

    return swappableFloat32.mValue;
}


typedef	UInt64	StoredFloat64;
typedef union {
	Float64			mValue;
	StoredFloat64	mStoredValue;
} SwappableFloat64;

static	inline	force_inline	StoredFloat64 EndianF64_NtoB(Float64 value) {
	SwappableFloat64	swappableFloat64;
	swappableFloat64.mValue = value;

	return EndianU64_NtoB(swappableFloat64.mStoredValue);
}

static	inline	force_inline	StoredFloat64 EndianF64_NtoL(Float64 value) {
	SwappableFloat64	swappableFloat64;
	swappableFloat64.mValue = value;

	return EndianU64_NtoL(swappableFloat64.mStoredValue);
}

static	inline	force_inline	Float64 EndianF64_BtoN(StoredFloat64 value) {
	SwappableFloat64	swappableFloat64;
	swappableFloat64.mStoredValue = EndianU64_BtoN(value);

    return swappableFloat64.mValue;
}

static	inline	force_inline	Float64 EndianF64_LtoN(StoredFloat64 value) {
	SwappableFloat64	swappableFloat64;
	swappableFloat64.mStoredValue = EndianU64_LtoN(value);

    return swappableFloat64.mValue;
}

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNumber

template <typename T> class TNumber : public CHashable {
	// Methods
	public:
				// Lifecycle methods
				TNumber(T value) : mValue(value) {}
				TNumber(const TNumber<T>& other) : mValue(other.mValue) {}

				// CEquatable methods
		bool	operator==(const CEquatable& other) const
					{ return mValue == ((const TNumber<T>&) other).mValue; }

				// CHashable methods
		void	hashInto(CHashable::HashCollector& hashableHashCollector) const
					{ hashableHashCollector.add((const UInt8*) &mValue, sizeof(T)); }

				// Instance methods
		T		operator*() const
					{ return mValue; }

	// Properties
	private:
		T	mValue;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SNumber

struct SNumber {
					// Class methods
	static	UInt16	getNextPowerOf2(UInt16 value);

	static	bool	randomBool();

	static	Float32	randomFloat32(Float32 min, Float32 max);
	static	Float32	randomFloat32(Float32 max)
						{ return randomFloat32(0.0, max); }

	static	UInt32	randomUInt32(UInt32 min, UInt32 max);
	static	UInt32	randomUInt32(UInt32 max)
						{ return randomUInt32(0, max); }
};
