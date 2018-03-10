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


#include "GuGJKPenetrationWrapper.h"
#include "GuVecSphere.h"
#include "GuVecCapsule.h"
#include "GuVecBox.h"
#include "GuVecShrunkBox.h"
#include "GuVecConvexHull.h"
#include "GuVecShrunkConvexHull.h"
#include "GuVecTriangle.h"
#include "GuGJKRaycast.h"
#include "GuGJKPenetration.h"

namespace physx
{
namespace Gu
{

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//													gjk/epa with contact dist
	//			
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//capsule vs other  

	//b space,  warm start
	PxGJKStatus GJKLocalPenetration(const CapsuleV& a, const BoxV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	PxGJKStatus GJKLocalPenetration(const CapsuleV& a, const ShrunkBoxV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}  

	PxGJKStatus GJKLocalPenetration(const CapsuleV& a, const ConvexHullV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	PxGJKStatus GJKLocalPenetration(const CapsuleV& a, const ShrunkConvexHullV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	PxGJKStatus GJKLocalPenetration(const CapsuleV& a, const ConvexHullNoScaleV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	PxGJKStatus GJKLocalPenetration(const CapsuleV& a, const ShrunkConvexHullNoScaleV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PxGJKStatus GJKLocalPenetration(const TriangleV& a, const BoxV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	PxGJKStatus GJKLocalPenetration(const TriangleV& a, const TriangleV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	PxGJKStatus GJKLocalPenetration(const TriangleV& a, const ConvexHullV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	PxGJKStatus GJKLocalPenetration(const TriangleV& a, const ConvexHullNoScaleV& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		return 	gjkLocalPenetration(a, b, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size, takeCoreShape);
	}

	//box vs other

	//relative space, warm start
	PxGJKStatus GJKRelativePenetration(const BoxV& a, const BoxV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}


	PxGJKStatus GJKRelativePenetration(const BoxV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}

	PxGJKStatus GJKRelativePenetration(const ShrunkBoxV& a, const ShrunkBoxV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}

	PxGJKStatus GJKRelativePenetration(const ShrunkBoxV& a, const ShrunkConvexHullV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}

	PxGJKStatus GJKRelativePenetration(const ShrunkBoxV& a, const ShrunkConvexHullNoScaleV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}


	/*
		convexHull vs others
	*/

	//relative space, warm start

	PxGJKStatus GJKRelativePenetration(const ConvexHullV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}

	PxGJKStatus GJKRelativePenetration(const ShrunkConvexHullV& a, const ShrunkConvexHullV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}

	PxGJKStatus GJKRelativePenetration(const ShrunkConvexHullV& a, const ShrunkConvexHullNoScaleV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}

	PxGJKStatus GJKRelativePenetration(const ShrunkConvexHullNoScaleV& a, const ShrunkConvexHullV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}

	PxGJKStatus GJKRelativePenetration(const ShrunkConvexHullNoScaleV& a, const ShrunkConvexHullNoScaleV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices,PxU8& _size)
	{
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, aIndices, bIndices, _size);
	}

	

	//triangle vs others, no warmstart
	PxGJKStatus GJKRelativePenetration(const TriangleV& a, const BoxV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return gjkRelativePenetration(a, b, aToB, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}

	PxGJKStatus GJKRelativePenetration(const TriangleV& a, const CapsuleV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return gjkRelativePenetration(a, b, aToB, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}

	PxGJKStatus GJKRelativePenetration(const TriangleV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return gjkRelativePenetration(a, b, aToB, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}
	
	//capsule vs other

	PxGJKStatus GJKRelativePenetration(const CapsuleV& a, const CapsuleV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return 	gjkRelativePenetration(a, b, aToB, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}

	PxGJKStatus GJKRelativePenetration(const CapsuleV& a, const BoxV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return 	gjkRelativePenetration(a, b, aToB, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}

	PxGJKStatus GJKRelativePenetration(const CapsuleV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return gjkRelativePenetration(a, b, aToB, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}

	//box vs other
	PxGJKStatus GJKRelativePenetration(const BoxV& a, const BoxV& b, const Ps::aos::PsMatTransformV& aTob, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}

	PxGJKStatus GJKRelativePenetration(const BoxV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aTob,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}


	/*
		convexHull vs others
	*/
	PxGJKStatus GJKRelativePenetration(const ConvexHullV& a, const ConvexHullV& b, const Ps::aos::PsMatTransformV& aTob, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}  

	PxGJKStatus GJKRelativePenetration(const ShrunkConvexHullV& a, const ShrunkConvexHullV& b, const Ps::aos::PsMatTransformV& aTob, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		PxU8 size = 0;
		return 	gjkRelativePenetration(a, b, aTob, contactDist, contactA,contactB, normal, penetrationDepth, NULL, NULL, size);
	}  

}
}
