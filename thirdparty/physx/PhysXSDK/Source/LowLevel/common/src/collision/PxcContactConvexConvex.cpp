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

#include "PsMathUtils.h"
#include "GuContactPolygonPolygon.h"
#include "GuGeometryUnion.h"
#include "GuConvexHelper.h"
#include "GuGeomUtilsInternal.h"
#include "GuSeparatingAxes.h"
#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"
#include "GuContactBuffer.h"

// csigg: the single reference of gEnableOptims (below) has been
// replaced with the actual value to prevent ios64 compiler crash.
// static const int gEnableOptims = 1;
#define CONVEX_CONVEX_ROUGH_FIRST_PASS
#define TEST_INTERNAL_OBJECTS
#ifdef TEST_INTERNAL_OBJECTS
	#define USE_BOX_DATA
#endif

using namespace physx;
using namespace Gu;

#ifdef TEST_INTERNAL_OBJECTS
#ifdef USE_BOX_DATA
PX_FORCE_INLINE void BoxSupport(const float extents[3], const PxVec3& sv, float p[3])
{
#if defined(PX_X360) || defined(PX_PS3)
	p[0] = physx::intrinsics::fsel(sv.x, extents[0], -extents[0]);
	p[1] = physx::intrinsics::fsel(sv.y, extents[1], -extents[1]);
	p[2] = physx::intrinsics::fsel(sv.z, extents[2], -extents[2]);
#else
	// This version is ~11.000 cycles (4%) faster overall in one of the tests.
	PX_IR(p[0]) = PX_IR(extents[0])|(PX_IR(sv.x)&PX_SIGN_BITMASK);
	PX_IR(p[1]) = PX_IR(extents[1])|(PX_IR(sv.y)&PX_SIGN_BITMASK);
	PX_IR(p[2]) = PX_IR(extents[2])|(PX_IR(sv.z)&PX_SIGN_BITMASK);
//	p[0] = sv[0] < 0.0f ? -extents[0] : extents[0];
//	p[1] = sv[1] < 0.0f ? -extents[1] : extents[1];
//	p[2] = sv[2] < 0.0f ? -extents[2] : extents[2];
#endif
}

#endif

#ifdef PX_DEBUG
static const PxReal testInternalObjectsEpsilon = 1.0e-3f;
#endif

#ifdef DO_NOT_REMOVE
	PX_FORCE_INLINE void TestInternalObjects_(	const PxVec3& delta_c, const PxVec3& axis,
												const PolygonalData& polyData0, const PolygonalData& polyData1,
												const Cm::Matrix34& tr0, const Cm::Matrix34& tr1,
												float& dmin, float contactDistance)
	{
		{
/*			float projected0 = axis.dot(tr0.base3);
			float projected1 = axis.dot(tr1.base3);
			float min0 = projected0 - polyData0.mInternal.mRadius;
			float max0 = projected0 + polyData0.mInternal.mRadius;
			float min1 = projected1 - polyData1.mInternal.mRadius;
			float max1 = projected1 + polyData1.mInternal.mRadius;
*/
			float MinMaxRadius = polyData0.mInternal.mRadius  + polyData1.mInternal.mRadius;
			PxVec3 delta = tr0.base3 - tr1.base3;
			const PxReal dp = axis.dot(delta);
//			const PxReal dp2 = axis.dot(delta_c);

//			const PxReal d0 = max0 - min1;
//			const PxReal d0 = projected0 + polyData0.mInternal.mRadius - projected1 + polyData1.mInternal.mRadius;
//			const PxReal d0 = projected0 - projected1 + polyData0.mInternal.mRadius  + polyData1.mInternal.mRadius;
//			const PxReal d0 = projected0 - projected1 + MinMaxRadius;
//			const PxReal d0 = axis.dot(tr0.base3) - axis.dot(tr1.base3) + MinMaxRadius;
//			const PxReal d0 = axis.dot(tr0.base3 - tr1.base3) + MinMaxRadius;
//			const PxReal d0 = MinMaxRadius + axis.dot(delta);
			const PxReal d0 = MinMaxRadius + dp;

//			const PxReal d1 = max1 - min0;
//			const PxReal d1 = projected1 + polyData1.mInternal.mRadius - projected0 + polyData0.mInternal.mRadius;
//			const PxReal d1 = projected1 - projected0 + polyData1.mInternal.mRadius + polyData0.mInternal.mRadius;
//			const PxReal d1 = projected1 - projected0 + MinMaxRadius;
//			const PxReal d1 = axis.dot(tr1.base3) - axis.dot(tr0.base3) + MinMaxRadius;
//			const PxReal d1 = axis.dot(tr1.base3 - tr0.base3) + MinMaxRadius;
//			const PxReal d1 = MinMaxRadius - axis.dot(delta);
			const PxReal d1 = MinMaxRadius - dp;

			dmin = physx::intrinsics::selectMin(d0, d1);
			return;
		}

	#ifdef USE_BOX_DATA
		const PxVec3 localAxis0 = tr0.rotateTranspose(axis);
		const PxVec3 localAxis1 = tr1.rotateTranspose(axis);

		const float dp = delta_c.dot(axis);

		float p0[3];
		BoxSupport(polyData0.mInternal.mExtents, localAxis0, p0);
		float p1[3];
		BoxSupport(polyData1.mInternal.mExtents, localAxis1, p1);

		const float Radius0 = p0[0]*localAxis0.x + p0[1]*localAxis0.y + p0[2]*localAxis0.z;
		const float Radius1 = p1[0]*localAxis1.x + p1[1]*localAxis1.y + p1[2]*localAxis1.z;

		const float MinRadius = physx::intrinsics::selectMax(Radius0, polyData0.mInternal.mRadius);
		const float MaxRadius = physx::intrinsics::selectMax(Radius1, polyData1.mInternal.mRadius);
	#else
		const float dp = delta_c.dot(axis);
		const float MinRadius = polyData0.mInternal.mRadius;
		const float MaxRadius = polyData1.mInternal.mRadius;
	#endif
		const float MinMaxRadius = MaxRadius + MinRadius;
		const float d0 = MinMaxRadius + dp;
		const float d1 = MinMaxRadius - dp;

		dmin = physx::intrinsics::selectMin(d0, d1);
	}
#endif

PX_FORCE_INLINE bool TestInternalObjects(	const PxVec3& delta_c, const PxVec3& axis,
											const PolygonalData& polyData0, const PolygonalData& polyData1,
											const Cm::Matrix34& tr0, const Cm::Matrix34& tr1,
											float dmin, float /*contactDistance*/)
{
#ifdef USE_BOX_DATA
	const PxVec3 localAxis0 = tr0.rotateTranspose(axis);
	const PxVec3 localAxis1 = tr1.rotateTranspose(axis);

	const float dp = delta_c.dot(axis);

	float p0[3];
	BoxSupport(polyData0.mInternal.mExtents, localAxis0, p0);
	float p1[3];
	BoxSupport(polyData1.mInternal.mExtents, localAxis1, p1);

	const float Radius0 = p0[0]*localAxis0.x + p0[1]*localAxis0.y + p0[2]*localAxis0.z;
	const float Radius1 = p1[0]*localAxis1.x + p1[1]*localAxis1.y + p1[2]*localAxis1.z;

	const float MinRadius = physx::intrinsics::selectMax(Radius0, polyData0.mInternal.mRadius);
	const float MaxRadius = physx::intrinsics::selectMax(Radius1, polyData1.mInternal.mRadius);
#else
	const float dp = delta_c.dot(axis);
	const float MinRadius = polyData0.mInternal.mRadius;
	const float MaxRadius = polyData1.mInternal.mRadius;
#endif
	const float MinMaxRadius = MaxRadius + MinRadius;
	const float d0 = MinMaxRadius + dp;
	const float d1 = MinMaxRadius - dp;

	const float depth = physx::intrinsics::selectMin(d0, d1);
	if(depth>dmin)
		return false;
	return true;
}
#endif

