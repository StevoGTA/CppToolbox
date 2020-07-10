//----------------------------------------------------------------------------------------------------------------------
//	CMatrix.h			©2015 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C3DGeometry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TMatrix2x1

template <typename T> struct TMatrix2x1 {
					// Lifecycle methods
					TMatrix2x1(T m1_1, T m2_1) : m1_1(m1_1), m2_1(m2_1) {}

					// Instance methods
	TMatrix2x1<T>	operator+(const TMatrix2x1<T>& m) const
						{ return TMatrix2x1<T>(m1_1 + m.m1_1, m2_1 + m.m2_1); }

	// Properties
	T	m1_1;
	T	m2_1;
};

typedef TMatrix2x1<Float32>	SMatrix2x1_32;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMatrix2x2

template <typename T> struct TMatrix2x2 {
					// Lifecycle methods
					TMatrix2x2(T m1_1, T m2_1, T m1_2, T m2_2) : m1_1(m1_1), m2_1(m2_1), m1_2(m1_2), m2_2(m2_2) {}

					// Instance methods
	TMatrix2x1<T>	operator*(const TMatrix2x1<T>& m) const
						{ return TMatrix2x1<T>(m1_1 * m.m1_1 + m1_2 * m.m2_1, m2_1 * m.m1_1 + m2_2 * m.m2_1); }

	// Properties
	T	m1_1;
	T	m2_1;
	T	m1_2;
	T	m2_2;
};

typedef TMatrix2x2<Float32>	SMatrix2x2_32;

//----------------------------------------------------------------------------------------------------------------------
// MARK: TMatrix4x1

template <typename T> struct TMatrix4x1 {
	// Lifecycle methods
	TMatrix4x1() : m1_1(0), m2_1(0), m3_1(0), m4_1(0) {}
	TMatrix4x1(T m1_1, T m2_1, T m3_1, T m4_1) : m1_1(m1_1), m2_1(m2_1), m3_1(m3_1), m4_1(m4_1) {}

	// Properties
	T	m1_1;
	T	m2_1;
	T	m3_1;
	T	m4_1;
};

typedef TMatrix4x1<Float32>	SMatrix4x1_32;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMatrix4x4

