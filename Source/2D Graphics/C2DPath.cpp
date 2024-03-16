//----------------------------------------------------------------------------------------------------------------------
//	C2DPath.cpp			©2017 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "C2DPath.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: E2DPathSegmentType

enum E2DPathSegmentType {
	k2DPathSegmentTypeMoveTo,
	k2DPathSegmentTypeLineTo,
	k2DPathSegmentTypeQuadCurveTo,
	k2DPathSegmentTypeCubicCurveTo,
	k2DPathSegmentTypeSmallerArcCounterclockwiseTo,
	k2DPathSegmentTypeSmallerArcClockwiseTo,
	k2DPathSegmentTypeLargerArcCounterclockwiseTo,
	k2DPathSegmentTypeLargerArcClockwiseTo,
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - C2DPath::Internals

class C2DPath::Internals {
	public:
				Internals() {}

		void	appendPathSegment(E2DPathSegmentType pathSegmentType, const CData& data)
					{
						// Append
						mPathSegmentData.appendBytes((const UInt8*) &pathSegmentType, sizeof(E2DPathSegmentType));
						mPathSegmentData += data;
					}

		CData	mPathSegmentData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - C2DPath

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
C2DPath::C2DPath()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
C2DPath::C2DPath(const C2DPath& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
	mInternals->mPathSegmentData = other.mInternals->mPathSegmentData;
}

//----------------------------------------------------------------------------------------------------------------------
C2DPath::~C2DPath()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::addMoveTo(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->appendPathSegment(k2DPathSegmentTypeMoveTo, data);
}

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::addLineTo(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->appendPathSegment(k2DPathSegmentTypeLineTo, data);
}

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::addQuadCurveTo(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->appendPathSegment(k2DPathSegmentTypeQuadCurveTo, data);
}

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::addCubicCurveTo(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->appendPathSegment(k2DPathSegmentTypeCubicCurveTo, data);
}

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::addSmallerArcCounterclockwiseTo(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->appendPathSegment(k2DPathSegmentTypeSmallerArcCounterclockwiseTo, data);
}

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::addSmallerArcClockwiseTo(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->appendPathSegment(k2DPathSegmentTypeSmallerArcClockwiseTo, data);
}

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::addLargerArcCounterclockwiseTo(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->appendPathSegment(k2DPathSegmentTypeLargerArcCounterclockwiseTo, data);
}

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::addLargerArcClockwiseTo(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->appendPathSegment(k2DPathSegmentTypeLargerArcClockwiseTo, data);
}

//----------------------------------------------------------------------------------------------------------------------
void C2DPath::iterateSegments(bool constructing)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*				bytePtr = (const UInt8*) mInternals->mPathSegmentData.getBytePtr();
			CData::ByteCount	bytesRemaining = mInternals->mPathSegmentData.getByteCount();
	
	while (bytesRemaining >= (SInt32) sizeof(E2DPathSegmentType)) {
		// Get type
		E2DPathSegmentType	pathSegmentType = *((E2DPathSegmentType*) bytePtr);
		bytePtr += sizeof(E2DPathSegmentType);
		bytesRemaining -= sizeof(E2DPathSegmentType);

		// Handle type
		UInt32	byteCount = 0;
		switch (pathSegmentType) {
			case k2DPathSegmentTypeMoveTo:
				// Move To
				byteCount = iterateMoveTo(constructing, CData(bytePtr, bytesRemaining, false));

				bytePtr += byteCount;
				bytesRemaining -= byteCount;
				break;
			
			case k2DPathSegmentTypeLineTo:
				// Line To
				byteCount = iterateLineTo(constructing, CData(bytePtr, bytesRemaining, false));

				bytePtr += byteCount;
				bytesRemaining -= byteCount;
				break;
			
			case k2DPathSegmentTypeQuadCurveTo:
				// Quad Curve To
				byteCount = iterateQuadCurveTo(constructing, CData(bytePtr, bytesRemaining, false));

				bytePtr += byteCount;
				bytesRemaining -= byteCount;
				break;
			
			case k2DPathSegmentTypeCubicCurveTo:
				// Cubic Curve To
				byteCount = iterateCubicCurveTo(constructing, CData(bytePtr, bytesRemaining, false));

				bytePtr += byteCount;
				bytesRemaining -= byteCount;
				break;
			
			case k2DPathSegmentTypeSmallerArcCounterclockwiseTo:
				// Ssmaller Arc Counterclockwise To
				byteCount = iterateSmallerArcCounterclockwiseTo(constructing, CData(bytePtr, bytesRemaining, false));

				bytePtr += byteCount;
				bytesRemaining -= byteCount;
				break;

			case k2DPathSegmentTypeSmallerArcClockwiseTo:
				// Smaller Arc Clockwise To
				byteCount = iterateSmallerArcClockwiseTo(constructing, CData(bytePtr, bytesRemaining, false));

				bytePtr += byteCount;
				bytesRemaining -= byteCount;
				break;

			case k2DPathSegmentTypeLargerArcCounterclockwiseTo:
				// Larger Arc Counterclockwise To
				byteCount = iterateLargerArcCounterclockwiseTo(constructing, CData(bytePtr, bytesRemaining, false));

				bytePtr += byteCount;
				bytesRemaining -= byteCount;
				break;

			case k2DPathSegmentTypeLargerArcClockwiseTo:
				// Larger Arc Clockwise To
				byteCount = iterateLargerArcClockwiseTo(constructing, CData(bytePtr, bytesRemaining, false));

				bytePtr += byteCount;
				bytesRemaining -= byteCount;
				break;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
CData C2DPath::getInitialSegmentData() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	CData::ByteCount	byteCount = mInternals->mPathSegmentData.getByteCount();
	if (byteCount >= sizeof(E2DPathSegmentType)) {
		// Return the rest of the data
		return CData((const UInt8*) mInternals->mPathSegmentData.getBytePtr() + sizeof(E2DPathSegmentType),
				byteCount - sizeof(E2DPathSegmentType));
	} else
		// No data
		return CData::mEmpty;
}
