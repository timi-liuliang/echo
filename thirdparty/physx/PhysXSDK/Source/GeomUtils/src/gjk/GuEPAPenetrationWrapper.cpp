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


#include "GuEPAPenetrationWrapper.h"
#include "GuVecSphere.h"
#include "GuVecCapsule.h"
#include "GuVecBox.h"
#include "GuVecConvexHull.h"
#include "GuVecConvexHullNoScale.h"
#include "GuVecTriangle.h"
#include "GuEPA.h"

namespace physx
{
namespace Gu
{

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//													gjk/epa with contact dist
	//			
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	/*
		convexHull vs others
	*/

	//relative space, warm start
	PX_PHYSX_COMMON_API PxGJKStatus EPALocalPenetration(const CapsuleV& a, const BoxV& b, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		EPASupportMapPairLocalImpl<CapsuleV, BoxV> supportMap(a, b);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);
	}

	PX_PHYSX_COMMON_API PxGJKStatus EPALocalPenetration(const CapsuleV& a, const ConvexHullV& b, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		EPASupportMapPairLocalImpl<CapsuleV, ConvexHullV> supportMap(a, b);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);
	}


	PX_PHYSX_COMMON_API PxGJKStatus EPALocalPenetration(const TriangleV& a, const BoxV& b,  Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		EPASupportMapPairLocalImpl<TriangleV, BoxV> supportMap(a, b);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);
	}

	PX_PHYSX_COMMON_API PxGJKStatus EPALocalPenetration(const TriangleV& a, const ConvexHullV& b, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		EPASupportMapPairLocalImpl<TriangleV, ConvexHullV> supportMap(a, b);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);

	}


	//box vs other
	PX_PHYSX_COMMON_API PxGJKStatus EPARelativePenetration(const BoxV& a, const BoxV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		EPASupportMapPairRelativeImpl<BoxV, BoxV> supportMap(a, b, aToB);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);
	}

	PX_PHYSX_COMMON_API PxGJKStatus EPARelativePenetration(const BoxV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		EPASupportMapPairRelativeImpl<BoxV, ConvexHullV> supportMap(a, b, aToB);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);
	}

	/*
		convexHull vs others
	*/
	PX_PHYSX_COMMON_API PxGJKStatus EPARelativePenetration(const ConvexHullV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{

		EPASupportMapPairRelativeImpl<ConvexHullV, ConvexHullV> supportMap(a, b, aToB);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);

	}  

	PX_PHYSX_COMMON_API PxGJKStatus EPARelativePenetration(const ConvexHullNoScaleV& a, const ConvexHullNoScaleV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{

		EPASupportMapPairRelativeImpl<ConvexHullNoScaleV, ConvexHullNoScaleV> supportMap(a, b, aToB);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);

	}  

	PX_PHYSX_COMMON_API PxGJKStatus EPARelativePenetration(const ConvexHullNoScaleV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{

		EPASupportMapPairRelativeImpl<ConvexHullNoScaleV, ConvexHullV> supportMap(a, b, aToB);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);

	}

	PX_PHYSX_COMMON_API PxGJKStatus EPARelativePenetration(const ConvexHullV& a, const ConvexHullNoScaleV& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{

		EPASupportMapPairRelativeImpl<ConvexHullV, ConvexHullNoScaleV> supportMap(a, b, aToB);
		return epaPenetration(a, b, &supportMap, aIndices, bIndices, _size, contactA, contactB, normal, penetrationDepth, takeCoreShape);

	}  

}
}
