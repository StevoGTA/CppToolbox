//----------------------------------------------------------------------------------------------------------------------
//	C2DPath.h			©2012 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: C2DPath

//	A 2D Path is made up of multiple subpaths.  Each subpath is a continuous path.  Subpaths are separated by move
//		segments.  A 2D Path begins with a startPoint.  Each additional segment moves to a new point which can be
//		queried as the currentPoint.

class C2DPath {
	// Methods
	public:
						// Lifecycle methods
						C2DPath();
						C2DPath(const C2DPath& other);
		virtual			~C2DPath();

	protected:
						// Instance methods
				void	addMoveTo(const CData& data);
				void	addLineTo(const CData& data);
				void	addQuadCurveTo(const CData& data);
				void	addCubicCurveTo(const CData& data);
				void	addSmallerArcCounterclockwiseTo(const CData& data);
				void	addSmallerArcClockwiseTo(const CData& data);
				void	addLargerArcCounterclockwiseTo(const CData& data);
				void	addLargerArcClockwiseTo(const CData& data);
				void	iterateSegments(bool constructing);
				CData	getInitialSegmentData() const;
		virtual	UInt32	iterateMoveTo(bool constructing, const CData& data) { return 0; }
		virtual	UInt32	iterateLineTo(bool constructing, const CData& data) { return 0; }
		virtual	UInt32	iterateQuadCurveTo(bool constructing, const CData& data) { return 0; }
		virtual	UInt32	iterateCubicCurveTo(bool constructing, const CData& data) { return 0; }
		virtual	UInt32	iterateSmallerArcCounterclockwiseTo(bool constructing, const CData& data) { return 0; }
		virtual	UInt32	iterateSmallerArcClockwiseTo(bool constructing, const CData& data) { return 0; }
		virtual	UInt32	iterateLargerArcCounterclockwiseTo(bool constructing, const CData& data) { return 0; }
		virtual	UInt32	iterateLargerArcClockwiseTo(bool constructing, const CData& data) { return 0; }

