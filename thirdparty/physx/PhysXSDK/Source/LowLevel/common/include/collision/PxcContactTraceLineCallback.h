/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PXC_CONTACTTRACESEGMENTCALLBACK_H
#define PXC_CONTACTTRACESEGMENTCALLBACK_H

#include "CmMatrix34.h"
#include "GuGeometryUnion.h"

#include "GuHeightFieldUtil.h"
#include "CmRenderOutput.h"
#include "GuDebug.h"
#include "GuContactBuffer.h"

namespace physx
{

#define DISTANCE_BASED_TEST

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct PxcContactTraceSegmentCallback
{
	PxVec3				mLine;
	Gu::ContactBuffer&	mContactBuffer;
	Cm::Matrix34		mTransform;
	PxReal				mContactDistance;
	PxU32				mPrevTriangleIndex; // currently only used to indicate first callback
//	Cm::RenderOutput&	mRO;

	PX_INLINE PxcContactTraceSegmentCallback(
		const PxVec3& line, Gu::ContactBuffer& contactBuffer,
		Cm::Matrix34 transform, PxReal contactDistance
		/*, Cm::RenderOutput& ro*/)
	: mLine(line), mContactBuffer(contactBuffer), mTransform(transform),
	mContactDistance(contactDistance), mPrevTriangleIndex(0xFFFFffff)//, mRO(ro)
	{
	}

	bool onEvent(PxU32 , PxU32* )
	{
		return true;
	}

	PX_INLINE bool faceHit(const Gu::HeightFieldUtil& /*hfUtil*/, const PxVec3& /*hitPoint*/, PxU32 /*triangleIndex*/) { return true; }

	// x,z is the point of projected face entry intercept in hf coords, rayHeight is at that same point
	PX_INLINE bool underFaceHit(
		const Gu::HeightFieldUtil& hfUtil, const PxVec3& triangleNormal,
		const PxVec3& crossedEdge, PxF32 x, PxF32 z, PxF32 rayHeight, PxU32 triangleIndex)
	{
		if (mPrevTriangleIndex == 0xFFFFffff) // we only record under-edge contacts so we need at least 2 face hits to have the edge
		{
			mPrevTriangleIndex = triangleIndex;
			//mPrevTriangleNormal = hfUtil.getTriangleNormal(triangleIndex);
			return true;
		}

		const Gu::HeightField& hf = hfUtil.getHeightField();
		PxF32 y = hfUtil.getHeightAtShapePoint(x, z); // TODO: optmization opportunity - this can be derived cheaply inside traceSegment
		PxF32 dy = rayHeight - y;

		if (!hf.isDeltaHeightInsideExtent(dy, mContactDistance))
			return true;

		// add contact
		PxVec3 n = crossedEdge.cross(mLine);
		if (n.y < 0) // Make sure cross product is facing correctly before clipping
			n = -n;

		if (n.y < 0) // degenerate case
			return true;

		const PxReal ll = n.magnitudeSquared();
		if (ll > 0) // normalize
			n *= PxRecipSqrt(ll);
		else // degenerate case
			return true; 

		// Scale delta height so it becomes the "penetration" along the normal
		dy *= n.y;
		if (hf.getThicknessFast() > 0)
		{
			n = -n;
			dy = -dy;
		}

		// compute the contact point
		const PxVec3 point(x, rayHeight, z);
		//mRO << PxVec3(1,0,0) << Gu::Debug::convertToPxMat44(mTransform)
		//	<< Cm::RenderOutput::LINES << point << point + triangleNormal;
#ifdef DISTANCE_BASED_TEST
		mContactBuffer.contact(
			mTransform.transform(point), mTransform.rotate(triangleNormal), dy, PXC_CONTACT_NO_FACE_INDEX, triangleIndex);
#else
		// add gContactDistance to compensate for fact that we don't support dist based contacts in box/convex-hf!
		// See comment at start of those functs.
		mContactBuffer.contact(
			mTransform.transform(point), mTransform.rotate(triangleNormal),
			dy + mContactDistance, PXC_CONTACT_NO_FACE_INDEX, triangleIndex);
#endif
		mPrevTriangleIndex = triangleIndex;
		//mPrevTriangleNormal = triangleNormal;
		return true;
	}

private:
	PxcContactTraceSegmentCallback& operator=(const PxcContactTraceSegmentCallback&);
};

}

#endif
