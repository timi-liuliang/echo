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
#include "PxVec3.h"
#include "GuIntersectionRayBox.h"
#include "GuIntersectionRayBoxSIMD.h"
#include "PxIntrinsics.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Computes a ray-AABB intersection.
*	Original code by Andrew Woo, from "Graphics Gems", Academic Press, 1990
*	Optimized code by Pierre Terdiman, 2000 (~20-30% faster on my Celeron 500)
*	Epsilon value added by Klaus Hartmann. (discarding it saves a few cycles only)
*
*	Hence this version is faster as well as more robust than the original one.
*
*	Should work provided:
*	1) the integer representation of 0.0f is 0x00000000
*	2) the sign bit of the float is the most significant one
*
*	Report bugs: p.terdiman@codercorner.com
*
*	\param		aabb		[in] the axis-aligned bounding box
*	\param		origin		[in] ray origin
*	\param		dir			[in] ray direction
*	\param		coord		[out] impact coordinates
*	\return		true if ray intersects AABB
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RAYAABB_EPSILON 0.00001f
bool Gu::rayAABBIntersect(const PxVec3& minimum, const PxVec3& maximum, const PxVec3& origin, const PxVec3& _dir, PxVec3& coord)
{
	Ps::IntBool Inside = Ps::IntTrue;
	PxVec3 MaxT(-1.0f, -1.0f, -1.0f);
	const PxReal* dir = &_dir.x;
	// Find candidate planes.
	for(PxU32 i=0;i<3;i++)
	{
		if(origin[i] < minimum[i])
		{
			coord[i]	= minimum[i];
			Inside		= Ps::IntFalse;

			// Calculate T distances to candidate planes
			if(PX_IR(dir[i]))	MaxT[i] = (minimum[i] - origin[i]) / dir[i];
		}
		else if(origin[i] > maximum[i])
		{
			coord[i]	= maximum[i];
			Inside		= Ps::IntFalse;

			// Calculate T distances to candidate planes
			if(PX_IR(dir[i]))	MaxT[i] = (maximum[i] - origin[i]) / dir[i];
		}
	}

	// Ray origin inside bounding box
	if(Inside)
	{
		coord = origin;
		return true;
	}

	// Get largest of the maxT's for final choice of intersection
	PxU32 WhichPlane = 0;
	if(MaxT[1] > MaxT[WhichPlane])	WhichPlane = 1;
	if(MaxT[2] > MaxT[WhichPlane])	WhichPlane = 2;

	// Check final candidate actually inside box
	if(PX_IR(MaxT[WhichPlane])&0x80000000) return false;

	for(PxU32 i=0;i<3;i++)
	{
		if(i!=WhichPlane)
		{
			coord[i] = origin[i] + MaxT[WhichPlane] * dir[i];
#ifdef RAYAABB_EPSILON
			if(coord[i] < minimum[i] - RAYAABB_EPSILON || coord[i] > maximum[i] + RAYAABB_EPSILON)	return false;
#else
			if(coord[i] < minimum[i] || coord[i] > maximum[i])	return false;
#endif
		}
	}
	return true;	// ray hits box
}