	// Properties
	private:
		class	Internals;
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - T2DPath

template <typename T> class T2DPath : public C2DPath {
	// Types
	public:
		typedef	void	(*MoveToProc)(const T2DPoint<T>& point, void* userData);
		typedef	void	(*LineToProc)(const T2DPoint<T>& point, void* userData);
		typedef	void	(*QuadCurveToProc)(const T2DPoint<T>& controlPoint, const T2DPoint<T>& point, void* userData);
		typedef	void	(*CubicCurveToProc)(const T2DPoint<T>& controlPoint1, const T2DPoint<T>& controlPoint2,
								const T2DPoint<T>& point, void* userData);
		typedef	void	(*ArcToProc)(T radiusX, T radiusY, T rotationAngleRadians, bool useLargerArc, bool isClockwise,
								const T2DPoint<T>& point, void* userData);

	// Methods
	public:
										// Lifecycle methods
										T2DPath(const T2DPoint<T>& startPoint) :
											C2DPath(), mCurrentPoint(startPoint)
											{ addMoveTo(startPoint); }
										T2DPath(const T2DPath& path) :
											C2DPath(path), mCurrentPoint(path.mCurrentPoint) {}
										T2DPath(const T2DPath& path, T2DAffineTransform<T> transform) :
											C2DPath(), mCurrentPoint(path.mCurrentPoint.applyTransform(transform))
											{ mConstructingIterateTransform = &transform; iterateSegments(true); }
										~T2DPath() {}

										// Instance methods
						T2DPath&		addMoveTo(const T2DPoint<T>& point)
											{
												// Add
												C2DPath::addMoveTo(CData((const UInt8*) &point, sizeof(point), false));

												// Update current point
												mCurrentPoint = point;

												return *this;
											}
						T2DPath&		addLineTo(const T2DPoint<T>& point)
											{
												// Add
												C2DPath::addLineTo(CData((const UInt8*) &point, sizeof(point), false));

												// Update current point
												mCurrentPoint = point;

												return *this;
											}
						T2DPath&		addQuadCurveTo(const T2DPoint<T>& controlPoint, const T2DPoint<T>& point)
											{
												// Prepare data
												CData	data;
												data.appendBytes((const UInt8*) &controlPoint, sizeof(controlPoint));
												data.appendBytes((const UInt8*) &point, sizeof(point));

												// Add
												C2DPath::addQuadCurveTo(data);

												// Update current point
												mCurrentPoint = point;

												return *this;
											}
						T2DPath&		addCubicCurveTo(const T2DPoint<T>& controlPoint1,
												const T2DPoint<T>& controlPoint2, const T2DPoint<T>& point)
											{
												// Prepare data
												CData	data;
												data.appendBytes((const UInt8*) &controlPoint1, sizeof(controlPoint1));
												data.appendBytes((const UInt8*) &controlPoint2, sizeof(controlPoint2));
												data.appendBytes((const UInt8*) &point, sizeof(point));

												// Add
												C2DPath::addCubicCurveTo(data);

												// Update current point
												mCurrentPoint = point;

												return *this;
											}
						T2DPath&		addArcTo(T radiusX, T radiusY, T rotationAngleRadians, bool useLargerArc,
												bool isClockwise, const T2DPoint<T>& point)
											{
												// Prepare data
												CData	data;
												data.appendBytes((const UInt8*) &radiusX, sizeof(radiusX));
												data.appendBytes((const UInt8*) &radiusY, sizeof(radiusY));
												data.appendBytes((const UInt8*) &rotationAngleRadians,
														sizeof(rotationAngleRadians));
												data.appendBytes((const UInt8*) &point, sizeof(point));

												// Add
												if (!useLargerArc && !isClockwise)
													addSmallerArcCounterclockwiseTo(data);
												else if (!useLargerArc)
													addSmallerArcClockwiseTo(data);
												else if (!isClockwise)
													addLargerArcCounterclockwiseTo(data);
												else
													addLargerArcClockwiseTo(data);

												// Update current point
												mCurrentPoint = point;

												return *this;
											}
						T2DPath&		close()
											{
												// Get info
												CData	data = getInitialSegmentData();

												// Ensure we have the needed bytes
												UInt32	byteCount = sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	void*		bytePtr = data.getBytePtr();
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													if (point != mCurrentPoint)
														// Close
														addLineTo(point);
												}

												return *this;
											}
				const	T2DPoint<T>&	getCurrentPoint() const { return mCurrentPoint; }
						void			iterateSegments(MoveToProc moveToProc, LineToProc lineToProc,
												QuadCurveToProc quadCurveToProc, CubicCurveToProc cubicCurveToProc,
												ArcToProc arcToProc, void* userData)
											{
												// Store
												mIterateMoveToProc = moveToProc;
												mIterateLineToProc = lineToProc;
												mIterateQuadCurveToProc = quadCurveToProc;
												mIterateCubicCurveToProc = cubicCurveToProc;
												mIterateArcToProc = arcToProc;
												mIterateUserData = userData;

												// Iterate segments
												C2DPath::iterateSegments(false);
											}

	protected:
										// Instance methods
		virtual			UInt32			iterateMoveTo(bool constructing, const CData& data)
											{
												// Ensure we have the needed bytes
												UInt32	byteCount = sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	UInt8*		bytePtr = (const UInt8*) data.getBytePtr();
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);