PX_FORCE_INLINE float PxcMultiplyAdd3x4(PxU32 i, const PxVec3& p0, const PxVec3& p1, const Cm::Matrix34& world)
{
	return (p1.x + p0.x) * world.base0[i] + (p1.y + p0.y) * world.base1[i] + (p1.z + p0.z) * world.base2[i] + 2.0f * world.base3[i];
}

PX_FORCE_INLINE float PxcMultiplySub3x4(PxU32 i, const PxVec3& p0, const PxVec3& p1, const Cm::Matrix34& world)
{
	return (p1.x - p0.x) * world.base0[i] + (p1.y - p0.y) * world.base1[i] + (p1.z - p0.z) * world.base2[i];
}

static PX_FORCE_INLINE bool testNormal(	const PxVec3& axis, PxReal min0, PxReal max0,
										const PolygonalData& polyData1,
										const Cm::Matrix34& m1to0,
										const Cm::FastVertex2ShapeScaling& scaling1,
										PxReal& depth, PxReal contactDistance)
{
	//The separating axis we want to test is a face normal of hull0
	PxReal min1, max1;
	(polyData1.mProjectHull)(polyData1, axis, m1to0, scaling1, min1, max1);

#ifdef _XBOX
	depth = physx::intrinsics::selectMin(max0 - min1, max1 - min0);
	const float cndt0 = physx::intrinsics::fsel(max0 + contactDistance - min1, 1.0f, 0.0f);
	const float cndt1 = physx::intrinsics::fsel(max1 + contactDistance - min0, 1.0f, 0.0f);
	return (cndt0*cndt1)!=0.0f;
#else
	if(max0+contactDistance<min1 || max1+contactDistance<min0)
		return false;

	const PxReal d0 = max0 - min1;
	const PxReal d1 = max1 - min0;
	depth = physx::intrinsics::selectMin(d0, d1);
	return true;
#endif
}

static PX_FORCE_INLINE bool testSeparatingAxis(	const PolygonalData& polyData0, const PolygonalData& polyData1,
												const Cm::Matrix34& world0, const Cm::Matrix34& world1,
												const Cm::FastVertex2ShapeScaling& scaling0, const Cm::FastVertex2ShapeScaling& scaling1,
												const PxVec3& axis, PxReal& depth, PxReal contactDistance)
{
	PxReal min0, max0;
	(polyData0.mProjectHull)(polyData0, axis, world0, scaling0, min0, max0);

	return testNormal(axis, min0, max0, polyData1, world1, scaling1, depth, contactDistance);
}

// PT: use default version for non-Xbox platforms - for now
#ifndef _XBOX

static bool PxcSegmentAABBIntersect(const PxVec3& p0, const PxVec3& p1, 
									const PxVec3& minimum, const PxVec3& maximum, 
									const Cm::Matrix34& world)
{
	PxVec3 boxExtent, diff, dir;
	PxReal fAWdU[3];

	dir.x = PxcMultiplySub3x4(0, p0, p1, world);
	boxExtent.x = maximum.x - minimum.x;
	diff.x = (PxcMultiplyAdd3x4(0, p0, p1, world) - (maximum.x+minimum.x));
	fAWdU[0] = PxAbs(dir.x);
	if(PxAbs(diff.x)> boxExtent.x + fAWdU[0]) return false;

	dir.y = PxcMultiplySub3x4(1, p0, p1, world);
	boxExtent.y = maximum.y - minimum.y;
	diff.y = (PxcMultiplyAdd3x4(1, p0, p1, world) - (maximum.y+minimum.y));
	fAWdU[1] = PxAbs(dir.y);
	if(PxAbs(diff.y)> boxExtent.y + fAWdU[1]) return false;

	dir.z = PxcMultiplySub3x4(2, p0, p1, world);
	boxExtent.z = maximum.z - minimum.z;
	diff.z = (PxcMultiplyAdd3x4(2, p0, p1, world) - (maximum.z+minimum.z));
	fAWdU[2] = PxAbs(dir.z);
	if(PxAbs(diff.z)> boxExtent.z + fAWdU[2]) return false;

	PxReal f;
	f = dir.y * diff.z - dir.z*diff.y; if(PxAbs(f)>boxExtent.y*fAWdU[2] + boxExtent.z*fAWdU[1]) return false;
	f = dir.z * diff.x - dir.x*diff.z; if(PxAbs(f)>boxExtent.x*fAWdU[2] + boxExtent.z*fAWdU[0]) return false;
	f = dir.x * diff.y - dir.y*diff.x; if(PxAbs(f)>boxExtent.x*fAWdU[1] + boxExtent.y*fAWdU[0]) return false;
	return true;
}

#define EXPERIMENT

/*
Edge culling can clearly be improved : in ConvexTest02 for example, edges don't even touch the other mesh sometimes.
*/
static void PxcFindSeparatingAxes(	SeparatingAxes& sa, const PxU32* PX_RESTRICT indices, PxU32 numPolygons,
									const PolygonalData& polyData,
									const Cm::Matrix34& world0, const PxPlane& plane,
									const Cm::Matrix34& m0to1, const PxBounds3& aabb, PxReal contactDistance,
									const Cm::FastVertex2ShapeScaling& scaling)
{
//	EdgeCache edgeCache;	// PT: TODO: check this is actually useful
	const PxVec3* PX_RESTRICT vertices = polyData.mVerts;
	const Gu::HullPolygonData* PX_RESTRICT polygons = polyData.mPolygons;
	const PxU8* PX_RESTRICT vrefsBase = polyData.mPolygonVertexRefs;

	while(numPolygons--)
	{
		//Get current polygon
		const Gu::HullPolygonData& P = polygons[*indices++];
		const PxU8* PX_RESTRICT VData = vrefsBase + P.mVRef8;

		// Loop through polygon vertices == polygon edges
		PxU32 numVerts = P.mNbVerts;

#ifdef EXPERIMENT
		PxVec3 p0,p1;
		PxU8 VRef0 = VData[0];
		p0 = scaling * vertices[VRef0];
		bool b0 = plane.distance(p0) <= contactDistance;
#endif

		for(PxU32 j = 0; j < numVerts; j++)
		{
			PxU32 j1 = j+1;
			if(j1 >= numVerts) j1 = 0;

#ifndef EXPERIMENT
			PxU8 VRef0 = VData[j];
#endif
			PxU8 VRef1 = VData[j1];

//			Ps::order(VRef0, VRef1); //make sure edge  (a,b) == edge (b,a)
//			if (edgeCache.isInCache(VRef0, VRef1))
//				continue;

			//transform points //TODO: once this works we could transform plan instead, that is more efficient!!

#ifdef EXPERIMENT
			p1 = scaling * vertices[VRef1];
#else
			const PxVec3 p0 = scaling * vertices[VRef0];
			const PxVec3 p1 = scaling * vertices[VRef1];
#endif

			// Cheap but effective culling!
#ifdef EXPERIMENT
			bool b1 = plane.distance(p1) <= contactDistance;
			if(b0 || b1)
#else
			if(plane.signedDistanceHessianNormalForm(p0) <= contactDistance ||
				plane.signedDistanceHessianNormalForm(p1) <= contactDistance)
#endif
			{
				if(PxcSegmentAABBIntersect(p0, p1, aabb.minimum, aabb.maximum, m0to1))
				{
					// Create current edge. We're only interested in different edge directions so we normalize.
					const PxVec3 currentEdge = world0.rotate(p0 - p1).getNormalized();
					sa.addAxis(currentEdge);
				}
			}
#ifdef EXPERIMENT
			VRef0 = VRef1;
			p0 = p1;
			b0 = b1;
#endif
		}
	}
}

