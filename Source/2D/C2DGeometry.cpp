//----------------------------------------------------------------------------------------------------------------------
//	C2DGeometry.cpp			©2012 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "C2DGeometry.h"

template<>	S2DPoint32				S2DPoint32::mZero = S2DPoint32();
template<>	S2DPoint64				S2DPoint64::mZero = S2DPoint64();

template<>	S2DSize32				S2DSize32::mZero = S2DSize32();
template<>	S2DSize64				S2DSize64::mZero = S2DSize64();

template<>	S2DRect32				S2DRect32::mZero = S2DRect32();
template<>	S2DRect64				S2DRect64::mZero = S2DRect64();

template<>	S2DAffineTransform32	S2DAffineTransform32::mIdentity = S2DAffineTransform32();
template<>	S2DAffineTransform64	S2DAffineTransform64::mIdentity = S2DAffineTransform64();
