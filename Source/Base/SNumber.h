//----------------------------------------------------------------------------------------------------------------------
//	SNumber.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Byte swapping

typedef	struct {UInt32 mV;} StoredFloat32;
typedef	struct {UInt64 mV;} StoredFloat64;

static	inline	force_inline	StoredFloat32 EndianF32_NtoB(Float32 arg) {
	union {
		Float32			mValue;
		StoredFloat32	mStoredValue;
	} result;

	result.mValue = arg;
	result.mStoredValue.mV = EndianU32_NtoB(result.mStoredValue.mV);

	return result.mStoredValue;
}

static	inline	force_inline	StoredFloat32 EndianF32_NtoL(Float32 arg) {
	union {
		Float32			mValue;
		StoredFloat32	mStoredValue;
	} result;

	result.mValue = arg;
	result.mStoredValue.mV = EndianU32_NtoL(result.mStoredValue.mV);

	return result.mStoredValue;
}

static	inline	force_inline	Float32 EndianF32_BtoN(StoredFloat32 arg) {
	union {
		Float32			mValue;
		StoredFloat32	mStoredValue;
	} result;

    result.mStoredValue = arg;
	result.mStoredValue.mV = EndianU32_BtoN(result.mStoredValue.mV);

    return result.mValue;
}

static	inline	force_inline	Float32 EndianF32_LtoN(StoredFloat32 arg) {
	union {
		Float32			mValue;
		StoredFloat32	mStoredValue;
	} result;

    result.mStoredValue = arg;
	result.mStoredValue.mV = EndianU32_LtoN(result.mStoredValue.mV);

    return result.mValue;
}

static	inline	force_inline	StoredFloat32 EndianF32_NtoS(Float32 arg) {
	union {
		Float32			mValue;
		StoredFloat32	mStoredValue;
	} result;

	result.mValue = arg;
	result.mStoredValue.mV = EndianU32_NtoB(result.mStoredValue.mV);

	return result.mStoredValue;
}

static	inline	force_inline	Float32 EndianF32_StoN(StoredFloat32 arg) {
	union {
		Float32			mValue;
		StoredFloat32	mStoredValue;
	} result;

    result.mStoredValue = arg;
	result.mStoredValue.mV = EndianU32_NtoB(result.mStoredValue.mV);

    return result.mValue;
}

static	inline	force_inline	StoredFloat64 EndianF64_NtoB(Float64 arg) {
	union {
		Float64			mValue;
		StoredFloat64	mStoredValue;
	} result;

	result.mValue = arg;
	result.mStoredValue.mV = EndianU64_NtoB(result.mStoredValue.mV);

	return result.mStoredValue;
}

static	inline	force_inline	StoredFloat64 EndianF64_NtoL(Float64 arg) {
	union {
		Float64			mValue;
		StoredFloat64	mStoredValue;
	} result;

	result.mValue = arg;
	result.mStoredValue.mV = EndianU64_NtoL(result.mStoredValue.mV);

	return result.mStoredValue;
}

static	inline	force_inline	Float64 EndianF64_BtoN(StoredFloat64 arg) {
	union {
		Float64			mValue;
		StoredFloat64	mStoredValue;
	} result;

    result.mStoredValue = arg;
	result.mStoredValue.mV = EndianU64_BtoN(result.mStoredValue.mV);

    return result.mValue;
}

static	inline	force_inline	Float64 EndianF64_LtoN(StoredFloat64 arg) {
	union {
		Float64			mValue;
		StoredFloat64	mStoredValue;
	} result;

    result.mStoredValue = arg;
	result.mStoredValue.mV = EndianU64_LtoN(result.mStoredValue.mV);

    return result.mValue;
}

static	inline	force_inline	StoredFloat64 EndianF64_NtoS(Float64 arg) {
	union {
		Float64			mValue;
		StoredFloat64	mStoredValue;
	} result;

	result.mValue = arg;
	result.mStoredValue.mV = EndianU64_NtoB(result.mStoredValue.mV);

	return result.mStoredValue;
}

static	inline	force_inline	Float64 EndianF64_StoN(StoredFloat64 arg) {
	union {
		Float64			mValue;
		StoredFloat64	mStoredValue;
	} result;

    result.mStoredValue = arg;
	result.mStoredValue.mV = EndianU64_NtoB(result.mStoredValue.mV);

    return result.mValue;
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
