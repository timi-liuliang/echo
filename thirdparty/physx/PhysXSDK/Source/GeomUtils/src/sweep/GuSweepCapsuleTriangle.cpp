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

#include "GuSweepCapsuleTriangle.h"
#include "GuCapsule.h"
#include "PxTriangle.h"
#include "GuDistanceSegmentTriangle.h"
#include "GuDistanceSegmentTriangleSIMD.h"
#include "GuIntersectionTriangleBox.h"
#include "GuSweepSphereTriangle.h"
#include "GuSweepTriangleUtils.h"

using namespace physx;
using namespace Gu;
using namespace physx::shdfnd::aos;

#ifdef __SPU__
// AP: function version to reduce SPU code size
void sweepCapsuleTrianglesOutputTri2(
	const PxVec3& p0, const PxVec3& p1, const PxVec3& p2, const PxVec3& d, PxTriangle* extrudedTris,
	PxU32& nbExtrudedTris, PxVec3* extrudedTrisNormals
	)
{
	PxTriangle& t = extrudedTris[nbExtrudedTris];
	t.verts[0] = p0;
	t.verts[1] = p1;
	t.verts[2] = p2;
	PxVec3 nrm;
	t.denormalizedNormal(nrm);
	if(nrm.dot(d)>0.0f)
	{
		PxVec3 tmp = t.verts[1];
		t.verts[1] = t.verts[2];
		t.verts[2] = tmp;
		nrm = -nrm;
	}
	extrudedTrisNormals[nbExtrudedTris] = nrm;
	nbExtrudedTris++;
}
#define _OUTPUT_TRI2(p0, p1, p2, d) sweepCapsuleTrianglesOutputTri2(p0, p1, p2, d, extrudedTris, nbExtrudedTris, extrudedTrisNormals);
#else
#define _OUTPUT_TRI2(p0, p1, p2, d){			\
PxTriangle& tri = extrudedTris[nbExtrudedTris];	\
tri.verts[0] = p0;								\
tri.verts[1] = p1;								\
tri.verts[2] = p2;								\
PxVec3 nrm;										\
tri.denormalizedNormal(nrm);					\
if(nrm.dot(d)>0.0f) {							\
PxVec3 tmp = tri.verts[1];						\
tri.verts[1] = tri.verts[2];					\
tri.verts[2] = tmp;								\
nrm = -nrm;										\
}												\
extrudedTrisNormals[nbExtrudedTris] = nrm;		\
nbExtrudedTris++; }
#endif

