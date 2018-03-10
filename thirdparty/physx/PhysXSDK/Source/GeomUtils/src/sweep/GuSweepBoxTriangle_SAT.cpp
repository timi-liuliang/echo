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

#include "GuSweepBoxTriangle_SAT.h"
#include "GuSweepSharedTests.h"

using namespace physx;
using namespace Gu;

// We have separation if one of those conditions is true:
//     -BoxExt > TriMax (box strictly to the right of the triangle)
//      BoxExt < TriMin (box strictly to the left of the triangle
// <=>  d0 = -BoxExt - TriMax > 0
//      d1 = BoxExt - TriMin < 0
// Hence we have overlap if d0 <= 0 and d1 >= 0
// overlap = (d0<=0.0f && d1>=0.0f)
#ifdef _XBOX
	#define TEST_OVERLAP									\
		const float d0 = -BoxExt - TriMax;					\
		const float d1 = BoxExt - TriMin;					\
		const float cndt0i = physx::intrinsics::fsel(d0, 0.0f, 1.0f);		\
		const float cndt1i = physx::intrinsics::fsel(d1, 1.0f, 0.0f);		\
		const float bIntersect = cndt0i * cndt1i;			\
		bValidMTD *= bIntersect;
#else
	#define TEST_OVERLAP									\
		const float d0 = -BoxExt - TriMax;					\
		const float d1 = BoxExt - TriMin;					\
		const bool bIntersect = (d0<=0.0f && d1>=0.0f);		\
		bValidMTD &= bIntersect;
#endif

// PT: inlining this one is important. Returning floats looks bad but is faster on Xbox.
#ifdef _XBOX
static PX_FORCE_INLINE float testAxis(	const PxTriangle& tri, const PxVec3& extents,
										const PxVec3& dir, const PxVec3& axis,
										float& bValidMTD,
										float& tfirst, float& tlast)
#else
static SPU_INLINE		int testAxis(	const PxTriangle& tri, const PxVec3& extents,
										const PxVec3& dir, const PxVec3& axis,
										bool& bValidMTD, float& tfirst, float& tlast)
#endif
{
	const float d0t = tri.verts[0].dot(axis);
	const float d1t = tri.verts[1].dot(axis);
	const float d2t = tri.verts[2].dot(axis);

	float TriMin = physx::intrinsics::selectMin(d0t, d1t);
	float TriMax = physx::intrinsics::selectMax(d0t, d1t);
	TriMin = physx::intrinsics::selectMin(TriMin, d2t);
	TriMax = physx::intrinsics::selectMax(TriMax, d2t);

	////////

	const float BoxExt = physx::intrinsics::abs(axis.x)*extents.x + physx::intrinsics::abs(axis.y)*extents.y + physx::intrinsics::abs(axis.z)*extents.z;
	TEST_OVERLAP

	const float v = dir.dot(axis);
	if(physx::intrinsics::abs(v) < 1.0E-6f)
#ifdef _XBOX
//		return float(bIntersect);
		return bIntersect;
#else
		return bIntersect;
#endif
	const float oneOverV = -1.0f / v;

//	float t0 = d0 * oneOverV;
//	float t1 = d1 * oneOverV;
//	if(t0 > t1)	TSwap(t0, t1);
	const float t0_ = d0 * oneOverV;
	const float t1_ = d1 * oneOverV;
	float t0 = physx::intrinsics::selectMin(t0_, t1_);
	float t1 = physx::intrinsics::selectMax(t0_, t1_);

#ifdef _XBOX
	const float cndt0 = physx::intrinsics::fsel(tlast - t0, 1.0f, 0.0f);
	const float cndt1 = physx::intrinsics::fsel(t1 - tfirst, 1.0f, 0.0f);
#else
	if(t0 > tlast)	return false;
	if(t1 < tfirst)	return false;
#endif

//	if(t1 < tlast)	tlast = t1;
	tlast = physx::intrinsics::selectMin(t1, tlast);

//	if(t0 > tfirst)	tfirst = t0;
	tfirst = physx::intrinsics::selectMax(t0, tfirst);

#ifdef _XBOX
//	return int(cndt0*cndt1);
	return cndt0*cndt1;
#else
	return true;
#endif
}

#ifdef _XBOX
static PX_FORCE_INLINE float testAxisXYZ(	const PxTriangle& tri, const PxVec3& extents,
											const PxVec3& dir, float oneOverDir,
											float& bValidMTD, float& tfirst, float& tlast, const unsigned int XYZ)
#else
static SPU_INLINE		int testAxisXYZ(	const PxTriangle& tri, const PxVec3& extents,
											const PxVec3& dir, float oneOverDir,
											bool& bValidMTD, float& tfirst, float& tlast, const unsigned int XYZ)