#else

#include "PsVecMath.h"
using namespace Ps::aos;

union FloatInt
{
	float	f;
	int		i;
};

// PT: use this version on Xbox
static int PxcSegmentAABBIntersect_VMX_Final(
	const PxVec3& p0, const PxVec3& p1, 
	const Vec3V basis0,
	const Vec3V basis1,
	const Vec3V basis2,
	const Vec3V basis3_2,
	const Vec3V boxExtent4,
	const Vec3V boxSum4,
	const Vec3V boxExtYXX,
	const Vec3V boxExtZZY
	)
{
	const Vec3V p0_4 = V3LoadUnsafeA(p0);
	const Vec3V p1_4 = V3LoadUnsafeA(p1);

	const Vec3V delta4 = V3Sub(p1_4, p0_4);
	Vec3V dir4 = M33MulV3(Mat33V(basis0, basis1, basis2), delta4);

	const Vec3V fAWdU4 = V3Abs(dir4);

	const Vec3V boxExt_absDir4 = V3Add(boxExtent4, fAWdU4);

	const Vec3V sum4 = V3Add(p1_4, p0_4);
	const Vec3V tmp4b = M33MulV3AddV3(Mat33V(basis0, basis1, basis2), sum4, basis3_2);
	const Vec3V diff4 = V3Sub(tmp4b, boxSum4);

	if(!V3AllGrtrOrEq(boxExt_absDir4, V3Abs(diff4)))
		return 0;

	const Vec3V f4 = V3Cross(dir4, diff4);

	const Vec3V fAWdU_ZZY =  V3PermZZY(fAWdU4);
	const Vec3V fAWdU_YXX =  V3PermYXX(fAWdU4);
	Vec3V boxExt_absDir2_4 =  V3Mul(boxExtYXX, fAWdU_ZZY);
	boxExt_absDir2_4 =  V3MulAdd(boxExtZZY, fAWdU_YXX, boxExt_absDir2_4);	// "For best performance, the result should be assigned back to either V2 or V3."

	if(!V3AllGrtrOrEq(boxExt_absDir2_4, V3Abs(f4)))
		return 0;
	return 1;
}

// PT: this function saves a FCMP because we know the edge magnitude won't be zero
PX_FORCE_INLINE PxVec3 getNormalized(const PxVec3& src)
{
	const PxReal m = src.magnitudeSquared();
	PX_ASSERT(m!=0.0f);
	return src * PxRecipSqrt(m);
}

//#define USE_SIMD_NORMALIZE	// Ends up slower because it intoduces LHS
#ifdef USE_SIMD_NORMALIZE
PX_FORCE_INLINE void SIMD_Normalize(PxVec3& n, float& oneOverM)
{
//	const PxReal magnitude = shapeSpaceNormal.normalize();
	const Cm::PxSimd::Vector4 v = Cm::PxSimd::load(n);
	Cm::PxSimd::Vector4 oneOverN; 
	const Cm::PxSimd::Vector4 normalized = Cm::PxSimd::normalizeRecip(v,oneOverN);
	Cm::PxSimd::store(oneOverM, oneOverN);
	Cm::PxSimd::store(n, normalized);
}
#endif

// PT: this one gets rid of the 2 biggest "top issues" in the convex stack scene but it's stll not great because
// we could compute 4 times less stuff here.
#define USE_VRSQRTEFP

	struct Vertex
	{
		PxVec3		pt;	// Pre-transformed vertex
		FloatInt	d;	// Distance to plane - contact distance
	};

/*static void transformVertices_VMX(PxU32 nb, Vertex* PX_RESTRICT transformed, const PxVec3* PX_RESTRICT vertices, const Cm::FastVertex2ShapeScaling& scaling, const PxVec3& n, float offset)
{
	const PxMat33Legacy& scaleRot = scaling.getVertex2ShapeSkew();
	const PxMat33& newMat = reinterpret_cast<const PxMat33&>(scaleRot);	// PT: see proof in PxcShapeConvex.cpp
	const Cm::PxSimd::Vector4 scaleBase0 = Cm::PxSimd::load(newMat.column0);	// ### shouldn't be reloaded all the time!
	const Cm::PxSimd::Vector4 scaleBase1 = Cm::PxSimd::load(newMat.column1);	// ### shouldn't be reloaded all the time!
	const Cm::PxSimd::Vector4 scaleBase2 = Cm::PxSimd::load(newMat.column2);	// ### shouldn't be reloaded all the time!

	const Cm::PxSimd::Vector4 planeNormal4 = Cm::PxSimd::load(n);	// ### shouldn't be reloaded all the time!
	const Cm::PxSimd::Vector4 offset4 = Cm::PxSimd::load(offset);	// ### shouldn't be reloaded all the time!

	for(PxU32 i=0;i<nb;i++)
	{
//		const PxVec3 tmp = scaling * vertices[i];	// PT: writing it in a local variable *does* remove an LHS, even with a restricted pointer...
		const Cm::PxSimd::Vector4 pt = Cm::PxSimd::load(vertices[i]);
		const Cm::PxSimd::Vector4 tmp = Cm::PxSimd::rotateInv(scaleBase0, scaleBase1, scaleBase2, pt);

//		transformed[i].pt = tmp;
		Cm::PxSimd::store(transformed[i].pt, tmp);

//		transformed[i].d.f = plane.normal.dot(tmp) + offset;	// Cheap but effective culling!
		const Cm::PxSimd::Vector4 dist = Cm::PxSimd::add(offset4, Cm::PxSimd::dot(planeNormal4, tmp));
		Cm::PxSimd::store(transformed[i].d.f, dist);
	}
}*/

