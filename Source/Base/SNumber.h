//----------------------------------------------------------------------------------------------------------------------
//	SNumber.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

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
// MARK: - SNumberWrapper

template <typename T> struct SNumberWrapper {
	// Lifecycle methods
	SNumberWrapper(T value) : mValue(value) {}
	SNumberWrapper(const SNumberWrapper& other) : mValue(other.mValue) {}

	// Properties
	T	mValue;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SNumber

struct SNumber {
					// Class methods
	static	UInt16	getNextPowerOf2(UInt16 value);
};
