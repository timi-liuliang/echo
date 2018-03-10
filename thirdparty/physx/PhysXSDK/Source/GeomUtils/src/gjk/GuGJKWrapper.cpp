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


#include "GuGJKWrapper.h"
#include "GuVecCapsule.h"
#include "GuVecBox.h"
#include "GuVecConvexHull.h"
#include "GuVecTriangle.h"
#include "GuVecShrunkConvexHull.h"
#include "GuVecShrunkBox.h"
#include "GuGJKRaycast.h"
#include "GuGJK.h"

namespace physx
{
namespace Gu
{

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//													gjk
	//			
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	PxGJKStatus GJKRelative(const CapsuleV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelative(a, b, aToB, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKRelative(const CapsuleV& a, const BoxV& b, const Ps::aos::PsMatTransformV& aToB,  Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelative(a, b, aToB, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKRelative(const BoxV& a,	const BoxV& b, const Ps::aos::PsMatTransformV& aToB,  Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelative(a, b, aToB, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKRelative(const BoxV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelative(a, b, aToB, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKRelative(const ConvexHullV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelative(a, b, aToB, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKRelative(const TriangleV& a, const BoxV& b, const Ps::aos::PsMatTransformV& aToB,  Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelative(a, b, aToB, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKRelative(const TriangleV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB,  Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelative(a, b, aToB, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKLocal(const CapsuleV& a, const ConvexHullV& b, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkLocal(a, b, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKLocal(const TriangleV& a, const BoxV& b, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkLocal(a, b, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKLocal(const TriangleV& a, const ConvexHullV& b,  Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkLocal(a, b, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKRelativeTesselation(const BoxV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg sqTolerance, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelativeTesselation(a, b, aToB, sqTolerance, closestA, closestB, normal, sqDist);
	}

	PxGJKStatus GJKRelativeTesselation(const ConvexHullV& a, const ConvexHullV& b,  const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg sqTolerance, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		return gjkRelativeTesselation(a, b, aToB, sqTolerance, closestA, closestB, normal, sqDist);
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//													gjk raycast 
	//			
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
		triangle vs others
	*/

/*
		triangle vs others
	*/

	bool GJKLocalRayCast(TriangleV& a, CapsuleV& b, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkLocalRayCast<TriangleV, CapsuleV, TriangleV, CapsuleV>(a, b, initialLambda,  s, r, lambda, normal, closestA, inflation, initialOverlap);
	}

	bool GJKLocalRayCast(TriangleV& a, BoxV& b, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkLocalRayCast<TriangleV, BoxV, TriangleV, ShrunkBoxV>(a, b, initialLambda,  s, r, lambda, normal, closestA, inflation, initialOverlap);
	}

	bool GJKLocalRayCast(TriangleV& a, ConvexHullV& b, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkLocalRayCast<TriangleV, ConvexHullV, TriangleV, ShrunkConvexHullV>(a, b, initialLambda,  s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	bool GJKLocalRayCast(CapsuleV& a, BoxV& b, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkLocalRayCast<CapsuleV, BoxV, CapsuleV, ShrunkBoxV>(a, b, initialLambda,  s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	bool GJKLocalRayCast(CapsuleV& a, ConvexHullV& b, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkLocalRayCast<CapsuleV, ConvexHullV, CapsuleV, ShrunkConvexHullV>(a, b, initialLambda,  s, r, lambda, normal, closestA, inflation, initialOverlap);
	}

	bool GJKRelativeRayCast(TriangleV& a, BoxV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA,  const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<TriangleV, BoxV, TriangleV, ShrunkBoxV>(a, b, aToB,initialLambda,  s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	bool GJKRelativeRayCast(TriangleV& a, CapsuleV& b,  const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA,  const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<TriangleV, CapsuleV, TriangleV, CapsuleV>(a, b, aToB, initialLambda, s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	bool GJKRelativeRayCast(TriangleV& a, ConvexHullV& b,  const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA,  const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<TriangleV, ConvexHullV, TriangleV, ShrunkConvexHullV>(a, b, aToB, initialLambda, s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	/*
		capsule vs others
	*/
	bool GJKRelativeRayCast(CapsuleV& a, CapsuleV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA,const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<CapsuleV, CapsuleV, CapsuleV, CapsuleV>(a, b, aToB, initialLambda, s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	bool GJKRelativeRayCast(CapsuleV& a, BoxV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<CapsuleV, BoxV, CapsuleV, ShrunkBoxV>(a, b, aToB, initialLambda, s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	bool GJKRelativeRayCast(CapsuleV& a, ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<CapsuleV, ConvexHullV, CapsuleV, ShrunkConvexHullV>(a, b, aToB, initialLambda, s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	/*
		box vs others
	*/
	bool GJKRelativeRayCast(BoxV& a, BoxV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<BoxV, BoxV, ShrunkBoxV, ShrunkBoxV>(a, b, aToB, initialLambda, s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}
	
	bool GJKRelativeRayCast(BoxV& a, ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<BoxV, ConvexHullV, ShrunkBoxV, ShrunkConvexHullV>(a, b, aToB, initialLambda, s, r, lambda, normal, closestA,  inflation, initialOverlap);
	}

	/*
		convexhull vs others
	*/

	bool GJKRelativeRayCast(ConvexHullV& a, ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA,const PxReal inflation, const bool initialOverlap)
	{
		return gjkRelativeRayCast<ConvexHullV, ConvexHullV, ShrunkConvexHullV, ShrunkConvexHullV>(a, b, aToB, initialLambda, s, r,lambda, normal, closestA,  inflation, initialOverlap);
	}
	
}
}   