static void PxcFindSeparatingAxes(	SeparatingAxes& sa, const PxU32* PX_RESTRICT indices, PxU32 numPolygons,
									const PolygonalData& polyData,
									const Cm::Matrix34& world0, const PxPlane& plane,
									const Cm::Matrix34& m0to1, const PxBounds3& aabb, PxReal contactDistance,
									const Cm::FastVertex2ShapeScaling& scaling)
{
	// TODO:
	// - precompute plane distances
	// - inverse rotate plane
	// - optimize ray-aabb box
	//		- box extents
	//		- point rotations
	// - optimize normalize
	// - check that PxRecipSqrt() avoids a divide!! <= nope, doesn't

	const PxVec3* PX_RESTRICT vertices = polyData.mVerts;
	const Gu::HullPolygonData* PX_RESTRICT polygons = polyData.mPolygons;
	const PxU8* PX_RESTRICT vrefsBase = polyData.mPolygonVertexRefs;

	// ### prefetch vertices here?

	// Pre-transform vertices
	// ### skip if identity
	// ### once this works we could transform plan instead, that is more efficient!!
	const PxU32 totalNbVerts = polyData.mNbVerts;
	Vertex* PX_RESTRICT transformed = (Vertex* PX_RESTRICT)PxAlloca(totalNbVerts*sizeof(Vertex));
	const float offset = plane.d - contactDistance;
	for(PxU32 i=0;i<totalNbVerts;i++)
	{
		const PxVec3 tmp = scaling * vertices[i];	// PT: writing it in a local variable *does* remove an LHS, even with a restricted pointer...
		transformed[i].pt = tmp;
		transformed[i].d.f = plane.n.dot(tmp) + offset;	// Cheap but effective culling!
	}
//	transformVertices_VMX(totalNbVerts, transformed, vertices, scaling, plane.normal, offset);

	// Precompute some data for the ray-box test. Let us do some work while finishing storing the stuff above...
	const PxVec3 boxExtent = aabb.maximum - aabb.minimum;
	const PxVec3 boxSum = aabb.maximum + aabb.minimum;

// PT: keep constant SIMD vectors loaded
const Vec3V basis0 = V3LoadUnsafeA(m0to1.base0);	// ### keep in vector form
const Vec3V basis1 = V3LoadUnsafeA(m0to1.base1);	// ### keep in vector form
const Vec3V basis2 = V3LoadUnsafeA(m0to1.base2);	// ### keep in vector form
const Vec3V basis3 = V3LoadUnsafeA(m0to1.base3);	// ### keep in vector form
const Vec3V basis3_2 = V3Add(basis3, basis3);	// ### keep in vector form
const Vec3V boxExtent4 = V3LoadUnsafeA(boxExtent);	// ### keep in vector form
const Vec3V boxSum4 = V3LoadUnsafeA(boxSum);	// ### keep in vector form
const Vec3V boxExtYXX =  V3PermYXX(boxExtent4);	// ### keep in vector form
const Vec3V boxExtZZY =  V3PermZZY(boxExtent4);	// ### keep in vector form
#ifdef USE_VRSQRTEFP
const Vec3V world0_basis0 = V3LoadUnsafeA(world0.base0);	// ### keep in vector form
const Vec3V world0_basis1 = V3LoadUnsafeA(world0.base1);	// ### keep in vector form
const Vec3V world0_basis2 = V3LoadUnsafeA(world0.base2);	// ### keep in vector form
#endif
	// ### prefetch polygons here?

	const PxU32* PX_RESTRICT last = indices + numPolygons;
	while(indices!=last)
	{
		//Get current polygon
		const Gu::HullPolygonData& P = polygons[*indices++];
		const PxU8* PX_RESTRICT VData = vrefsBase + P.mVRef8;

		// Loop through polygon vertices == polygon edges
		const PxU32 numVerts = P.mNbVerts;
//		const PxU8* PX_RESTRICT VDataLast = VData + numVerts;
		for(PxU32 j=0; j<numVerts; j++)
//		while(VData!=VDataLast)
		{
			PxU32 j1 = j+1;
			if(j1 >= numVerts) j1 = 0;
			const PxU8 VRef0 = VData[j];	// ### reading 1 byte!
			const PxU8 VRef1 = VData[j1];	// ### reading 1 byte!

			if(transformed[VRef0].d.i <= 0 || transformed[VRef1].d.i <= 0)
			{
				const PxVec3& p0 = transformed[VRef0].pt;
				const PxVec3& p1 = transformed[VRef1].pt;

				if(PxcSegmentAABBIntersect_VMX_Final(
					p0, p1,
					basis0, basis1, basis2, basis3_2,
					boxExtent4, boxSum4,
					boxExtYXX, boxExtZZY
					))
				{
#ifdef USE_VRSQRTEFP
					const Vec3V p0_4 = V3LoadUnsafeA(p0);
					const Vec3V p1_4 = V3LoadUnsafeA(p1);
					const Vec3V delta_4 = V3Sub(p0_4, p1_4);
					const Vec3V rotated_4 = M33MulV3(Mat33V(world0_basis0, world0_basis1, world0_basis2), delta_4);
					const Vec3V currentEdge4 = V3NormalizeSafe(rotated_4);
					PxVec3 currentEdge;
					V3StoreU(currentEdge4, currentEdge);
					sa.addAxis(currentEdge);
#else
					// Create current edge. We're only interested in different edge directions so we normalize.
					const PxVec3 currentEdge = getNormalized(world0.rotate(p0 - p1));
					sa.addAxis(currentEdge);
#endif
				}
			}
		}
	}
}
#endif



static bool PxcTestFacesSepAxesBackface(const PolygonalData& polyData0, const PolygonalData& polyData1,
										const Cm::Matrix34& world0, const Cm::Matrix34& world1,
										 const Cm::FastVertex2ShapeScaling& scaling0, const Cm::FastVertex2ShapeScaling& scaling1,
										const Cm::Matrix34& m1to0, const PxVec3& delta,
										PxReal& dmin, PxVec3& sep, PxU32& id, PxU32* PX_RESTRICT indices_, PxU32& numIndices,
										PxReal contactDistance
#ifdef TEST_INTERNAL_OBJECTS
										, const PxVec3& worldDelta
#endif
										)
{
	id = PX_INVALID_U32;
	PxU32* indices = indices_;

	const PxU32 num = polyData0.mNbPolygons;
	const PxVec3* PX_RESTRICT vertices = polyData0.mVerts;
	const Gu::HullPolygonData* PX_RESTRICT polygons = polyData0.mPolygons;

	//transform delta from hull0 shape into vertex space:
	const PxVec3 vertSpaceDelta = scaling0 % delta;

	// PT: prefetch polygon data
	{
		const PxU32 dataSize = num*sizeof(Gu::HullPolygonData);
		for(PxU32 offset=0; offset < dataSize; offset+=128)
			Ps::prefetchLine(polygons, offset);
	}

#ifdef _XBOX
	FloatInt* PX_RESTRICT planeDist = (FloatInt* PX_RESTRICT)PxAlloca(num*sizeof(FloatInt));
	for(PxU32 i=0; i < num; i++)
	{
		const Gu::HullPolygonData& P = polygons[i];
		const PxPlane& PL = P.mPlane;

		planeDist[i].f = PL.n.dot(vertSpaceDelta);
	}
#endif

	for(PxU32 i=0; i < num; i++)
	{
#ifdef _XBOX
		// Do backface-culling
		if(planeDist[i].i < 0)	// PT: integer code to avoid FCMPs
			continue;

		const Gu::HullPolygonData& P = polygons[i];
		const PxPlane& PL = P.mPlane;
#else
		const Gu::HullPolygonData& P = polygons[i];
		const PxPlane& PL = P.mPlane;

		// Do backface-culling
		if(PL.n.dot(vertSpaceDelta) < 0.0f)
			continue;
#endif

		//normals transform by inverse transpose: (and transpose(skew) == skew as its symmetric)
		PxVec3 shapeSpaceNormal = scaling0 % PL.n;
		//renormalize: (Arr!)
#ifdef USE_SIMD_NORMALIZE
		float invMagnitude;
		SIMD_Normalize(shapeSpaceNormal, invMagnitude);
#else
		const PxReal magnitude = shapeSpaceNormal.normalize();	// PT: We need to find a way to skip this normalize
#endif

#ifdef TEST_INTERNAL_OBJECTS

/*
const PxVec3 worldNormal_ = world0.rotate(shapeSpaceNormal);
PxReal d0;
bool test0 = PxcTestSeparatingAxis(polyData0, polyData1, world0, world1, scaling0, scaling1, worldNormal_, d0, contactDistance);

PxReal d1;
const float invMagnitude0 = 1.0f / magnitude;
bool test1 = PxcTestNormal(shapeSpaceNormal, P.getMin(vertices) * invMagnitude0, P.getMax() * invMagnitude0, polyData1, m1to0, scaling1, d1, contactDistance);

PxReal d2;
TestInternalObjects_(worldDelta, worldNormal_, polyData0, polyData1, world0, world1, d2, contactDistance);
*/

		const PxVec3 worldNormal = world0.rotate(shapeSpaceNormal);
		if(!TestInternalObjects(worldDelta, worldNormal, polyData0, polyData1, world0, world1, dmin, contactDistance))
		{
	#ifdef PX_DEBUG
			PxReal d;
		#ifndef USE_SIMD_NORMALIZE
			const float invMagnitude = 1.0f / magnitude;
		#endif
			if(testNormal(shapeSpaceNormal, P.getMin(vertices) * invMagnitude, P.getMax() * invMagnitude, polyData1, m1to0, scaling1, d, contactDistance))	//note how we scale scalars by skew magnitude change as we do plane d-s.
			{
				PX_ASSERT(d + testInternalObjectsEpsilon >= dmin);
			}
	#endif
			continue;
		}
#endif

		*indices++ = i;

		////////////////////
		/*
		//gotta transform minimum and maximum from vertex to shape space!
		//I think they transform like the 'd' of the plane, basically by magnitude division!
		//unfortunately I am not certain of that, so let's convert them to a point in vertex space, transform that, and then convert back to a plane d.

		//let's start by transforming the plane's d:
		//a point on the plane:

		PxVec3 vertSpaceDPoint = PL.normal * -PL.d;

		//make sure this is on the plane:
		PxReal distZero = PL.signedDistanceHessianNormalForm(vertSpaceDPoint);	//should be zero

		//transform: 

		PxVec3 shapeSpaceDPoint = cache.mVertex2ShapeSkew[skewIndex] * vertSpaceDPoint;

		//make into a d offset again by projecting along the plane:
		PxcPlane shapeSpacePlane(shapeSpaceNormal, shapeSpaceDPoint);

		//see what D is!!


		//NOTE: for boxes scale[0] is always id so this is all redundant.  Hopefully for convex convex it will become useful!
		*/

		////////////////////

		PxReal d;
#ifndef USE_SIMD_NORMALIZE
		const float invMagnitude = 1.0f / magnitude;
#endif
		if(!testNormal(shapeSpaceNormal, P.getMin(vertices) * invMagnitude, P.getMax() * invMagnitude, polyData1, m1to0, scaling1, d, contactDistance))	//note how we scale scalars by skew magnitude change as we do plane d-s.
			return false;

		if(d < dmin)
		{
#ifdef TEST_INTERNAL_OBJECTS
			sep = worldNormal;
#else
			sep = world0.rotate(shapeSpaceNormal);
#endif
			dmin = d;
			id = i;
		}
	}

	numIndices = (PxU32)(indices - indices_);

	PX_ASSERT(id!=PX_INVALID_U32); //Should never happen with this version

	return true;
}

