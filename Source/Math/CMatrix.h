//----------------------------------------------------------------------------------------------------------------------
//	CMatrix.h			©2015 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TMatrix2x1

template <typename T> struct TMatrix2x1 {
					// Lifecycle methods
					TMatrix2x1(T m1_1, T m2_1) : m1_1(m1_1), m2_1(m2_1) {}

					// Instance methods
	TMatrix2x1<T>	operator+(TMatrix2x1<T> m) { return TMatrix2x1<T>(m1_1 + m.m1_1, m2_1 + m.m2_1); }

	// Properties
	T	m1_1;
	T	m2_1;
};

typedef TMatrix2x1<Float32>	SMatrix2x1_32;
typedef TMatrix2x1<Float64>	SMatrix2x1_64;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMatrix2x2

template <typename T> struct TMatrix2x2 {
					// Lifecycle methods
					TMatrix2x2(T m1_1, T m2_1, T m1_2, T m2_2) : m1_1(m1_1), m2_1(m2_1), m1_2(m1_2), m2_2(m2_2) {}

					// Instance methods
	TMatrix2x1<T>	operator*(TMatrix2x1<T> m)
						{ return TMatrix2x1<T>(m1_1 * m.m1_1 + m1_2 * m.m2_1, m2_1 * m.m1_1 + m2_2 * m.m2_1); }

	// Properties
	T	m1_1;
	T	m2_1;
	T	m1_2;
	T	m2_2;
};

typedef TMatrix2x2<Float32>	SMatrix2x2_32;
typedef TMatrix2x2<Float64>	SMatrix2x2_64;