bool Gu::sweepCapsuleTriangles(	PxU32 nbTris, const PxTriangle* PX_RESTRICT triangles,							// Triangle data
								const Capsule& capsule,															// Capsule data
								const PxVec3& unitDir, const PxReal distance,									// Ray data
								const PxU32* PX_RESTRICT cachedIndex,											// Cache data
								PxHitFlags& outFlags, PxF32& t, PxVec3& normal, PxVec3& hit, PxU32& hitIndex,	// Results
								PxVec3& _triNormal,
								PxHitFlags hintFlags, bool isDoubleSided,										// Query modifiers
								const Box* cullBox)																// Cull data
{
	if(!nbTris)
		return false;

	const bool meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;
	const bool doBackfaceCulling = !isDoubleSided && !meshBothSides;

	const PxVec3 capsuleCenter = capsule.computeCenter();

	if(!(hintFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP))
	{
		// PT: test if shapes initially overlap
		const PxVec3 segmentExtent = capsule.p1 - capsule.p0;
		const PxReal r2 = capsule.radius*capsule.radius;
		for(PxU32 i=0;i<nbTris;i++)
		{
			// PT: add culling here for now, but could be made more efficiently...
			const PxTriangle& currentSrcTri = triangles[i];	// PT: src tri, i.e. non-extruded

			// Create triangle normal
			PxVec3 denormalizedNormal;
			currentSrcTri.denormalizedNormal(denormalizedNormal);

			// Backface culling
			if(doBackfaceCulling && (denormalizedNormal.dot(unitDir) > 0.0f))
				continue;

			const PxVec3& p0 = triangles[i].verts[0];
			const PxVec3& p1 = triangles[i].verts[1];
			const PxVec3& p2 = triangles[i].verts[2];

#ifdef __SPU__
			Vec3V dummy1, dummy2;
			FloatV result = distanceSegmentTriangleSquared(
				V3LoadU(capsule.p0), V3LoadU(capsule.p1), V3LoadU(p0), V3LoadU(p1), V3LoadU(p2), dummy1, dummy2);
			PxReal dist2 = FStore(result);
#else
			// AP: switching to SIMD version produced -25% regression in Sweep*SphereHfld
			PxReal dist2 = distanceSegmentTriangleSquared(capsule.p0, segmentExtent, p0, p1 - p0, p2 - p0);
#endif
			if (dist2<=r2)
			{
				hitIndex	= i;
				t			= 0.0f;
				normal		= -unitDir;
				outFlags	= PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL;
				_triNormal	= denormalizedNormal.getNormalized();
				return true;
			}
		}
	}

	// The nice thing with this approach is that we "just" fallback to already existing code

	// PT: we can fallback to sphere sweep:
	// - if the capsule is degenerate (i.e. it's a sphere)
	// - if the sweep direction is the same as the capsule axis, in which case we can just sweep the top or bottom sphere

	const PxVec3 extrusionDir = (capsule.p0 - capsule.p1)*0.5f;	// Extrusion dir = capsule segment
	const PxReal halfHeight = extrusionDir.magnitude();
	bool mustExtrude = halfHeight!=0.0f;
	if(mustExtrude)
	{
		const PxVec3 capsuleAxis = extrusionDir/halfHeight;
		const PxReal colinearity = PxAbs(capsuleAxis.dot(unitDir));
		mustExtrude = (colinearity < (1.0f - LOCAL_EPSILON));
	}

	if(!mustExtrude)
	{
		const PxVec3 sphereCenter = capsuleCenter + unitDir * halfHeight;
		return sweepSphereTriangles(nbTris, triangles, sphereCenter, capsule.radius, unitDir, distance, cachedIndex, hit, normal, t, hitIndex, outFlags, _triNormal, isDoubleSided, meshBothSides);
	}

	// PT: extrude mesh on the fly. This is a modified copy of sweepSphereTriangles, unfortunately
	PxTriangle extrudedTris[7];
	PxVec3 extrudedTrisNormals[7];	// Not normalized

	hitIndex = PX_INVALID_U32;
	const PxU32 initIndex = getInitIndex(cachedIndex, nbTris);

	const PxReal radius = capsule.radius;
	PxReal curT = distance;
	const PxReal dpc0 = capsuleCenter.dot(unitDir);

	// PT: we will copy the best triangle here. Using indices alone doesn't work
	// since we extrude on-the-fly (and we don't want to re-extrude later)
	PxTriangle bestTri;
	PxVec3 bestTriNormal(0.0f);
	PxReal mostOpposingHitDot = 2.0f;

	for(PxU32 ii=0; ii<nbTris; ii++)	// We need i for returned triangle index
	{
		const PxU32 i = getTriangleIndex(ii, initIndex);

		const PxTriangle& currentSrcTri = triangles[i];	// PT: src tri, i.e. non-extruded

///////////// PT: this part comes from "ExtrudeMesh"
		// Create triangle normal
		PxVec3 denormalizedNormal;
		currentSrcTri.denormalizedNormal(denormalizedNormal);

		// Backface culling
		if(doBackfaceCulling && (denormalizedNormal.dot(unitDir) > 0.0f))
			continue;

		if(cullBox)
		{
			const PxVec3 tmp0 = cullBox->rotateInv(currentSrcTri.verts[0] - cullBox->center);
			const PxVec3 tmp1 = cullBox->rotateInv(currentSrcTri.verts[1] - cullBox->center);
			const PxVec3 tmp2 = cullBox->rotateInv(currentSrcTri.verts[2] - cullBox->center);
			const PxVec3 center(0.0f);
			if(!intersectTriangleBox(center, cullBox->extents, tmp0, tmp1, tmp2))
				continue;
		}

		// Extrude mesh on the fly
		PxU32 nbExtrudedTris=0;

		PxVec3 p0 = currentSrcTri.verts[0];
		PxVec3 p1 = currentSrcTri.verts[1];
		PxVec3 p2 = currentSrcTri.verts[2];

		PxVec3 p0b = p0 + extrusionDir;
		PxVec3 p1b = p1 + extrusionDir;
		PxVec3 p2b = p2 + extrusionDir;

		p0 -= extrusionDir;
		p1 -= extrusionDir;
		p2 -= extrusionDir;

#define _OUTPUT_TRI(p0, p1, p2){														\
extrudedTris[nbExtrudedTris].verts[0] = p0;												\
extrudedTris[nbExtrudedTris].verts[1] = p1;												\
extrudedTris[nbExtrudedTris].verts[2] = p2;												\
extrudedTris[nbExtrudedTris].denormalizedNormal(extrudedTrisNormals[nbExtrudedTris]);	\
nbExtrudedTris++;}

		if(denormalizedNormal.dot(extrusionDir) >= 0.0f)	_OUTPUT_TRI(p0b, p1b, p2b)
		else												_OUTPUT_TRI(p0, p1, p2)

		// ### it's probably useless to extrude all the shared edges !!!!!
		//if(CurrentFlags & TriangleCollisionFlag::eACTIVE_EDGE12)
		{
			_OUTPUT_TRI2(p1, p1b, p2b, unitDir)
			_OUTPUT_TRI2(p1, p2b, p2, unitDir)
		}
		//if(CurrentFlags & TriangleCollisionFlag::eACTIVE_EDGE20)
		{
			_OUTPUT_TRI2(p0, p2, p2b, unitDir)
			_OUTPUT_TRI2(p0, p2b, p0b, unitDir)
		}
		//if(CurrentFlags & TriangleCollisionFlag::eACTIVE_EDGE01)
		{
			_OUTPUT_TRI2(p0b, p1b, p1, unitDir)
			_OUTPUT_TRI2(p0b, p1, p0, unitDir)
		}
/////////////

		// PT: TODO: this one is new, to fix the tweak issue. However this wasn't
		// here before so the perf hit should be analyzed.
		denormalizedNormal.normalize();
		const PxReal hitDot1 = computeAlignmentValue(denormalizedNormal, unitDir);

		for(PxU32 j=0;j<nbExtrudedTris;j++)
		{
			const PxTriangle& currentTri = extrudedTris[j];

			PxVec3& triNormal = extrudedTrisNormals[j];
			// Backface culling
			if(doBackfaceCulling && (triNormal.dot(unitDir)) > 0.0f)
				continue;

			// PT: beware, culling is only ok on the sphere I think
			if(rejectTriangle(capsuleCenter, unitDir, curT, radius, currentTri, dpc0))
				continue;

			const PxReal magnitude = triNormal.magnitude();
			if(magnitude==0.0f)
				continue;

			triNormal /= magnitude;

			PxReal currentDistance;
			if(!sweepTriSphere(currentTri, triNormal, capsuleCenter, radius, unitDir, currentDistance))
				continue;

			const PxReal distEpsilon = GU_EPSILON_SAME_DISTANCE; // pick a farther hit within distEpsilon that is more opposing than the previous closest hit
			if(!keepTriangle(currentDistance, hitDot1, curT, mostOpposingHitDot, distEpsilon))
				continue;

			curT = currentDistance;
			hitIndex = i;
			mostOpposingHitDot = hitDot1; // arbitrary bias. works for hitDot1=-1, prevHitDot=0
			bestTri = currentTri;
			bestTriNormal = denormalizedNormal;
		}
	}

	if(hitIndex==PX_INVALID_U32)
		return false;	// We didn't touch any triangle

	t = curT;

	// Compute impact data only once, using best triangle
	computeSphereTriImpactData(hit, normal, capsuleCenter, unitDir, curT, bestTri);

	// PT: by design, returned normal is opposed to the sweep direction.
	if(shouldFlipNormal(normal, meshBothSides, isDoubleSided, bestTriNormal, unitDir))
		normal = -normal;

	_triNormal = bestTriNormal;

	// PT: revisit this
	if(hitIndex!=PX_INVALID_U32)
	{
//		hitIndex = Ids[hitIndex];

		// PT: deadline in a few hours. No time. Should be cleaned later or re-thought.
		// PT: we need to recompute a hit here because the hit between the *capsule* and the source mesh can be very
		// different from the hit between the *sphere* and the extruded mesh.

		// Touched tri
		const PxVec3& p0 = triangles[hitIndex].verts[0];
		const PxVec3& p1 = triangles[hitIndex].verts[1];
		const PxVec3& p2 = triangles[hitIndex].verts[2];

		// AP: measured to be a bit faster than the scalar version
		Vec3V pointOnSeg, pointOnTri;
		distanceSegmentTriangleSquared(
			V3LoadU(capsule.p0 + unitDir*t), V3LoadU(capsule.p1 + unitDir*curT),
			V3LoadU(p0), V3LoadU(p1), V3LoadU(p2),
			pointOnSeg, pointOnTri);
		V3StoreU(pointOnTri, hit);

		outFlags = PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL|PxHitFlag::ePOSITION;
	}
	return true;
}