// PT: isolating this piece of code allows us to better see what happens in PIX. For some reason it looks
// like isolating it creates *less* LHS than before, but I don't understand why. There's still a lot of
// bad stuff there anyway
//PX_FORCE_INLINE
static void prepareData(PxPlane& witnessPlane0, PxPlane& witnessPlane1,
						PxBounds3& aabb0, PxBounds3& aabb1,
						const PxBounds3& hullBounds0, const PxBounds3& hullBounds1,
						const PxPlane& vertSpacePlane0, const PxPlane& vertSpacePlane1,
						const Cm::FastVertex2ShapeScaling& scaling0, const Cm::FastVertex2ShapeScaling& scaling1,
						const Cm::Matrix34& m0to1, const Cm::Matrix34& m1to0,
						PxReal contactDistance
						)
{
	scaling0.transformPlaneToShapeSpace(vertSpacePlane0.n, vertSpacePlane0.d, witnessPlane0.n, witnessPlane0.d);
	scaling1.transformPlaneToShapeSpace(vertSpacePlane1.n, vertSpacePlane1.d, witnessPlane1.n, witnessPlane1.d);

	//witnessPlane0 = witnessPlane0.getTransformed(m0to1);
	//witnessPlane1 = witnessPlane1.getTransformed(m1to0);
	const PxVec3 newN0 = m0to1.rotate(witnessPlane0.n);
	witnessPlane0 = PxPlane(newN0, witnessPlane0.d - m0to1.base3.dot(newN0));
	const PxVec3 newN1 = m1to0.rotate(witnessPlane1.n);
	witnessPlane1 = PxPlane(newN1, witnessPlane1.d - m1to0.base3.dot(newN1));

	aabb0 = hullBounds0;
	aabb1 = hullBounds1;

	//gotta inflate	// PT: perfect LHS recipe here....
	const PxVec3 inflate(contactDistance);
	aabb0.minimum -= inflate;
	aabb1.minimum -= inflate;
	aabb0.maximum += inflate;
	aabb1.maximum += inflate;
}

