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

#ifndef GU_INTERSECTION_RAY_TRIANGLE_H
#define GU_INTERSECTION_RAY_TRIANGLE_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"

namespace physx
{

namespace Gu
{
	PX_PHYSX_COMMON_API bool intersectRayTriangleCulling(	const PxVec3& orig, 
															const PxVec3& dir, 
															const PxVec3& vert0, 
															const PxVec3& vert1, 
															const PxVec3& vert2, 
															PxReal& t,
															PxReal& u, 
															PxReal& v, 
															float enlarge=0.0f);

	PX_PHYSX_COMMON_API bool intersectRayTriangleNoCulling(	const PxVec3& orig, 
															const PxVec3& dir, 
															const PxVec3& vert0, 
															const PxVec3& vert1, 
															const PxVec3& vert2, 
															PxReal& t,
															PxReal& u, 
															PxReal& v, 
															float enlarge=0.0f);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Computes a ray-triangle intersection test.
	*	From Tomas Moeller's "Fast Minimum Storage Ray-Triangle Intersection"
	*	Could be optimized and cut into 2 methods (culled or not). Should make a batch one too to avoid the call overhead, or make it inline.
	*
	*	\param		orig	[in] ray origin
	*	\param		dir		[in] ray direction
	*	\param		vert0	[in] triangle vertex
	*	\param		vert1	[in] triangle vertex
	*	\param		vert2	[in] triangle vertex
	*	\param		t		[out] distance
	*	\param		u		[out] impact barycentric coordinate
	*	\param		v		[out] impact barycentric coordinate
	*	\param		cull	[in] true to use backface culling
	*	\param		enlarge [in] enlarge triangle by specified epsilon in UV space to avoid false near-edge rejections
	*	\return		true on overlap
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PX_FORCE_INLINE bool intersectRayTriangle(const PxVec3& orig, 
								const PxVec3& dir, 
								const PxVec3& vert0, 
								const PxVec3& vert1, 
								const PxVec3& vert2, 
								PxReal& t,
								PxReal& u, 
								PxReal& v, 
								bool cull,
								float enlarge=0.0f)
	{
		return cull ?	intersectRayTriangleCulling(orig, dir, vert0, vert1, vert2, t, u, v, enlarge)
					:	intersectRayTriangleNoCulling(orig, dir, vert0, vert1, vert2, t, u, v, enlarge);
	}

} // namespace Gu

}

#endif