#endif
{
	const float d0t = tri.verts[0][XYZ];
	const float d1t = tri.verts[1][XYZ];
	const float d2t = tri.verts[2][XYZ];

	float TriMin = physx::intrinsics::selectMin(d0t, d1t);
	float TriMax = physx::intrinsics::selectMax(d0t, d1t);
	TriMin = physx::intrinsics::selectMin(TriMin, d2t);
	TriMax = physx::intrinsics::selectMax(TriMax, d2t);

	////////

	const float BoxExt = extents[XYZ];
	TEST_OVERLAP

	const float v = dir[XYZ];
	if(physx::intrinsics::abs(v) < 1.0E-6f)
#ifdef _XBOX
//		return float(bIntersect);
		return bIntersect;
#else
		return bIntersect;
#endif
	const float oneOverV = -1.0f * oneOverDir;

//	float t0 = d0 * oneOverV;
//	float t1 = d1 * oneOverV;
//	if(t0 > t1)	TSwap(t0, t1);
	const float t0_ = d0 * oneOverV;
	const float t1_ = d1 * oneOverV;
	float t0 = physx::intrinsics::selectMin(t0_, t1_);
	float t1 = physx::intrinsics::selectMax(t0_, t1_);

#ifdef _XBOX
	const float cndt0 = physx::intrinsics::fsel(tlast - t0, 1.0f, 0.0f);
	const float cndt1 = physx::intrinsics::fsel(t1 - tfirst, 1.0f, 0.0f);
#else
	if(t0 > tlast)	return false;
	if(t1 < tfirst)	return false;
#endif

//	if(t1 < tlast)	tlast = t1;
	tlast = physx::intrinsics::selectMin(t1, tlast);

//	if(t0 > tfirst)	tfirst = t0;
	tfirst = physx::intrinsics::selectMax(t0, tfirst);

#ifdef _XBOX
	return cndt0*cndt1;
//	return int(cndt0*cndt1);
#else
	return true;
#endif
}

static PX_FORCE_INLINE int testSeparationAxes(	const PxTriangle& tri, const PxVec3& extents,
												const PxVec3& normal, const PxVec3& dir, const PxVec3& oneOverDir, float tmax, float& tcoll)
{
#ifdef _XBOX
	float bValidMTD = 1.0f;
#else
	bool bValidMTD = true;
#endif
	tcoll = tmax;
	float tfirst = -FLT_MAX;
	float tlast  = FLT_MAX;

	// Triangle normal
#ifdef _XBOX
	if(testAxis(tri, extents, dir, normal, bValidMTD, tfirst, tlast)==0.0f)
#else
	if(!testAxis(tri, extents, dir, normal, bValidMTD, tfirst, tlast))
#endif
		return 0;

	// Box normals
#ifdef _XBOX
	if(testAxisXYZ(tri, extents, dir, oneOverDir.x, bValidMTD, tfirst, tlast, 0)==0.0f)
		return 0;
	if(testAxisXYZ(tri, extents, dir, oneOverDir.y, bValidMTD, tfirst, tlast, 1)==0.0f)
		return 0;
	if(testAxisXYZ(tri, extents, dir, oneOverDir.z, bValidMTD, tfirst, tlast, 2)==0.0f)
		return 0;
#else
	if(!testAxisXYZ(tri, extents, dir, oneOverDir.x, bValidMTD, tfirst, tlast, 0))
		return 0;
	if(!testAxisXYZ(tri, extents, dir, oneOverDir.y, bValidMTD, tfirst, tlast, 1))
		return 0;
	if(!testAxisXYZ(tri, extents, dir, oneOverDir.z, bValidMTD, tfirst, tlast, 2))
		return 0;
#endif
	// Edges
	for(PxU32 i=0; i<3; i++)
	{
		int ip1 = int(i+1);
		if(i>=2)	ip1 = 0;
		const PxVec3 TriEdge = tri.verts[ip1] - tri.verts[i];

#ifdef _XBOX
		{
			const PxVec3 Sep = Ps::cross100(TriEdge);
			if((Sep.dot(Sep))>=1.0E-6f && testAxis(tri, extents, dir, Sep, bValidMTD, tfirst, tlast)==0.0f)
				return 0;
		}
		{
			const PxVec3 Sep = Ps::cross010(TriEdge);
			if((Sep.dot(Sep))>=1.0E-6f && testAxis(tri, extents, dir, Sep, bValidMTD, tfirst, tlast)==0.0f)
				return 0;
		}
		{
			const PxVec3 Sep = Ps::cross001(TriEdge);
			if((Sep.dot(Sep))>=1.0E-6f && testAxis(tri, extents, dir, Sep, bValidMTD, tfirst, tlast)==0.0f)
				return 0;
		}
#else
		{
			const PxVec3 Sep = Ps::cross100(TriEdge);
			if((Sep.dot(Sep))>=1.0E-6f && !testAxis(tri, extents, dir, Sep, bValidMTD, tfirst, tlast))
				return 0;
		}
		{
			const PxVec3 Sep = Ps::cross010(TriEdge);
			if((Sep.dot(Sep))>=1.0E-6f && !testAxis(tri, extents, dir, Sep, bValidMTD, tfirst, tlast))
				return 0;
		}
		{
			const PxVec3 Sep = Ps::cross001(TriEdge);
			if((Sep.dot(Sep))>=1.0E-6f && !testAxis(tri, extents, dir, Sep, bValidMTD, tfirst, tlast))
				return 0;
		}
#endif
	}

	if(tfirst > tmax || tlast < 0.0f)	return 0;
	if(tfirst <= 0.0f)
	{
#ifdef _XBOX
		if(bValidMTD==0.0f)	return 0;
#else
		if(!bValidMTD)	return 0;
#endif
		tcoll = 0.0f;
	}
	else tcoll = tfirst;

	return 1;
}

// PT: SAT-based version, in box space
// AP: uninlining on SPU doesn't help
int Gu::triBoxSweepTestBoxSpace(const PxTriangle& tri, const PxVec3& extents, const PxVec3& dir, const PxVec3& oneOverDir, float tmax, float& toi, bool doBackfaceCulling)
{
	// Create triangle normal
	PxVec3 triNormal;
	tri.denormalizedNormal(triNormal);

	// Backface culling
	if(doBackfaceCulling && (triNormal.dot(dir)) >= 0.0f)	// ">=" is important !
		return 0;

	// The SAT test will properly detect initial overlaps, no need for extra tests
	return testSeparationAxes(tri, extents, triNormal, dir, oneOverDir, tmax, toi);
}