static bool PxcBruteForceOverlapBackface(	const PxBounds3& hullBounds0, const PxBounds3& hullBounds1,
											const PolygonalData& polyData0, const PolygonalData& polyData1,
											const Cm::Matrix34& world0, const Cm::Matrix34& world1,
											const Cm::FastVertex2ShapeScaling& scaling0, const Cm::FastVertex2ShapeScaling& scaling1,
											const Cm::Matrix34& m0to1, const Cm::Matrix34& m1to0, const PxVec3& delta,
											PxU32& id0, PxU32& id1,
											PxReal& depth, PxVec3& sep, PxcSepAxisType& code, PxReal contactDistance)
{
	const PxVec3 localDelta0 = world0.rotateTranspose(delta);
	PxU32* PX_RESTRICT indices0  = (PxU32*)PxAlloca(polyData0.mNbPolygons*sizeof(PxU32));
	
	PxU32 numIndices0;
	PxReal dmin0 = PX_MAX_REAL;
	PxVec3 vec0;
	if(!PxcTestFacesSepAxesBackface(polyData0, polyData1, world0, world1, scaling0, scaling1, m1to0, localDelta0, dmin0, vec0, id0, indices0, numIndices0, contactDistance
#ifdef TEST_INTERNAL_OBJECTS
		, -delta
#endif
		))
		return false;

	const PxVec3 localDelta1 = world1.rotateTranspose(delta);

	PxU32* PX_RESTRICT indices1 = (PxU32*)PxAlloca(polyData1.mNbPolygons*sizeof(PxU32));

	PxU32 numIndices1;
	PxReal dmin1 = PX_MAX_REAL;
	PxVec3 vec1;
	if(!PxcTestFacesSepAxesBackface(polyData1, polyData0, world1, world0, scaling1, scaling0, m0to1, -localDelta1, dmin1, vec1, id1, indices1, numIndices1, contactDistance
#ifdef TEST_INTERNAL_OBJECTS
		, delta
#endif
		))
		return false;

	PxReal dmin = dmin0;
	PxVec3 vec = vec0;
	code = SA_NORMAL0;

	if(dmin1 < dmin)
	{
		dmin = dmin1;
		vec = vec1;
		code = SA_NORMAL1;
	}

	PX_ASSERT(id0!=PX_INVALID_U32);
	PX_ASSERT(id1!=PX_INVALID_U32);

	// Brute-force find a separating axis
	SeparatingAxes mSA0;
	SeparatingAxes mSA1;
	mSA0.reset();
	mSA1.reset();
	PxPlane witnessPlane0;
	PxPlane witnessPlane1;
	PxBounds3 aabb0, aabb1;

	prepareData(witnessPlane0, witnessPlane1,
				aabb0, aabb1,
				hullBounds0, hullBounds1,
				polyData0.mPolygons[id0].mPlane,
				polyData1.mPolygons[id1].mPlane,
				scaling0, scaling1,
				m0to1, m1to0,
				contactDistance);

	// Find possibly separating axes
	PxcFindSeparatingAxes(mSA0, indices0, numIndices0, polyData0, world0, witnessPlane1, m0to1, aabb1, contactDistance, scaling0);
	PxcFindSeparatingAxes(mSA1, indices1, numIndices1, polyData1, world1, witnessPlane0, m1to0, aabb0, contactDistance, scaling1);
//	PxcFindSeparatingAxes(context.mSA0, &id0, 1, hull0, world0, witnessPlane1, m0to1, aabbMin1, aabbMax1);
//	PxcFindSeparatingAxes(context.mSA1, &id1, 1, hull1, world1, witnessPlane0, m1to0, aabbMin0, aabbMax0);

	const PxU32 numEdges0 = mSA0.getNumAxes();
	const PxVec3* PX_RESTRICT edges0 = mSA0.getAxes();

	const PxU32 numEdges1 = mSA1.getNumAxes();
	const PxVec3* PX_RESTRICT edges1 = mSA1.getAxes();

	//	Worst case = convex test 02 with big meshes: 23 & 23 edges => 23*23 = 529 tests!
	//	printf("%d - %d\n", NbEdges0, NbEdges1);	// maximum = ~20 in test scenes.

#ifdef _XBOX
	const FloatV epsilon_4 = FEps6();

#endif

	for(PxU32 i=0; i < numEdges0; i++)
	{
#ifdef _XBOX
		const Vec3V edge0_4 = V3LoadUnsafeA(edges0[i]);
#else
		const PxVec3& edge0 = edges0[i];
#endif
		for(PxU32 j=0; j < numEdges1; j++)
		{
#ifdef _XBOX
			const Vec3V edge1_4 = V3LoadUnsafeA(edges1[j]);
#else
			const PxVec3& edge1 = edges1[j];
#endif

#ifdef _XBOX
			const Vec3V sepAxis_4 = V3Cross(edge0_4, edge1_4);
			if(V3AllGrtr(epsilon_4, V3Abs(sepAxis_4)))
				continue;


			PxVec3 sepAxis;
			V3StoreU(V3NormalizeSafe(sepAxis_4), sepAxis);

			{
#else
			PxVec3 sepAxis = edge0.cross(edge1);
			if(!Ps::isAlmostZero(sepAxis))
			{
				sepAxis = sepAxis.getNormalized();
#endif

#ifdef TEST_INTERNAL_OBJECTS
				if(!TestInternalObjects(-delta, sepAxis, polyData0, polyData1, world0, world1, dmin, contactDistance))
				{
	#ifdef PX_DEBUG
					PxReal d;
					if(testSeparatingAxis(polyData0, polyData1, world0, world1, scaling0, scaling1, sepAxis, d, contactDistance))
					{
						PX_ASSERT(d + testInternalObjectsEpsilon >= dmin);
					}
	#endif
					continue;
				}
#endif

				PxReal d;
				if(!testSeparatingAxis(polyData0, polyData1, world0, world1, scaling0, scaling1, sepAxis, d, contactDistance))
					return false;

				if(d<dmin)
				{
					dmin = d;
					vec = sepAxis;
					code = SA_EE;
				}
			}
		}
	}

	depth = dmin;
	sep = vec;

	return true;
}

#ifdef PX_VMX
// PT: TODO: the VMX code is useless here, if it doesn't get rid of vcmps
#include "PsVecMath.h"
using namespace Ps::aos;
#endif

static bool PxcTestFacesSepAxesBackfaceRoughPass(
										 const PolygonalData& polyData0, const PolygonalData& polyData1,
										 const Cm::Matrix34& world0, const Cm::Matrix34& world1,
										 const Cm::FastVertex2ShapeScaling& scaling0, const Cm::FastVertex2ShapeScaling& scaling1,
										 const Cm::Matrix34& m1to0, const PxVec3& /*witness*/, const PxVec3& delta,
										 PxReal& dmin, PxVec3& sep, PxU32& id,
										 PxReal contactDistance
#ifdef TEST_INTERNAL_OBJECTS
										, const PxVec3& worldDelta
#endif
										 )
{
	id = PX_INVALID_U32;

	const PxU32 num = polyData0.mNbPolygons;
	const Gu::HullPolygonData* PX_RESTRICT polygons = polyData0.mPolygons;
	const PxVec3* PX_RESTRICT vertices = polyData0.mVerts;

	//transform delta from hull0 shape into vertex space:
	const PxVec3 vertSpaceDelta = scaling0 % delta;  //todo: VMX

#ifdef PX_VMX
	FloatV zero = FZero();

	Vec3V vertSpaceDelta4 = V3LoadU(vertSpaceDelta);
#endif

#ifdef _XBOX
	// PT: prefetch polygon data
	{
		const PxU32 dataSize = num*sizeof(Gu::HullPolygonData);
		for(PxU32 offset=0; offset < dataSize; offset+=128)
			Ps::prefetchLine(polygons, offset);
	}
#endif


	for(PxU32 i=0; i < num; i++)
	{
		const Gu::HullPolygonData& P = polygons[i];
		const PxPlane& PL = P.mPlane;

#ifdef PX_VMX
		Vec3V v_pl = V3LoadU(PL.n);
		FloatV dot = V3Dot(v_pl, vertSpaceDelta4);

		if(FAllGrtr(zero, dot))	// PT: TODO: check this doesn't have vcmps ... because we can do better...
			continue;
#else
		if(PL.n.dot(vertSpaceDelta) < 0.0f)
			continue; //backface-cull
#endif

		PxVec3 shapeSpaceNormal = scaling0 % PL.n;	//normals transform with inverse transpose
		//renormalize: (Arr!)
#ifdef USE_SIMD_NORMALIZE
		float invMagnitude;
		SIMD_Normalize(shapeSpaceNormal, invMagnitude);
#else
		const PxReal magnitude = shapeSpaceNormal.normalize();	// PT: We need to find a way to skip this normalize
#endif

#ifdef TEST_INTERNAL_OBJECTS
		const PxVec3 worldNormal = world0.rotate(shapeSpaceNormal);
		if(!TestInternalObjects(worldDelta, worldNormal, polyData0, polyData1, world0, world1, dmin, contactDistance))
		{
	#ifdef PX_DEBUG
			PxReal d;
#ifndef USE_SIMD_NORMALIZE
			const float invMagnitude = 1.0f / magnitude;
#endif
			if(testNormal(shapeSpaceNormal, P.getMin(vertices) * invMagnitude, P.getMax() * invMagnitude, /*hull1,*/ polyData1, m1to0, scaling1, d, contactDistance))	//note how we scale scalars by skew magnitude change as we do plane d-s.
			{
				PX_ASSERT(d + testInternalObjectsEpsilon >= dmin);
			}
	#endif
			continue;
		}
#endif

		PxReal d;
#ifndef USE_SIMD_NORMALIZE
		const float invMagnitude = 1.0f / magnitude;
#endif
		if(!testNormal(shapeSpaceNormal, P.getMin(vertices) * invMagnitude, P.getMax() * invMagnitude, /*hull1,*/ polyData1, m1to0, scaling1, d, contactDistance))	//note how we scale scalars by skew magnitude change as we do plane d-s.
			return false;

		if(d < dmin)
		{
#ifdef TEST_INTERNAL_OBJECTS
			sep = worldNormal;
#else
			sep = world0.rotate(shapeSpaceNormal);
#endif
			dmin = d;
			id = i;
		}
	}

	PX_ASSERT(id!=PX_INVALID_U32); //Should never happen with this version

	return true;
}

static bool PxcBruteForceOverlapBackfaceRoughPass(	const PolygonalData& polyData0, const PolygonalData& polyData1,
													const Cm::Matrix34& world0, const Cm::Matrix34& world1,
													const Cm::FastVertex2ShapeScaling& scaling0, const Cm::FastVertex2ShapeScaling& scaling1,
													const Cm::Matrix34& m0to1, const Cm::Matrix34& m1to0, const PxVec3& delta,
													PxU32& id0, PxU32& id1,
													PxReal& depth, PxVec3& sep, PxcSepAxisType& code, PxReal contactDistance)
{
	PxReal dmin0 = PX_MAX_REAL;
	PxReal dmin1 = PX_MAX_REAL;
	PxVec3 vec0, vec1;

	const PxVec3 localDelta0 = world0.rotateTranspose(delta);
	const PxVec3 localCenter1in0 = m1to0.transform(polyData1.mCenter);
	if(!PxcTestFacesSepAxesBackfaceRoughPass(polyData0, polyData1, world0, world1, scaling0, scaling1, m1to0, localCenter1in0, localDelta0, dmin0, vec0, id0, contactDistance
#ifdef TEST_INTERNAL_OBJECTS
		, -delta
#endif
		))
		return false;

	const PxVec3 localDelta1 = world1.rotateTranspose(delta);
	const PxVec3 localCenter0in1 = m0to1.transform(polyData0.mCenter);
	if(!PxcTestFacesSepAxesBackfaceRoughPass(polyData1, polyData0, world1, world0, scaling1, scaling0, m0to1, localCenter0in1, -localDelta1, dmin1, vec1, id1, contactDistance
#ifdef TEST_INTERNAL_OBJECTS
		, delta
#endif
		))
		return false;

	PxReal dmin = dmin0;
	PxVec3 vec = vec0;
	code = SA_NORMAL0;

	if(dmin1 < dmin)
	{
		dmin = dmin1;
		vec = vec1;
		code = SA_NORMAL1;
	}

	PX_ASSERT(id0!=PX_INVALID_U32);
	PX_ASSERT(id1!=PX_INVALID_U32);

	depth = dmin;
	sep = vec;

	return true;
}


static bool PxcContactHullHull(	const PolygonalData& polyData0, const PolygonalData& polyData1,
								const PxBounds3& hullBounds0, const PxBounds3& hullBounds1,
								const PxTransform& transform0, const PxTransform& transform1,
								PxReal contactDistance, ContactBuffer& contactBuffer,
								const Cm::FastVertex2ShapeScaling& scaling0, const Cm::FastVertex2ShapeScaling& scaling1,
								bool idtScale0, bool idtScale1);

// Box-convex contact generation
// this can no longer share code with convex-convex because that case needs scaling for both shapes, while this only needs it for one, which may be significant perf wise.
//
// PT: duplicating the full convex-vs-convex codepath is a lot worse. Look at it this way: if scaling is really "significant perf wise" then we made the whole convex-convex
// codepath significantly slower, and this is a lot more important than just box-vs-convex. The proper approach is to share the code and make sure scaling is NOT a perf hit.
// PT: please leave this function in the same translation unit as PxcContactHullHull.

namespace physx
{
bool PxcContactBoxConvex(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	const PxBoxGeometry& shapeBox = shape0.get<const PxBoxGeometry>();

	Cm::FastVertex2ShapeScaling idtScaling;

	const PxBounds3 boxBounds(-shapeBox.halfExtents, shapeBox.halfExtents);

	PolygonalData polyData0;
	PolygonalBox polyBox(shapeBox.halfExtents);
	polyBox.getPolygonalData(&polyData0);

	///////

	Cm::FastVertex2ShapeScaling convexScaling;
	PxBounds3 convexBounds;
	PolygonalData polyData1;
	const bool idtScale = getConvexData(shape1, convexScaling, convexBounds, polyData1);

	return PxcContactHullHull(	polyData0, polyData1, boxBounds, convexBounds,
								transform0, transform1, contactDistance, contactBuffer,
								idtScaling, convexScaling, true, idtScale);
}

// PT: please leave this function in the same translation unit as PxcContactHullHull.
bool PxcContactConvexConvex(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	Cm::FastVertex2ShapeScaling scaling0, scaling1;
	PxBounds3 convexBounds0, convexBounds1;
	PolygonalData polyData0, polyData1;
	const bool idtScale0 = getConvexData(shape0, scaling0, convexBounds0, polyData0);
	const bool idtScale1 = getConvexData(shape1, scaling1, convexBounds1, polyData1);

	return PxcContactHullHull(	polyData0, polyData1, convexBounds0, convexBounds1,
								transform0, transform1, contactDistance, contactBuffer,
								scaling0, scaling1, idtScale0, idtScale1);
}
}

static bool PxcContactHullHull(	const PolygonalData& polyData0, const PolygonalData& polyData1,
								const PxBounds3& hullBounds0, const PxBounds3& hullBounds1,
								const PxTransform& transform0, const PxTransform& transform1,
								PxReal contactDistance, ContactBuffer& contactBuffer,
								const Cm::FastVertex2ShapeScaling& scaling0, const Cm::FastVertex2ShapeScaling& scaling1,
								bool idtScale0, bool idtScale1)
{
	// PART 1 - ~5ms/19 for all that junk...

	////////////////////

	// Compute matrices
	const Cm::Matrix34 world0(transform0);
	const Cm::Matrix34 world1(transform1);

	const PxVec3 worldCenter0 = world0.transform(polyData0.mCenter);
	const PxVec3 worldCenter1 = world1.transform(polyData1.mCenter);
	
	const PxVec3 deltaC = worldCenter1 - worldCenter0;

	PxReal depth;

	// PT: prefetch vertices
	// The test is needed until the box implements this function as well
	if(polyData0.mPrefetchHull)	(polyData0.mPrefetchHull)(polyData0.mNbVerts, polyData0.mVerts);
	if(polyData1.mPrefetchHull)	(polyData1.mPrefetchHull)(polyData1.mNbVerts, polyData1.mVerts);

	///////////////////////////////////////////////////////////////////////////

	// Early-exit test: quickly discard obviously non-colliding cases.
	// PT: we used to skip this when objects were touching, which provided a small speedup (saving 2 full hull projections).
	// We may want to add this back at some point in the future, if possible.
	if(true) //AM: now we definitely want to test the cached axis to get the depth value for the cached axis, even if we had a contact before.
	{
		if(!testSeparatingAxis(polyData0, polyData1, world0, world1, scaling0, scaling1, deltaC, depth, contactDistance))
		//there was no contact previously and we reject using the same prev. axis.
			return false;
	}

	///////////////////////////////////////////////////////////////////////////

	// Compute relative transforms
	PxTransform t0to1 = transform1.transformInv(transform0);
	PxTransform t1to0 = transform0.transformInv(transform1);

	PxU32 c0(0x7FFF);
	PxU32 c1(0x7FFF);
	PxVec3 worldNormal;

	const Cm::Matrix34 m0to1(t0to1);
	const Cm::Matrix34 m1to0(t1to0);

	// PART 2 - ~5 or 6ms/19 for the actual meat of the algo

#ifdef CONVEX_CONVEX_ROUGH_FIRST_PASS
	// PT: it is a bad idea to skip the rough pass, for two reasons:
	// 1) performance. The rough pass is obviously a lot faster.
	// 2) stability. The "skipIt" optimization relies on the rough pass being present to catch the cases where it fails. If you disable the rough pass
	// "skipIt" can skip the whole work, contacts get lost, and we never "try again" ==> explosions
//	bool TryRoughPass = (contactDistance == 0.0f);	//Rough first pass doesn't work with dist based for some reason.
    for(int TryRoughPass = 1 /*gEnableOptims*/; 0 <= TryRoughPass; --TryRoughPass)
    {
#endif

	{
		PxcSepAxisType code;
		PxU32 id0, id1;
		//PxReal depth;
#ifdef CONVEX_CONVEX_ROUGH_FIRST_PASS
		bool Status;
		if(TryRoughPass)
		{
			// ~6ms/19
			Status = PxcBruteForceOverlapBackfaceRoughPass(polyData0, polyData1, world0, world1, scaling0, scaling1, m0to1, m1to0, deltaC, id0, id1, depth, worldNormal, code, contactDistance);
		}
		else
		{
			// ~5ms/19 ??
			Status = PxcBruteForceOverlapBackface(
//				hull0, hull1,
				hullBounds0, hullBounds1,
				polyData0, polyData1, world0, world1, scaling0, scaling1, m0to1, m1to0, deltaC, id0, id1, depth, worldNormal, code, contactDistance);
		}

		if(!Status)
#else
		if(!PxcBruteForceOverlapBackface(
//			hull0, hull1,
			hullBounds0, hullBounds1,
			polyData0, polyData1,
			world0, world1, scaling0, scaling1, m0to1, m1to0, deltaC, id0, id1, depth, worldNormal, code, contactDistance))
#endif
			return false;

		if(deltaC.dot(worldNormal) < 0.0f)
			worldNormal = -worldNormal;

/*
worldNormal = -partialSep;
depth = -partialDepth;
code = SA_EE;
*/

	// PART 3 - ~3ms/19 to find best polygons

		// Takes ~3ms/19
		if(code==SA_NORMAL0)
		{
			c0 = id0;
			c1 = (polyData1.mSelectClosestEdgeCB)(polyData1, scaling1, world1.rotateTranspose(-worldNormal));
		}
		else if(code==SA_NORMAL1)
		{
			c0 = (polyData0.mSelectClosestEdgeCB)(polyData0, scaling0, world0.rotateTranspose(worldNormal));
			c1 = id1;
		}
		else if(code==SA_EE)
		{
			c0 = (polyData0.mSelectClosestEdgeCB)(polyData0, scaling0, world0.rotateTranspose(worldNormal));
			c1 = (polyData1.mSelectClosestEdgeCB)(polyData1, scaling1, world1.rotateTranspose(-worldNormal));
		}
	}

	const Gu::HullPolygonData& HP0 = polyData0.mPolygons[c0];
	const Gu::HullPolygonData& HP1 = polyData1.mPolygons[c1];
	// PT: prefetching those guys saves ~600.000 cycles in convex-convex benchmark
	Ps::prefetchLine(&HP0.mPlane);
	Ps::prefetchLine(&HP1.mPlane);

	//ok, we have a new depth value. convert to real distance. 
//	PxReal separation = -depth;		//depth was either computed in initial cached-axis check, or better, when skipIt was false, in sep axis search.
//	if (separation < 0.0f)
//		separation = 0.0f;	//we don't want to store penetration values.
	const PxReal separation = physx::intrinsics::fsel(depth, 0.0f, -depth);

	PxVec3 worldNormal0;
	PxPlane shapeSpacePlane0;
	if(idtScale0)
	{
		copyPlane(&shapeSpacePlane0, &HP0);
		worldNormal0 = world0.rotate(HP0.mPlane.n);
	}
	else
	{
		scaling0.transformPlaneToShapeSpace(HP0.mPlane.n,HP0.mPlane.d,shapeSpacePlane0.n,shapeSpacePlane0.d);
		worldNormal0 = world0.rotate(shapeSpacePlane0.n);
	}

	PxVec3 worldNormal1;
	PxPlane shapeSpacePlane1;
	if(idtScale1)
	{
		copyPlane(&shapeSpacePlane1, &HP1);
		worldNormal1 = world1.rotate(HP1.mPlane.n);
	}
	else
	{
		scaling1.transformPlaneToShapeSpace(HP1.mPlane.n,HP1.mPlane.d,shapeSpacePlane1.n,shapeSpacePlane1.d);
		worldNormal1 = world1.rotate(shapeSpacePlane1.n);
	}

	Ps::IntBool flag;
	{
		const PxReal d0 = PxAbs(worldNormal0.dot(worldNormal));
		const PxReal d1 = PxAbs(worldNormal1.dot(worldNormal));
		bool f = d0>d1; //which face normal is the separating axis closest to.
		flag = f;
	}

////////////////////NEW DIST HANDLING//////////////////////
//TODO: skip this if there is no dist involved!
	//printf("\nseparation = %f", separation);
		PX_ASSERT(separation >= 0.0f);	//be sure this got fetched somewhere in the chaos above.
//		separation = - depth;	//convert to real distance. 
//		if (separation < 0.0f)
//			separation = 0.0f;	//don't do anything when penetrating!

		const PxReal cCCDEpsilon = contactBuffer.meshContactMargin;
		const PxReal contactGenPositionShift = separation + cCCDEpsilon;	//if we're at a distance, shift so we're within SKIN_COMPENSATOR_DISTANCE penetration.

		const PxVec3 contactGenPositionShiftVec = worldNormal * -contactGenPositionShift;	//shift one of the bodies this distance toward the other just for Pierre's contact generation.  Then the bodies should be penetrating exactly by MIN_SEPARATION_FOR_PENALTY - ideal conditions for this contact generator.

		//note: for some reason this has to change sign!

		//this will make contact gen always generate contacts at about MSP.  Shift them back to the true real distance, and then to a solver compliant distance given that 
		//the solver converges to MSP penetration, while we want it to converge to 0 penetration.
		//to real distance:
//		const PxReal polyPolySeparationShift = separation; //(+ or - depending on which way normal goes)

		//The system:  We always shift convex 0 (arbitrary).  If the contact is attached to convex 0 then we will need to shift the contact point, otherwise not.
		const PxVec3 newBase3 = world0.base3 - contactGenPositionShiftVec;
		const Cm::Matrix34 world0_Tweaked(world0.base0, world0.base1, world0.base2, newBase3);
		PxTransform shifted0(newBase3, transform0.q);

		t0to1 = transform1.transformInv(shifted0);
		t1to0 = shifted0.transformInv(transform1);
		Cm::Matrix34 m0to1_Tweaked;
		Cm::Matrix34 m1to0_Tweaked;
		m0to1_Tweaked.set(t0to1.q);		m0to1_Tweaked.base3 = t0to1.p;
		m1to0_Tweaked.set(t1to0.q);		m1to0_Tweaked.base3 = t1to0.p;

//////////////////////////////////////////////////
	//pretransform convex polygon if we have scaling!
	PxVec3* scaledVertices0;
	PxU8* stackIndices0;
	GET_SCALEX_CONVEX(scaledVertices0, stackIndices0, idtScale0, HP0.mNbVerts, scaling0, polyData0.mVerts, polyData0.getPolygonVertexRefs(HP0))

	//pretransform convex polygon if we have scaling!
	PxVec3* scaledVertices1;
	PxU8* stackIndices1;
	GET_SCALEX_CONVEX(scaledVertices1, stackIndices1, idtScale1, HP1.mNbVerts, scaling1, polyData1.mVerts, polyData1.getPolygonVertexRefs(HP1))

	// PART 4 - ~3ms/19 to generate contacts
	// So we need to switch:
	// - HP0, HP1
	// - scaledVertices0, scaledVertices1
	// - stackIndices0, stackIndices1
	// - world0, world1
	// - shapeSpacePlane0, shapeSpacePlane1
	// - worldNormal0, worldNormal1
	// - m0to1, m1to0
	// - true, false
	const PxMat33 RotT0 = findRotationMatrixFromZ(shapeSpacePlane0.n);
	const PxMat33 RotT1 = findRotationMatrixFromZ(shapeSpacePlane1.n);

	if(flag)
	{
		if(contactPolygonPolygonExt(HP0.mNbVerts, scaledVertices0, stackIndices0, world0_Tweaked, shapeSpacePlane0, RotT0,
									HP1.mNbVerts, scaledVertices1, stackIndices1, world1, shapeSpacePlane1, RotT1,
									worldNormal0, m0to1_Tweaked, m1to0_Tweaked,
									PXC_CONTACT_NO_FACE_INDEX, PXC_CONTACT_NO_FACE_INDEX,
									contactBuffer,
									true, contactGenPositionShiftVec, contactGenPositionShift))
			return true;
	}
	else
	{
		if(contactPolygonPolygonExt(HP1.mNbVerts, scaledVertices1, stackIndices1, world1, shapeSpacePlane1, RotT1,
									HP0.mNbVerts, scaledVertices0, stackIndices0, world0_Tweaked, shapeSpacePlane0, RotT0,
									worldNormal1, m1to0_Tweaked, m0to1_Tweaked,
									PXC_CONTACT_NO_FACE_INDEX, PXC_CONTACT_NO_FACE_INDEX,
									contactBuffer,
									false, contactGenPositionShiftVec, contactGenPositionShift))
			return true;
	}

#ifdef CONVEX_CONVEX_ROUGH_FIRST_PASS
    }
#endif
	return false;
}
