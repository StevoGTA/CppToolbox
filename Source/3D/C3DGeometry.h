//----------------------------------------------------------------------------------------------------------------------
//	C3DGeometry.h			©2017 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: T3DPoint

template <typename T> struct T3DPoint {
						// Lifecycle methods
						T3DPoint() : mX(0.0), mY(0.0), mZ(0.0) {}
						T3DPoint(T x, T y, T z) : mX(x), mY(y), mZ(z) {}
						T3DPoint(const CString& string)
							{
								// "{0.0,0.0,0.0}" => ",0.0,0.0,0.0}" => ["", "0.0", "0.0", "0.0}"]
								TArray<CString>	array =
														string.replacingSubStrings(CString::mBraceOpen, CString::mComma)
																.components(CString::mComma);
								if (array.getCount() >= 4) {
									// Extract values
									mX = array[1].getFloat32();
									mY = array[2].getFloat32();
									mZ = array[3].getFloat32();
								} else {
									// Use default values
									mX = 0.0;
									mY = 0.0;
									mZ = 0.0;
								}
							}

						// Instance methods
	inline	T3DPoint	offset(T offsetX, T offsetY, T offsetZ) const
							{ return T3DPoint(mX + offsetX, mY + offsetY, mZ = offsetX.z); }
	inline	T3DPoint	midpoint(const T3DPoint& point, T ratio) const
							{ return T3DPoint(mX + (point.mX - mX) * ratio, mY + (point.mY - mY) * ratio,
									mZ = (point.mZ - mZ) * ratio); }
	inline	T3DPoint	reflectedBy(const T3DPoint& anchorPoint) const
							{
								return T3DPoint(anchorPoint.mX + (anchorPoint.mX - mX),
										anchorPoint.mY + (anchorPoint.mY - mY), anchorPoint.mZ + (anchorPoint.mZ - mZ));
							}
	inline	T			distanceTo(const T3DPoint& point) const
							{ return sqrt(distanceSquaredTo(point)); }
	inline	T			distanceSquaredTo(const T3DPoint& point) const
							{
								T	dx = point.mX - mX;
								T	dy = point.mY - mY;
								T	dz = point.mZ - mZ;

								return dx * dx + dy * dy + dz * dz;
							}

	inline	bool		operator==(const T3DPoint& other) const
							{ return (mX == other.mX) && (mY == other.mY) && (mZ == other.mZ); }
	inline	bool		operator!=(const T3DPoint& other) const
							{ return (mX != other.mX) || (mY != other.mY) || (mZ != other.mZ); }

	inline	CString		asString() const
							{
								return CString::mBraceOpen + CString(mX, 5, 3) + CString::mComma + CString(mY, 5, 3) +
										CString::mComma + CString(mZ, 5, 3) + CString::mBraceClose;
							}

	// Properties
					T			mX;
					T			mY;
					T			mZ;

	static	const	T3DPoint<T>	mZero;
};

typedef	T3DPoint<Float32>	S3DPointF32;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - T3DOffset

template <typename T> struct T3DOffset {
						// Lifecycle methods
						T3DOffset() : mDX(0.0), mDY(0.0), mDZ(0.0) {}
						T3DOffset(T dx, T dy, T dz) : mDX(dx), mDY(dy), mDZ(dz) {}
						T3DOffset(T3DPoint<T> fromPoint, T3DPoint<T> toPoint) :
								mDX(toPoint.mX - fromPoint.mX), mDY(toPoint.mY - fromPoint.mY),
										mDZ(toPoint.mZ - fromPoint.mZ)
								{}

						// Instance methods
	inline	T3DOffset	inverted() const
							{ return T3DOffset(-mDX, -mDY, -mDZ); }

	// Properties
	T	mDX;
	T	mDY;
	T	mDZ;
};

typedef	T3DOffset<Float32>	S3DOffsetF32;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - T3DVector

template <typename T> struct T3DVector {
							// Lifecycle methods
							T3DVector() : mDX(0.0), mDY(0.0), mDZ(0.0) {}
							T3DVector(T dx, T dy, T dz) : mDX(dx), mDY(dy), mDZ(dz) {}
							T3DVector(const T3DPoint<T>& startPoint, const T3DPoint<T>& endPoint) :
								mDX(endPoint.mX - startPoint.mX), mDY(endPoint.mY - startPoint.mY),
										mDZ(endPoint.mZ - startPoint.mZ) {}

							// Instance methods
	inline	T				magnitude() const
								{ return sqrt(magnitudeSquared()); }
	inline	T				magnitudeSquared() const
								{ return mDX * mDX + mDY * mDY + mDZ * mDZ; }
	inline	T3DVector<T>	normalized() const { return *this / magnitude(); }
	inline	T				dot(const T3DVector& vector) const
								{ return mDX * vector.mDX + mDY * vector.mDY + mDZ * vector.mDZ; }
	inline	T3DVector<T>	crossed(const T3DVector& vector) const
								{
									return T3DVector<T>(
														mDY * vector.mDZ - mDZ * vector.mDY,
														mDZ * vector.mDX - mDX * vector.mDZ,
														mDX * vector.mDY - mDY * vector.mDX
													   );
								}

	inline	T3DPoint<T>		operator+(const T3DPoint<T>& point) const
								{ return T3DPoint<T>(point.mX + mDX, point.mY + mDY, point.mZ + mDZ); }
	inline	T3DVector<T>	operator+(const T3DVector<T>& other) const
								{ return T3DVector<T>(mDX + other.mDX, mDY + other.mDY, mDZ + other.mDZ); }
	inline	T3DVector<T>	operator-(const T3DVector<T>& other) const
								{ return T3DVector<T>(mDX - other.mDX, mDY - other.mDY, mDZ - other.mDZ); }

	inline	T3DVector<T>	operator*(T factor) const { return T3DVector<T>(mDX * factor, mDY * factor, mDZ * factor); }
	inline	T3DVector<T>&	operator*=(T factor) { mDX *= factor; mDY *= factor; mDZ *= factor; return *this; }
	inline	T3DVector<T>	operator/(T factor) const { return T3DVector<T>(mDX / factor, mDY / factor, mDZ / factor); }
	inline	T3DVector<T>&	operator/=(T factor) { mDX /= factor; mDY /= factor; mDZ /= factor; return *this; }

	// Properties
	T	mDX;
	T	mDY;
	T	mDZ;
};

typedef	T3DVector<Float32>	S3DVectorF32;