/**
*	Computes a ray-AABB intersection.
*	Original code by Andrew Woo, from "Graphics Gems", Academic Press, 1990
*	Optimized code by Pierre Terdiman, 2000 (~20-30% faster on my Celeron 500)
*	Epsilon value added by Klaus Hartmann. (discarding it saves a few cycles only)
*  Return of intersected face code and parameter by Adam!  Also modified behavior for ray starts inside AABB. 2004 :-p
*
*	Hence this version is faster as well as more robust than the original one.
*
*	Should work provided:
*	1) the integer representation of 0.0f is 0x00000000
*	2) the sign bit of the float is the most significant one
*
*	Report bugs: p.terdiman@codercorner.com
*
*	\param		aabb		[in] the axis-aligned bounding box
*	\param		origin		[in] ray origin
*	\param		dir			[in] ray direction
*	\param		coord		[out] impact coordinates

\param		t			[out] t such that coord = origin + dir * t
*	\return					false if ray does not intersect AABB, or ray origin is inside AABB. Else:
1 + coordinate index of box axis that was hit 
Note: sign bit that determines if the minimum (0) or maximum (1) of the axis was hit is equal to sign(coord[returnVal-1]).
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PxU32 Gu::rayAABBIntersect2(const PxVec3& minimum, const PxVec3& maximum, const PxVec3& origin, const PxVec3& _dir, PxVec3& coord, PxReal & t)
{
	Ps::IntBool Inside = Ps::IntTrue;
	PxVec3 MaxT(-1.0f, -1.0f, -1.0f);
	const PxReal* dir = &_dir.x;
	// Find candidate planes.
	for(PxU32 i=0;i<3;i++)
	{
		if(origin[i] < minimum[i])
		{
			coord[i]	= minimum[i];
			Inside		= Ps::IntFalse;

			// Calculate T distances to candidate planes
			if(PX_IR(dir[i]))	MaxT[i] = (minimum[i] - origin[i]) / dir[i];
		}
		else if(origin[i] > maximum[i])
		{
			coord[i]	= maximum[i];
			Inside		= Ps::IntFalse;

			// Calculate T distances to candidate planes
			if(PX_IR(dir[i]))	MaxT[i] = (maximum[i] - origin[i]) / dir[i];
		}
	}

	// Ray origin inside bounding box
	if(Inside)
	{
		coord = origin;
		t = 0;
		return 1;
	}

	// Get largest of the maxT's for final choice of intersection
	PxU32 WhichPlane = 0;
	if(MaxT[1] > MaxT[WhichPlane])	WhichPlane = 1;
	if(MaxT[2] > MaxT[WhichPlane])	WhichPlane = 2;

	// Check final candidate actually inside box
	if(PX_IR(MaxT[WhichPlane])&0x80000000) return 0;

	for(PxU32 i=0;i<3;i++)
	{
		if(i!=WhichPlane)
		{
			coord[i] = origin[i] + MaxT[WhichPlane] * dir[i];
#ifdef RAYAABB_EPSILON
			if(coord[i] < minimum[i] - RAYAABB_EPSILON || coord[i] > maximum[i] + RAYAABB_EPSILON)	return 0;
#else
			if(coord[i] < minimum[i] || coord[i] > maximum[i])	return 0;
#endif
		}
	}
	t = MaxT[WhichPlane];
	return 1 + WhichPlane;	// ray hits box
}

// Collide ray defined by ray origin (ro) and ray direction (rd)
// with the bounding box. Returns -1 on no collision and the face index
// for first intersection if a collision is found together with
// the distance to the collision points (tnear and tfar)

// ptchernev: 
// Should we use an enum, or should we keep the anonymous ints? 
// Should we increment the return code by one (return 0 for non intersection)?

int Gu::intersectRayAABB(const PxVec3& minimum, const PxVec3& maximum, const PxVec3& ro, const PxVec3& rd, float& tnear, float& tfar)
{
	// Refactor
	int ret=-1;

	tnear = -PX_MAX_F32;
	tfar = PX_MAX_F32;
	// PT: why did we change the initial epsilon value?
	#define LOCAL_EPSILON PX_EPS_F32
	//#define LOCAL_EPSILON 0.0001f

	for(unsigned int a=0;a<3;a++)
	{
		if(rd[a]>-LOCAL_EPSILON && rd[a]<LOCAL_EPSILON)
		{
			if(ro[a]<minimum[a] || ro[a]>maximum[a])
				return -1;
		}	
		else
		{
			const PxReal OneOverDir = 1.0f / rd[a];
			PxReal t1 = (minimum[a]-ro[a]) * OneOverDir;
			PxReal t2 = (maximum[a]-ro[a]) * OneOverDir;

			unsigned int b = a;
			if(t1>t2)
			{
				PxReal t=t1;
				t1=t2;
				t2=t;
				b += 3;
			}

			if(t1>tnear)
			{
				tnear = t1;
				ret = (int)b;
			}
			if(t2<tfar)
				tfar=t2;
			if(tnear>tfar || tfar<LOCAL_EPSILON)
				return -1;
		}
	}

	if(tnear>tfar || tfar<LOCAL_EPSILON)
		return -1;

	return ret;
}

int Gu::intersectRayAABB(const Ps::aos::Vec3VArg minimum, const Ps::aos::Vec3VArg maximum, 
							const Ps::aos::Vec3VArg ro, const Ps::aos::Vec3VArg rd, 
							Ps::aos::FloatV& tnear, Ps::aos::FloatV& tfar)
{
	#define LOCAL_EPSILON PX_EPS_F32
	using namespace Ps::aos;
	const FloatV eps = FLoad(LOCAL_EPSILON);
	const Vec3V absD = V3Abs(rd);
	
	const BoolV bParallel = V3IsGrtrOrEq(Vec3V_From_FloatV_WUndefined(eps), absD);
	const BoolV bOutOfRange = BAnd(V3IsGrtr(minimum, ro), V3IsGrtr(ro, maximum));
	const BoolV bParallelAndOutOfRange(BAnd(bParallel, bOutOfRange));
	//const BoolV bMiss(BAnyTrue3(bParallelAndOutOfRange));
	if(BAllEq(bParallelAndOutOfRange, BTTTT()))
		return -1;

	const FloatV nOne = FNeg(FOne());
	const Vec3V oneOverDir = V3Recip(rd);
	PX_ALIGN(16, const PxF32 tmp[4]) = {3.f, 4.f, 5.f, 0.f};
	PX_ALIGN(16, const PxF32 tmp1[4]) = {0.f, 1.f, 2.f, 0.f};

	const Vec3V v0 = Vec3V_From_Vec4V_WUndefined(V4LoadA(tmp));
	const Vec3V v1 = Vec3V_From_Vec4V_WUndefined(V4LoadA(tmp1));
	
	const Vec3V _tSlab0 = V3Mul(V3Sub(minimum, ro), oneOverDir);
	const Vec3V _tSlab1 = V3Mul(V3Sub(maximum, ro), oneOverDir);

	const BoolV con = V3IsGrtr(_tSlab0, _tSlab1);
	const Vec3V index = V3Sel(con, v0, v1);

	const Vec3V tSlab0 = V3Min(_tSlab0, _tSlab1);
	const Vec3V tSlab1 = V3Max(_tSlab0, _tSlab1);

	const FloatV x = V3GetX(tSlab0);
	const FloatV y = V3GetY(tSlab0);
	const BoolV con0 = BAllTrue3(V3IsGrtrOrEq(Vec3V_From_FloatV_WUndefined(x), tSlab0));
	const BoolV con1 = BAllTrue3(V3IsGrtrOrEq(Vec3V_From_FloatV_WUndefined(y), tSlab0));
	const FloatV tVal = FSel(con0, x, FSel(con1, y, V3GetZ(tSlab0)));
	const FloatV i = FSel(con0, V3GetX(index), FSel(con1, V3GetY(index), V3GetZ(index)));

	const FloatV tVal2 = FMin(V3GetX(tSlab1), FMin(V3GetY(tSlab1), V3GetZ(tSlab1)));
	const BoolV bHitSlab(BAnd(FIsGrtrOrEq(tVal2, tVal), FIsGrtrOrEq(tVal2, eps)));
	const FloatV ret = FSel(bHitSlab, i, nOne);

	tnear = tVal;
	tfar = tVal2;

	return (int)FStore(ret);
	
}

// PT: specialized version where oneOverDir is available
int Gu::intersectRayAABB(const PxVec3& minimum, const PxVec3& maximum, const PxVec3& ro, const PxVec3& rd, const PxVec3& oneOverDir, float& tnear, float& tfar)
{
	// PT: why did we change the initial epsilon value?
	#define LOCAL_EPSILON PX_EPS_F32
	//#define LOCAL_EPSILON 0.0001f

	// PT: this unrolled loop is a lot faster on Xbox

#ifdef _XBOX

	// PT: seems the Xbox likes the integer code better
	const float epsilon = LOCAL_EPSILON;
	const int fabsx = (int&)(rd.x) & 0x7fffffff;
	const int fabsy = (int&)(rd.y) & 0x7fffffff;
	const int fabsz = (int&)(rd.z) & 0x7fffffff;

	if(fabsx<(int&)epsilon)
//	if(physx::intrinsics::abs(rd.x)<LOCAL_EPSILON)
//	if(rd.x>-LOCAL_EPSILON && rd.x<LOCAL_EPSILON)
		if(ro.x<minimum.x || ro.x>maximum.x)
				return -1;
	if(fabsy<(int&)epsilon)
//	if(physx::intrinsics::abs(rd.y)<LOCAL_EPSILON)
//	if(rd.y>-LOCAL_EPSILON && rd.y<LOCAL_EPSILON)
		if(ro.y<minimum.y || ro.y>maximum.y)
				return -1;
	if(fabsz<(int&)epsilon)
//	if(physx::intrinsics::abs(rd.z)<LOCAL_EPSILON)
//	if(rd.z>-LOCAL_EPSILON && rd.z<LOCAL_EPSILON)
		if(ro.z<minimum.z || ro.z>maximum.z)
				return -1;

	const PxReal t1x_candidate = (minimum.x - ro.x) * oneOverDir.x;
	const PxReal t2x_candidate = (maximum.x - ro.x) * oneOverDir.x;
	const PxReal t1y_candidate = (minimum.y - ro.y) * oneOverDir.y;
	const PxReal t2y_candidate = (maximum.y - ro.y) * oneOverDir.y;
	const PxReal t1z_candidate = (minimum.z - ro.z) * oneOverDir.z;
	const PxReal t2z_candidate = (maximum.z - ro.z) * oneOverDir.z;

	const float deltax = t1x_candidate - t2x_candidate;
	const float deltay = t1y_candidate - t2y_candidate;
	const float deltaz = t1z_candidate - t2z_candidate;

	const float t1x = physx::intrinsics::fsel(deltax, t2x_candidate, t1x_candidate);
	const float t1y = physx::intrinsics::fsel(deltay, t2y_candidate, t1y_candidate);
	const float t1z = physx::intrinsics::fsel(deltaz, t2z_candidate, t1z_candidate);

	const float t2x = physx::intrinsics::fsel(deltax, t1x_candidate, t2x_candidate);
	const float t2y = physx::intrinsics::fsel(deltay, t1y_candidate, t2y_candidate);
	const float t2z = physx::intrinsics::fsel(deltaz, t1z_candidate, t2z_candidate);

	const float bxf = physx::intrinsics::fsel(deltax, 3.0f, 0.0f);
	const float byf = physx::intrinsics::fsel(deltay, 4.0f, 1.0f);
	const float bzf = physx::intrinsics::fsel(deltaz, 5.0f, 2.0f);

	tnear = t1x;
	tfar = t2x;
	float ret = bxf;

	const float delta = t1y - tnear;
	tnear = physx::intrinsics::fsel(delta, t1y, tnear);
	ret = physx::intrinsics::fsel(delta, byf, ret);
	tfar = physx::intrinsics::selectMin(tfar, t2y);

	const float delta2 = t1z - tnear;
	tnear = physx::intrinsics::fsel(delta2, t1z, tnear);
	ret = physx::intrinsics::fsel(delta2, bzf, ret);

	tfar = physx::intrinsics::selectMin(tfar, t2z);

	if(tnear>tfar || tfar<LOCAL_EPSILON)	// PT: TODO: get rid of that one
		return -1;

	return int(ret);

#else

	if(physx::intrinsics::abs(rd.x)<LOCAL_EPSILON)
//	if(rd.x>-LOCAL_EPSILON && rd.x<LOCAL_EPSILON)
		if(ro.x<minimum.x || ro.x>maximum.x)
				return -1;
	if(physx::intrinsics::abs(rd.y)<LOCAL_EPSILON)
//	if(rd.y>-LOCAL_EPSILON && rd.y<LOCAL_EPSILON)
		if(ro.y<minimum.y || ro.y>maximum.y)
				return -1;
	if(physx::intrinsics::abs(rd.z)<LOCAL_EPSILON)
//	if(rd.z>-LOCAL_EPSILON && rd.z<LOCAL_EPSILON)
		if(ro.z<minimum.z || ro.z>maximum.z)
				return -1;

	PxReal t1x = (minimum.x - ro.x) * oneOverDir.x;
	PxReal t2x = (maximum.x - ro.x) * oneOverDir.x;
	PxReal t1y = (minimum.y - ro.y) * oneOverDir.y;
	PxReal t2y = (maximum.y - ro.y) * oneOverDir.y;
	PxReal t1z = (minimum.z - ro.z) * oneOverDir.z;
	PxReal t2z = (maximum.z - ro.z) * oneOverDir.z;

	int bx;
	int by;
	int bz;

	if(t1x>t2x)
	{
		PxReal t=t1x; t1x=t2x; t2x=t;
		bx = 3;
	}
	else
	{
		bx = 0;
	}

	if(t1y>t2y)
	{
		PxReal t=t1y; t1y=t2y; t2y=t;
		by = 4;
	}
	else
	{
		by = 1;
	}

	if(t1z>t2z)
	{
		PxReal t=t1z; t1z=t2z; t2z=t;
		bz = 5;
	}
	else
	{
		bz = 2;
	}

	int ret;
//	if(t1x>tnear)	// PT: no need to test for the first value
	{
		tnear = t1x;
		ret = bx;
	}
//	tfar = PxMin(tfar, t2x);
	tfar = t2x;		// PT: no need to test for the first value

	if(t1y>tnear)
	{
		tnear = t1y;
		ret = by;
	}
	tfar = PxMin(tfar, t2y);

	if(t1z>tnear)
	{
		tnear = t1z;
		ret = bz;
	}
	tfar = PxMin(tfar, t2z);

	if(tnear>tfar || tfar<LOCAL_EPSILON)
		return -1;

	return ret;
#endif
}

int Gu::intersectRayAABB(const Ps::aos::Vec3VArg minimum, const Ps::aos::Vec3VArg maximum, 
							const Ps::aos::Vec3VArg ro, const Ps::aos::Vec3VArg rd,const Ps::aos::Vec3VArg oneOverDir, 
							Ps::aos::FloatV& tnear, Ps::aos::FloatV& tfar)
{
	#define LOCAL_EPSILON PX_EPS_F32
	using namespace Ps::aos;
	const FloatV eps = FLoad(LOCAL_EPSILON);
	const Vec3V absD = V3Abs(rd);
	
	const BoolV bParallel = V3IsGrtrOrEq(Vec3V_From_FloatV_WUndefined(eps), absD);
	const BoolV bOutOfRange = BAnd(V3IsGrtr(minimum, ro), V3IsGrtr(ro, maximum));
	const BoolV bParallelAndOutOfRange(BAnd(bParallel, bOutOfRange));
	//const BoolV bMiss(BAnyTrue3(bParallelAndOutOfRange));
	if(BAllEq(bParallelAndOutOfRange, BTTTT()))
		return -1;

	const FloatV nOne = FNeg(FOne());
	PX_ALIGN(16, const PxF32 tmp[4]) = {3.f, 4.f, 5.f, 0.f};
	PX_ALIGN(16, const PxF32 tmp1[4]) = {0.f, 1.f, 2.f, 0.f};

	const Vec3V v0 = Vec3V_From_Vec4V_WUndefined(V4LoadA(tmp));
	const Vec3V v1 = Vec3V_From_Vec4V_WUndefined(V4LoadA(tmp1));
	
	const Vec3V _tSlab0 = V3Mul(V3Sub(minimum, ro), oneOverDir);
	const Vec3V _tSlab1 = V3Mul(V3Sub(maximum, ro), oneOverDir);

	const BoolV con = V3IsGrtr(_tSlab0, _tSlab1);
	const Vec3V index = V3Sel(con, v0, v1);

	const Vec3V tSlab0 = V3Min(_tSlab0, _tSlab1);
	const Vec3V tSlab1 = V3Max(_tSlab0, _tSlab1);

	const FloatV x = V3GetX(tSlab0);
	const FloatV y = V3GetY(tSlab0);
	const BoolV con0 = BAllTrue3(V3IsGrtrOrEq(Vec3V_From_FloatV_WUndefined(x), tSlab0));
	const BoolV con1 = BAllTrue3(V3IsGrtrOrEq(Vec3V_From_FloatV_WUndefined(y), tSlab0));
	const FloatV tVal = FSel(con0, x, FSel(con1, y, V3GetZ(tSlab0)));
	const FloatV i = FSel(con0, V3GetX(index), FSel(con1, V3GetY(index), V3GetZ(index)));

	const FloatV tVal2 = FMin(V3GetX(tSlab1), FMin(V3GetY(tSlab1), V3GetZ(tSlab1)));
	const BoolV bHitSlab(BAnd(FIsGrtrOrEq(tVal2, tVal), FIsGrtrOrEq(tVal2, eps)));
	
	const FloatV ret = FSel(bHitSlab, i, nOne);

	tnear = tVal;
	tfar = tVal2;

	return (int)FStore(ret);
	
}

bool Gu::intersectRayAABB2(
	const PxVec3& minimum, const PxVec3& maximum, const PxVec3& ro, const PxVec3& rd, float maxDist, float& tnear, float& tfar)
{
	PX_ASSERT(maximum.x-minimum.x >= GU_MIN_AABB_EXTENT*0.5f);
	PX_ASSERT(maximum.y-minimum.y >= GU_MIN_AABB_EXTENT*0.5f);
	PX_ASSERT(maximum.z-minimum.z >= GU_MIN_AABB_EXTENT*0.5f);
	// not using vector math due to vector to integer pipeline penalties. TODO: verify that it's indeed faster
	namespace i = physx::intrinsics;

	// P+tD=a; t=(a-P)/D
	// t=(a - p.x)*1/d.x = a/d.x +(- p.x/d.x)
	const PxF32 dEpsilon = 1e-9f;
	// using recipFast fails height field unit tests case where a ray cast from y=10000 to 0 gets clipped to 0.27 in y
	PxF32 invDx = i::recip(i::selectMax(i::abs(rd.x), dEpsilon) * i::sign(rd.x));
	PxF32 tx0 = (minimum.x - ro.x) * invDx;
	PxF32 tx1 = (maximum.x - ro.x) * invDx;
	PxF32 txMin = i::selectMin(tx0, tx1);
	PxF32 txMax = i::selectMax(tx0, tx1);

	PxF32 invDy = i::recip(i::selectMax(i::abs(rd.y), dEpsilon) * i::sign(rd.y));
	PxF32 ty0 = (minimum.y - ro.y) * invDy;
	PxF32 ty1 = (maximum.y - ro.y) * invDy;
	PxF32 tyMin = i::selectMin(ty0, ty1);
	PxF32 tyMax = i::selectMax(ty0, ty1);

	PxF32 invDz = i::recip(i::selectMax(i::abs(rd.z), dEpsilon) * i::sign(rd.z));
	PxF32 tz0 = (minimum.z - ro.z) * invDz;
	PxF32 tz1 = (maximum.z - ro.z) * invDz;
	PxF32 tzMin = i::selectMin(tz0, tz1);
	PxF32 tzMax = i::selectMax(tz0, tz1);

	PxF32 maxOfNears = i::selectMax(i::selectMax(txMin, tyMin), tzMin);
	PxF32 minOfFars  = i::selectMin(i::selectMin(txMax, tyMax), tzMax);

	tnear = i::selectMax(maxOfNears, 0.0f);
	tfar  = i::selectMin(minOfFars, maxDist);

	return (tnear<tfar);
}

bool Gu::intersectRayAABB2(const Ps::aos::Vec3VArg minimum, const Ps::aos::Vec3VArg maximum, 
					   const Ps::aos::Vec3VArg ro, const Ps::aos::Vec3VArg rd, const Ps::aos::FloatVArg maxDist, 
					   Ps::aos::FloatV& tnear, Ps::aos::FloatV& tfar)
{
	using namespace Ps::aos;
	const FloatV zero = FZero();
	const Vec3V eps = V3Load(1e-9f);
	const Vec3V absRD = V3Max(V3Abs(rd), eps);
	const Vec3V signRD = V3Sign(rd);
	const Vec3V rdV = V3Mul(absRD, signRD);
	const Vec3V rdVRecip = V3Recip(rdV);
	const Vec3V _min = V3Mul(V3Sub(minimum, ro), rdVRecip);
	const Vec3V _max = V3Mul(V3Sub(maximum, ro), rdVRecip);
	const Vec3V min = V3Min(_max, _min);
	const Vec3V max = V3Max(_max, _min);
	const FloatV maxOfNears = FMax(V3GetX(min), FMax(V3GetY(min), V3GetZ(min)));
	const FloatV minOfFars = FMin(V3GetX(max), FMin(V3GetY(max), V3GetZ(max)));

	tnear = FMax(maxOfNears, zero);
	tfar  = FMin(minOfFars, maxDist);
	//tfar  = FAdd(FMin(minOfFars, maxDist), V3GetX(eps)); // AP: + epsilon because a test vs empty box should return true

	return FAllGrtr(tfar, tnear) != 0;
}
