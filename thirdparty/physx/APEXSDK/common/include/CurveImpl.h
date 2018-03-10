/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __CURVE_IMPL_H__
#define __CURVE_IMPL_H__

#include "NxApex.h"
#include "foundation/PxAssert.h"
#include "NxParameterized.h"
#include "PsArray.h"
#include "NxCurve.h"
#include <PsShare.h>

namespace physx
{
namespace apex
{

typedef physx::Array<NxVec2R> Vec2RPointArray;

/**
	The CurveImpl is a class for storing control points on a curve and evaluating the results later.
*/
class CurveImpl : public NxCurve
{
public:
	CurveImpl()
	{}

	~CurveImpl()
	{}

	/**
		Retrieve the output Y for the specified input x, based on the properties of the stored curve described
		by mControlPoints.
	*/
	physx::PxF32 evaluate(physx::PxF32 x) const;

	/**
		Add a control point to the list of control points, returning the index of the new point.
	*/
	physx::PxU32 addControlPoint(const NxVec2R& controlPoint);

	/**
		Add a control points to the list of control points.  Assuming the
		hPoints points to a list of vec2s
	*/
	void addControlPoints(::NxParameterized::Interface* param, ::NxParameterized::Handle& hPoints);

	/**
		Locates the control points that contain x, placing the resulting control points in the two
		out parameters. Returns true if the points were found, false otherwise. If the points were not
		found, the output variables are untouched
	*/
	bool calculateControlPoints(physx::PxF32 x, NxVec2R& outXPoints, NxVec2R& outYPoints) const;

	/**
		Locates the first control point with x larger than xValue or the nimber of control points if such point doesn't exist
	*/
	physx::PxU32 calculateFollowingControlPoint(physx::PxF32 xValue) const;

	///get the array of control points
	const NxVec2R* getControlPoints(physx::PxU32& outCount) const;

private:
	// mControlPoints is a sorted list of control points for a curve. Currently, the curve is a lame
	// lirp'd curve. We could add support for other curvetypes in the future, either bezier curves,
	// splines, etc.
	Vec2RPointArray mControlPoints;
};

}
} // namespace apex

#endif /* __CURVE_IMPL_H__ */