													// What we doing?
													if (constructing)
														// Add
														addMoveTo(point.applyTransform(*mConstructingIterateTransform));
													else
														// Call proc
														mIterateMoveToProc(point, mIterateUserData);
												}

												return byteCount;
											}
		virtual			UInt32			iterateLineTo(bool constructing, const CData& data)
											{
												// Ensure we have the needed bytes
												UInt32	byteCount = sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	UInt8*		bytePtr = (const UInt8*) data.getBytePtr();
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);

													// What we doing?
													if (constructing)
														// Add
														addLineTo(point.applyTransform(*mConstructingIterateTransform));
													else
														// Call proc
														mIterateLineToProc(point, mIterateUserData);
												}

												return byteCount;
											}
		virtual			UInt32			iterateQuadCurveTo(bool constructing, const CData& data)
											{
												// Ensure we have the needed bytes
												UInt32	byteCount = sizeof(T2DPoint<T>) + sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	UInt8*		bytePtr = (const UInt8*) data.getBytePtr();
															T2DPoint<T>	controlPoint = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);

													// What we doing?
													if (constructing)
														// Add
														addQuadCurveTo(
																controlPoint.applyTransform(
																		*mConstructingIterateTransform),
																point.applyTransform(*mConstructingIterateTransform));
													else
														// Call proc
														mIterateQuadCurveToProc(controlPoint, point, mIterateUserData);
												}

												return byteCount;
											}
		virtual			UInt32			iterateCubicCurveTo(bool constructing, const CData& data)
											{
												// Ensure we have the needed bytes
												UInt32	byteCount =
																sizeof(T2DPoint<T>) + sizeof(T2DPoint<T>) +
																		sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	UInt8*		bytePtr = (const UInt8*) data.getBytePtr();
															T2DPoint<T>	controlPoint1 = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);
															T2DPoint<T>	controlPoint2 = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);

													// What we doing?
													if (constructing)
														// Add
														addCubicCurveTo(
																controlPoint1.applyTransform(
																		*mConstructingIterateTransform),
																controlPoint2.applyTransform(
																		*mConstructingIterateTransform),
																point.applyTransform(*mConstructingIterateTransform));
													else
														// Call proc
														mIterateCubicCurveToProc(controlPoint1, controlPoint2, point,
																mIterateUserData);
												}

												return byteCount;
											}
		virtual			UInt32			iterateSmallerArcCounterclockwiseTo(bool constructing, const CData& data)
											{
												// Ensure we have the needed bytes
												UInt32	byteCount =
																sizeof(T) + sizeof(T) + sizeof(T) + sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	UInt8*	bytePtr = (const UInt8*) data.getBytePtr();
															T		radiusX = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T		radiusY = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T		rotationAngleRadians = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);

													// What we doing?
													if (constructing)
														// Add
														addArcTo(
																mConstructingIterateTransform->mA * radiusX +
																		mConstructingIterateTransform->mC * radiusY,
																mConstructingIterateTransform->mB * radiusX +
																		mConstructingIterateTransform->mD * radiusY,
																rotationAngleRadians, false, false,
																point.applyTransform(*mConstructingIterateTransform));
													else
														// Call proc
														mIterateArcToProc(radiusX, radiusY, rotationAngleRadians, false,
																false, point, mIterateUserData);
												}

												return byteCount;
											}
		virtual			UInt32			iterateSmallerArcClockwiseTo(bool constructing, const CData& data)
											{
												// Ensure we have the needed bytes
												UInt32	byteCount =
																sizeof(T) + sizeof(T) + sizeof(T) + sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	UInt8*	bytePtr = (const UInt8*) data.getBytePtr();
															T		radiusX = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T		radiusY = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T		rotationAngleRadians = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);

													// What we doing?
													if (constructing)
														// Add
														addArcTo(
																mConstructingIterateTransform->mA * radiusX +
																		mConstructingIterateTransform->mC * radiusY,
																mConstructingIterateTransform->mB * radiusX +
																		mConstructingIterateTransform->mD * radiusY,
																rotationAngleRadians, false, true,
																point.applyTransform(*mConstructingIterateTransform));
													else
														// Call proc
														mIterateArcToProc(radiusX, radiusY, rotationAngleRadians, false,
																true, point, mIterateUserData);
												}

												return byteCount;
											}
		virtual			UInt32			iterateLargerArcCounterclockwiseTo(bool constructing, const CData& data)
											{
												// Ensure we have the needed bytes
												UInt32	byteCount =
																sizeof(T) + sizeof(T) + sizeof(T) + sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	UInt8*	bytePtr = (const UInt8*) data.getBytePtr();
															T		radiusX = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T		radiusY = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T		rotationAngleRadians = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);

													// What we doing?
													if (constructing)
														// Add
														addArcTo(
																mConstructingIterateTransform->mA * radiusX +
																		mConstructingIterateTransform->mC * radiusY,
																mConstructingIterateTransform->mB * radiusX +
																		mConstructingIterateTransform->mD * radiusY,
																rotationAngleRadians, true, false,
																point.applyTransform(*mConstructingIterateTransform));
													else
														// Call proc
														mIterateArcToProc(radiusX, radiusY, rotationAngleRadians, true,
																false, point, mIterateUserData);
												}

												return byteCount;
											}
		virtual			UInt32			iterateLargerArcClockwiseTo(bool constructing, const CData& data)
											{
												// Ensure we have the needed bytes
												UInt32	byteCount =
																sizeof(T) + sizeof(T) + sizeof(T) + sizeof(T2DPoint<T>);
												if (data.getByteCount() >= byteCount) {
													// Decode
													const	UInt8*		bytePtr = (const UInt8*) data.getBytePtr();
															T			radiusX = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T			radiusY = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T			rotationAngleRadians = *((T*) bytePtr);
													bytePtr += sizeof(T);
															T2DPoint<T>	point = *((T2DPoint<T>*) bytePtr);
													bytePtr += sizeof(T2DPoint<T>);

													// What we doing?
													if (constructing)
														// Add
														addArcTo(
																mConstructingIterateTransform->mA * radiusX +
																		mConstructingIterateTransform->mC * radiusY,
																mConstructingIterateTransform->mB * radiusX +
																		mConstructingIterateTransform->mD * radiusY,
																rotationAngleRadians, true, true,
																point.applyTransform(*mConstructingIterateTransform));
													else
														// Call proc
														mIterateArcToProc(radiusX, radiusY, rotationAngleRadians, true,
																true, point, mIterateUserData);
												}

												return byteCount;
											}

		static			T2DPath			forCircle(const T2DPoint<T>& center, T radius)
											{ return forEllipse(center, radius, radius); }
		static			T2DPath			forEllipse(const T2DPoint<T>& center, T radiusX, T radiusY)
											{
												// Setup
												T			kX = radiusX * (M_SQRT2 - 1.0) * 4.0 / 3.0;
												T			kY = radiusY * (M_SQRT2 - 1.0) * 4.0 / 3.0;

												T2DPoint<T>	top = T2DPoint<T>(center.mX, center.mY - radiusY);
												T2DPoint<T>	bottom = T2DPoint<T>(center.mX, center.mY + radiusY);
												T2DPoint<T>	left = T2DPoint<T>(center.mX - radiusX, center.mY);
												T2DPoint<T>	right = T2DPoint<T>(center.mX + radiusX, center.mY);

												// Compose path
												T2DPath	path;
												path.addMoveTo(top)
													.addCubicCurveTo(T2DPoint<T>(top.mX - kX, top.mY),
															T2DPoint<T>(left.mX, left.mY - kY), left)
													.addCubicCurveTo(T2DPoint<T>(left.mX, left.mY + kX),
															T2DPoint<T>(bottom.mX - kY, bottom.mY), bottom)
													.addCubicCurveTo(T2DPoint<T>(bottom.mX + kX, bottom.mY),
															T2DPoint<T>(right.mX, right.mY + kY), right)
													.addCubicCurveTo(T2DPoint<T>(right.mX, right.mY - kX),
															T2DPoint<T>(top.mX + kY, top.mY), top)
													.close();
												
												return path;
											}
		static			T2DPath			forRect(const T2DRect<T>& rect)
											{
												// Setup
												T	minX = rect.getMinX();
												T	maxX = rect.getMaxX();
												T	minY = rect.getMinY();
												T	maxY = rect.getMaxY();

												// Compose path
												T2DPath	path;
												path.addMoveTo(T2DPoint<T>(minX, minY))
														.addLineTo(T2DPoint<T>(maxX, minY))
														.addLineTo(T2DPoint<T>(maxX, maxY))
														.addLineTo(T2DPoint<T>(minX, maxY))
														.addLineTo(T2DPoint<T>(minX, minY))
														.close();

												return path;
											}
		static			T2DPath			forRoundedRect(const T2DRect<T>& rect, T cornerRadius)
											{
												// From https://web.archive.org/web/20100602144213/http://developer.apple.com/iphone/library/samplecode/QuartzDemo/Listings/Quartz_QuartzCurves_m.html
												// Setup
												T	minX = rect.getMinX();
												T	midX = rect.getMidX();
												T	maxX = rect.getMaxX();
												T	minY = rect.getMinY();
												T	midY = rect.getMidY();
												T	maxY = rect.getMaxY();

												// Compose path
												//       minX    midX    maxX
												// minY    2       3       4
												// midY   1 9              5
												// maxY    8       7       6
												T2DPath	path;
												path.addMoveTo(T2DPoint<T>(minX, midY))
														.addArcTo(cornerRadius, cornerRadius, M_PI / 2.0, false, true,
																T2DPoint<T>(midX, minY))
														.addArcTo(cornerRadius, cornerRadius, M_PI / 2.0, false, true,
																T2DPoint<T>(maxX, midY))
														.addArcTo(cornerRadius, cornerRadius, M_PI / 2.0, false, true,
																T2DPoint<T>(midX, maxY))
														.addArcTo(cornerRadius, cornerRadius, M_PI / 2.0, false, true,
																T2DPoint<T>(minX, midY))
														.close();

												return path;
											}

	// Properties
	private:
		T2DPoint<T>				mCurrentPoint;
		T2DAffineTransform<T>*	mConstructingIterateTransform;

		MoveToProc				mIterateMoveToProc;
		LineToProc				mIterateLineToProc;
		QuadCurveToProc			mIterateQuadCurveToProc;
		CubicCurveToProc		mIterateCubicCurveToProc;
		ArcToProc				mIterateArcToProc;
		void*					mIterateUserData;
};

typedef	T2DPath<Float32>	S2DPath32;
typedef	T2DPath<Float64>	S2DPath64;
