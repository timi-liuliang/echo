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

#include "PsFPU.h"
#include "GuIntersectionRayTriangle.h"
#include "PxVec3.h"

#define LOCAL_EPSILON 0.000001f

using namespace physx;

bool Gu::intersectRayTriangleCulling(	const PxVec3& orig, const PxVec3& dir,
										const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2,
										PxReal&	t, PxReal& u, PxReal& v,
										PxF32 enlarge)
{
	//pxPrintf("in intersectRayTriangleCulling\n");
	// Find vectors for two edges sharing vert0
	const PxVec3 edge1 = vert1 - vert0;
	const PxVec3 edge2 = vert2 - vert0;

	// Begin calculating determinant - also used to calculate U parameter
	const PxVec3 pvec = dir.cross(edge2);

	// If determinant is near zero, ray lies in plane of triangle
	const PxReal det = edge1.dot(pvec);

	if(det<LOCAL_EPSILON)
		return false;

	// Calculate distance from vert0 to ray origin
	const PxVec3 tvec = orig - vert0;

	// Calculate U parameter and test bounds
	u = tvec.dot(pvec);
	if(u<-enlarge || u>det+enlarge)
		return false;

	// Prepare to test V parameter
	const PxVec3 qvec = tvec.cross(edge1);

	// Calculate V parameter and test bounds
	v = dir.dot(qvec);
	if(v<-enlarge || u+v>det+enlarge)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	t = edge2.dot(qvec);
	const PxReal inv_det = 1.0f / det;
	t *= inv_det;
	u *= inv_det;
	v *= inv_det;

	return true;
}

bool Gu::intersectRayTriangleNoCulling(	const PxVec3& orig, const PxVec3& dir,
										const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2,
										PxReal& t, PxReal& u, PxReal& v,
										PxF32 enlarge)
{
	//pxPrintf("in intersectRayTriangleNoCulling\n");
	// Find vectors for two edges sharing vert0
	const PxVec3 edge1 = vert1 - vert0;
	const PxVec3 edge2 = vert2 - vert0;

	// Begin calculating determinant - also used to calculate U parameter
	const PxVec3 pvec = dir.cross(edge2); // error ~ |v2-v0|

	// If determinant is near zero, ray lies in plane of triangle
	const PxReal det = edge1.dot(pvec); // error ~ |v2-v0|*|v1-v0|
	//pxPrintf("vert0=%.12f %.12f %.12f\n", vert0.x, vert0.y, vert0.z);
	//pxPrintf("vert1=%.12f %.12f %.12f\n", vert1.x, vert1.y, vert1.z);
	//pxPrintf("vert2=%.12f %.12f %.12f\n", vert2.x, vert2.y, vert2.z);
	//pxPrintf("orig=%.12f %.12f %.12f\n", orig.x, orig.y, orig.z);
	//pxPrintf("dir=%.12f %.12f %.12f\n", dir.x, dir.y, dir.z);
	//pxPrintf("det=%.12f\n", det);
	//pxPrintf("enlarge=%.12f\n", enlarge);

	// the non-culling branch
	if(det>-LOCAL_EPSILON && det<LOCAL_EPSILON)
	{
		//pxPrintf(">>>>>>>>>>>> false due to local epsilon\n");
		return false;
	}
	const PxReal inv_det = 1.0f / det;

	// Calculate distance from vert0 to ray origin
	const PxVec3 tvec = orig - vert0; // error ~ |orig-v0|

	// Calculate U parameter and test bounds
	u = tvec.dot(pvec) * inv_det;
	//pxPrintf("u=%.12f\n", u);
	if(u<-enlarge || u>1.0f+enlarge)
	{
		//pxPrintf(">>>>>>>>>>> false due to uv1\n");
		return false;
	}

	// prepare to test V parameter
	const PxVec3 qvec = tvec.cross(edge1);

	// Calculate V parameter and test bounds
	v = dir.dot(qvec) * inv_det;
	//pxPrintf("v=%.12f\n", v);
	if(v<-enlarge || u+v>1.0f+enlarge)
	{
		//pxPrintf(">>>>>>>>>>> false due to uv2\n");
		return false;
	}

	// Calculate t, ray intersects triangle
	t = edge2.dot(qvec) * inv_det;
	//pxPrintf(">>>>>>>>> true, t=%.12f\n", t);

	return true;
}