template <typename T> struct TMatrix4x4 {
					// Lifecycle methods
					TMatrix4x4() :
						m1_1(1), m2_1(0), m3_1(0), m4_1(0),
						m1_2(0), m2_2(1), m3_2(0), m4_2(0),
						m1_3(0), m2_3(0), m3_3(1), m4_3(0),
						m1_4(0), m2_4(0), m3_4(0), m4_4(1)
						{}
					TMatrix4x4(T m1_1, T m2_1, T m3_1, T m4_1, T m1_2, T m2_2, T m3_2, T m4_2, T m1_3, T m2_3, T m3_3,
							T m4_3, T m1_4, T m2_4, T m3_4, T m4_4) :
						m1_1(m1_1), m2_1(m2_1), m3_1(m3_1), m4_1(m4_1), m1_2(m1_2), m2_2(m2_2), m3_2(m3_2), m4_2(m4_2),
								m1_3(m1_3), m2_3(m2_3), m3_3(m3_3), m4_3(m4_3), m1_4(m1_4), m2_4(m2_4), m3_4(m3_4),
								m4_4(m4_4)
						{}

					// Instance methods
	TMatrix4x4<T>	rotateOnX(T radians)
						{
							// Setup
							T	cos = cos(radians);
							T	sin = sin(radians);

							return *this *
									TMatrix4x4<T>(	1.0, 0.0, 0.0, 0.0,
													0.0, cos, sin, 0.0,
													0.0, -sin, cos, 0.0,
													0.0, 0.0, 0.0, 1.0);
						}
	TMatrix4x4<T>	rotateOnY(T radians)
						{
							// Setup
							T	cos = cos(radians);
							T	sin = sin(radians);

							return *this *
									TMatrix4x4<T>(	cos, 0.0, -sin, 0.0,
													0.0, 0.0, 0.0, 0.0,
													sin, 0.0, cos, 0.0,
													0.0, 0.0, 0.0, 1.0);
						}
	TMatrix4x4<T>	rotateOnZ(T radians)
						{
							// Setup
							T	cosv = (T) cos(radians);
							T	sinv = (T) sin(radians);

							return *this *
									TMatrix4x4<T>(	cosv, sinv, 0.0, 0.0,
													-sinv, cosv, 0.0, 0.0,
													0.0, 0.0, 0.0, 0.0,
													0.0, 0.0, 0.0, 1.0);
						}
	TMatrix4x4<T>	scale(T sX, T sY, T sZ) const
						{
							return *this *
									TMatrix4x4<T>(sX, 0.0, 0.0, 0.0, 0.0, sY, 0.0, 0.0, 0.0, 0.0, sZ, 0.0, 0.0, 0.0,
											0.0, 1.0);
						}
	TMatrix4x4<T>	translate(const T3DOffset<T>& offset) const
						{
							return *this *
									TMatrix4x4<T>(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
											offset.mDX, offset.mDY, offset.mDZ, 1.0);
						}
	TMatrix4x4<T>	operator*(const TMatrix4x4<T>& m) const
						{
							return TMatrix4x4<T>(
									m1_1 * m.m1_1 + m1_2 * m.m2_1 + m1_3 * m.m3_1 + m1_4 * m.m4_1,
									m2_1 * m.m1_1 + m2_2 * m.m2_1 + m2_3 * m.m3_1 + m2_4 * m.m4_1,
									m3_1 * m.m1_1 + m3_2 * m.m2_1 + m3_3 * m.m3_1 + m3_4 * m.m4_1,
									m4_1 * m.m1_1 + m4_2 * m.m2_1 + m4_3 * m.m3_1 + m4_4 * m.m4_1,

									m1_1 * m.m1_2 + m1_2 * m.m2_2 + m1_3 * m.m3_2 + m1_4 * m.m4_2,
									m2_1 * m.m1_2 + m2_2 * m.m2_2 + m2_3 * m.m3_2 + m2_4 * m.m4_2,
									m3_1 * m.m1_2 + m3_2 * m.m2_2 + m3_3 * m.m3_2 + m3_4 * m.m4_2,
									m4_1 * m.m1_2 + m4_2 * m.m2_2 + m4_3 * m.m3_2 + m4_4 * m.m4_2,

									m1_1 * m.m1_3 + m1_2 * m.m2_3 + m1_3 * m.m3_3 + m1_4 * m.m4_3,
									m2_1 * m.m1_3 + m2_2 * m.m2_3 + m2_3 * m.m3_3 + m2_4 * m.m4_3,
									m3_1 * m.m1_3 + m3_2 * m.m2_3 + m3_3 * m.m3_3 + m3_4 * m.m4_3,
									m4_1 * m.m1_3 + m4_2 * m.m2_3 + m4_3 * m.m3_3 + m4_4 * m.m4_3,

									m1_1 * m.m1_4 + m1_2 * m.m2_4 + m1_3 * m.m3_4 + m1_4 * m.m4_4,
									m2_1 * m.m1_4 + m2_2 * m.m2_4 + m2_3 * m.m3_4 + m2_4 * m.m4_4,
									m3_1 * m.m1_4 + m3_2 * m.m2_4 + m3_3 * m.m3_4 + m3_4 * m.m4_4,
									m4_1 * m.m1_4 + m4_2 * m.m2_4 + m4_3 * m.m3_4 + m4_4 * m.m4_4);
						}

	// Properties
	T	m1_1;	// 0
	T	m2_1;	// 1
	T	m3_1;	// 2
	T	m4_1;	// 3
	T	m1_2;	// 4
	T	m2_2;	// 5
	T	m3_2;	// 6
	T	m4_2;	// 7
	T	m1_3;	// 8
	T	m2_3;	// 9
	T	m3_3;	// 10
	T	m4_3;	// 11
	T	m1_4;	// 12
	T	m2_4;	// 13
	T	m3_4;	// 14
	T	m4_4;	// 15
};

typedef TMatrix4x4<Float32>	SMatrix4x4_32;
