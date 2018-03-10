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


#include "PxsFluidCollisionMethods.h"
#include "PxsFluidConstants.h"
#include "PxcHeightFieldAabbTest.h"
#include "CmScaling.h"
#include "GuTriangleVertexPointers.h"
#include "GuMidphase.h"

using namespace physx;

//
// Collide particle against mesh triangle
//
// Project particle on triangle plane, check if projected particle is inside triangle
// using barycentric coordinates.
//																			//
//                 q2														//
//                 *														//
//               /   \														//
//              /      \													//
//             /         \													//
//            /            \												//
//        q0 *--------------* q1											//
//
// Triangle with points q0, q1, q2.
//
// Point p on plane defined by triangle:
//
//     p = q0 + (u * (q1 - q0)) + (v * (q2 - q0))
//       = q0 + (u * e0) + (v * e1)
//
// ->  (p - q0) = (u * e0) + (v * e1)	// Subtract q0 from both sides
//           e2 = (u * e0) + (v * e1)
//
// We have two unknowns (u and v) so we need two equations to solve for them. Dot both sides by e0 to get one and dot both sides by
// e1 to get a second.
//
//     e2 . e0 = ((u * e0) + (v * e1)) . e0		(1)
//     e2 . e1 = ((u * e0) + (v * e1)) . e1		(2)
//
// Distribute e0 and e1
//
//     e2 . e0 = u * (e0 . e0) + v * (e1 . e0)	(1)
//     e2 . e1 = u * (e0 . e1) + v * (e1 . e1)	(2)
//
// Solve vor u, v
//
//     u = ((e1.e1)(e0.e2) - (e0.e1)(e1.e2))  /  ((e0.e0)(e1.e1) - (e0.e1)(e0.e1))
//     v = ((e0.e0)(e1.e2) - (e0.e1)(e0.e2))  /  ((e0.e0)(e1.e1) - (e0.e1)(e0.e1))
//
// Setting a = e0.e0, b = e0.e1, c = e1.e1, d = e0.(-e2), e = e1.(-e2) we can write
//
//     u = (b*e - c*d) / (a*c - b*b)
//     v = (b*d - a*e) / (a*c - b*b)
//
// If (u >= 0) and (v >= 0) and (u + v <= 1) the point lies inside the triangle.
//
// Note that u and v do not need to be computed in full to do the test.
// Lets define the following substitutions:
//     x = (b*e - c*d)
//     y = (b*d - a*e)
//     z = (a*c - b*b)  // Always positive!
//
// If (x >= 0) and (y >= 0) and (x + y <= z) the point lies inside the triangle.
//
//

namespace physx
{
	PX_FORCE_INLINE PxU32 collideWithMeshTriangle(PxVec3& surfaceNormal, PxVec3& surfacePos,
								  PxVec3& proxSurfaceNormal, PxVec3& proxSurfacePos,
								  PxReal& ccTime, PxReal& distOldToSurface,
								  const PxVec3& oldPos, const PxVec3& newPos,
								  const PxVec3& origin, const PxVec3& e0,
								  const PxVec3& e1, bool hasCC,
								  const PxReal& collRadius, const PxReal& proxRadius)
	{
		PxU32 flags = 0;

		PxReal collisionRadius2 = collRadius * collRadius;
		PxReal proximityRadius2 = proxRadius * proxRadius;

		PxVec3 motion = newPos - oldPos;

		// dc and proximity tests
		PxVec3 tmpV = origin - newPos;

		PxReal a = e0.dot(e0);
		PxReal b = e0.dot(e1);
		PxReal c = e1.dot(e1);
		PxReal d = e0.dot(tmpV);
		PxReal e = e1.dot(tmpV);
		PxVec3 coords;
		coords.x = b*e - c*d;	// s * det
		coords.y = b*d - a*e;	// t * det
		coords.z = a*c - b*b;	// det

		bool insideCase = false;
		PxVec3 clampedCoords(PxVec3(0));
		if (coords.x <= 0.0f) 
		{
			c = PxMax(c, FLT_MIN);
			clampedCoords.y = -e/c;
		}
		else if (coords.y <= 0.0f) 
		{
			a = PxMax(a, FLT_MIN);
			clampedCoords.x = -d/a;
		}
		else if (coords.x + coords.y > coords.z) 
		{
			PxReal denominator = a + c - b - b;
			PxReal numerator   = c + e - b - d;
			denominator = PxMax(denominator, FLT_MIN);
			clampedCoords.x = numerator / denominator;
			clampedCoords.y = 1.0f - clampedCoords.x;
		}
		else // all inside 
		{	
			PxReal tmpF = PxMax(coords.z, FLT_MIN);
			tmpF = 1.0f / tmpF;
			clampedCoords.x = coords.x * tmpF;
			clampedCoords.y = coords.y * tmpF;
			insideCase = true;
		}
		clampedCoords.x = PxMax(clampedCoords.x, 0.0f);
		clampedCoords.y = PxMax(clampedCoords.y, 0.0f);
		clampedCoords.x = PxMin(clampedCoords.x, 1.0f);
		clampedCoords.y = PxMin(clampedCoords.y, 1.0f);

		// Closest point to particle inside triangle
		PxVec3 closest = origin + e0 * clampedCoords.x + e1 * clampedCoords.y;

		PxVec3 triangleOffset = newPos - closest;
		PxReal triangleDistance2 = triangleOffset.magnitudeSquared();

		PxVec3 triangleNormal = e0.cross(e1);
		PxReal e0e1Span = triangleNormal.magnitude();
		
		bool isInFront = triangleOffset.dot(triangleNormal) > 0.0f;

		// MS: Possible optimzation
		/*
		if (isInFront && (triangleDistance2 >= proximityRadius2))
			return flags;
		*/

		bool isInProximity = insideCase && (triangleDistance2 < proximityRadius2) && isInFront;
		bool isInDiscrete = (triangleDistance2 < collisionRadius2) && isInFront;

		if (!hasCC)
		{
			// Only apply discrete and proximity collisions if no continuous collisions was detected so far (for any colliding shape)

			if (isInDiscrete)
			{
				if (triangleDistance2 > PXS_FLUID_COLL_TRI_DISTANCE)
				{
					surfaceNormal = triangleOffset * PxRecipSqrt(triangleDistance2);
				}
				else
				{
					surfaceNormal = triangleNormal * (1.0f / e0e1Span);
				}
				surfacePos = closest + (surfaceNormal * collRadius);
				flags |= PXS_FLUID_COLL_FLAG_L_DC;
			}
			
			if (isInProximity)
			{
				proxSurfaceNormal = triangleNormal * (1.0f / e0e1Span);
				proxSurfacePos = closest + (proxSurfaceNormal * collRadius);
				flags |= PXS_FLUID_COLL_FLAG_L_PROX;

				tmpV = (oldPos - origin); //this time it's not the newPosition offset.
				distOldToSurface = proxSurfaceNormal.dot(tmpV);	// Need to return the distance to decide which constraints should be thrown away
			}
		}

		if (!isInDiscrete && !isInProximity)
		{
			// cc test (let's try only executing this if no discrete coll, or proximity happend).
			tmpV = origin - oldPos; //this time it's not the newPosition offset.
			PxReal pDistN = triangleNormal.dot(tmpV);
			PxReal rLengthN = triangleNormal.dot(motion);

			if (pDistN > 0.0f || rLengthN >= pDistN) 
				return flags;

			//we are in the half closed interval [0.0f, 1.0)
			
			PxReal t = pDistN / rLengthN;
			PX_ASSERT((t >= 0.0f) && (t < 1.0f));

			PxVec3 relativePOSITION = (motion * t);
			PxVec3 testPoint = oldPos + relativePOSITION;

			// a,b,c and coords.z don't depend on test point -> still valid
			tmpV = origin - testPoint;
			d = e0.dot(tmpV);
			e = e1.dot(tmpV);
			coords.x = b*e - c*d;
			coords.y = b*d - a*e;

			//maybe we don't need this for rare case leaking on triangle boundaries? 
			PxReal eps = coords.z * PXS_FLUID_COLL_RAY_EPSILON_FACTOR;

			if ((coords.x >= -eps) && (coords.y >= -eps) && (coords.x + coords.y <= coords.z + eps)) 
			{
				PxReal invLengthN = (1.0f / e0e1Span);
				distOldToSurface = -pDistN * invLengthN;	// Need to return the distance to decide which constraints should be thrown away
				surfaceNormal = triangleNormal * invLengthN;
				//surfacePos = testPoint + (surfaceNormal * collRadius);
				computeContinuousTargetPosition(surfacePos, oldPos, relativePOSITION, surfaceNormal, collRadius);
				ccTime = t;
				flags |= PXS_FLUID_COLL_FLAG_L_CC;
			}
		}

		return flags;
	}
}

PX_FORCE_INLINE void setConstraintData(PxsParticleCollData& collData,
										const PxReal& distToSurface,
										const PxVec3& normal,
										const PxVec3& position,
										const PxTransform& shape2World)
{
	PxU32 i;

	if (!(collData.particleFlags.low & PxvInternalParticleFlag::eCONSTRAINT_0_VALID))
	{
		i = 0;		
	}
	else if (!(collData.particleFlags.low & PxvInternalParticleFlag::eCONSTRAINT_1_VALID))
	{
		i = 1;	
	}
	else
	{
		PxVec3 oldWorldSurfacePos(shape2World.transform(collData.localOldPos));

		PxReal dist0 = collData.c0->normal.dot(oldWorldSurfacePos) - collData.c0->d;
		PxReal dist1 = collData.c1->normal.dot(oldWorldSurfacePos) - collData.c1->d;

		if ( dist0 < dist1)
		{
			if (distToSurface < dist1)
		         i = 1;
			else
				return;
		}
		else if (distToSurface < dist0)
		{
			i = 0;
		}
		else
			return;
	}

	PxVec3 newSurfaceNormal(shape2World.rotate(normal));
	PxVec3 newSurfacePos(shape2World.transform(position));
	PxsFluidConstraint cN(newSurfaceNormal, newSurfacePos);

	if(i == 0)
	{
		*collData.c0 = cN;			
		collData.particleFlags.low |= PxvInternalParticleFlag::eCONSTRAINT_0_VALID;
		collData.particleFlags.low &= PxU16(~PxvInternalParticleFlag::eCONSTRAINT_0_DYNAMIC);
	}
	else
	{
		*collData.c1 = cN;			
		collData.particleFlags.low |= PxvInternalParticleFlag::eCONSTRAINT_1_VALID;
		collData.particleFlags.low &= PxU16(~PxvInternalParticleFlag::eCONSTRAINT_1_DYNAMIC);
	}
}

PX_FORCE_INLINE void updateCollShapeData(PxsParticleCollData& collData, bool& hasCC, PxU32 collFlags,
										  PxReal ccTime, PxReal distOldToSurface,
										  const PxVec3& surfaceNormal, const PxVec3& surfacePos,
										  const PxVec3& proxSurfaceNormal, const PxVec3& proxSurfacePos, const PxTransform& shape2World)
{
	if (collFlags & PXS_FLUID_COLL_FLAG_L_CC)
	{
		if (ccTime < collData.ccTime)
		{
			// We want the collision that happened first
			collData.localSurfaceNormal = surfaceNormal;
			collData.localSurfacePos = surfacePos;
			collData.ccTime = ccTime;
			collData.localFlags = PXS_FLUID_COLL_FLAG_L_CC;	// Continuous collision should overwrite discrete collision (?)
		}
		
		setConstraintData(collData, distOldToSurface, surfaceNormal, surfacePos, shape2World);
		hasCC = true;
	}
	else if(!hasCC)
	{
		if (collFlags & PXS_FLUID_COLL_FLAG_L_PROX)
		{
			setConstraintData(collData, distOldToSurface, proxSurfaceNormal, proxSurfacePos, shape2World);

			collData.localFlags |= PXS_FLUID_COLL_FLAG_L_PROX;
		}

		if (collFlags & PXS_FLUID_COLL_FLAG_L_DC)
		{
			collData.localSurfaceNormal += surfaceNormal;
			collData.localSurfacePos += surfacePos;
			collData.localDcNum += 1.0f;
			collData.localFlags |= PXS_FLUID_COLL_FLAG_L_DC;
		}
	}
}

void collideCellWithMeshTriangles(PxsParticleCollData* collData, const PxU32* collDataIndices,
	PxU32 numCollDataIndices, const Gu::InternalTriangleMeshData& meshData, const Cm::FastVertex2ShapeScaling& scale,
	const PxVec3* triangleVerts, PxU32 numTriangles, PxReal proxRadius, const PxTransform& shape2World);

struct PxcContactCellMeshCallback : Gu::MeshHitCallback<PxRaycastHit>
{
	PxsParticleCollData* collData;
	const PxU32* collDataIndices;
	PxU32 numCollDataIndices;
	const Gu::InternalTriangleMeshData& meshData;	
	const Cm::FastVertex2ShapeScaling meshScaling;
	PxReal proxRadius;
	PxsFluidParticleOpcodeCache* cache;
	const PxTransform& shape2World;

		PxcContactCellMeshCallback(
			PxsParticleCollData* collData_,
			const PxU32* collDataIndices_,
			PxU32 numCollDataIndices_,
			const Gu::InternalTriangleMeshData& meshData_,									
			const Cm::FastVertex2ShapeScaling& meshScaling_,
			PxReal proxRadius_,
			PxsFluidParticleOpcodeCache* cache_,
			const PxTransform& shape2World_)
			:	Gu::MeshHitCallback<PxRaycastHit>(Gu::CallbackMode::eMULTIPLE),
				collData(collData_), collDataIndices(collDataIndices_),
				numCollDataIndices(numCollDataIndices_),meshData(meshData_),
				meshScaling(meshScaling_),proxRadius(proxRadius_),cache(cache_),shape2World(shape2World_)
		{
			PX_ASSERT(collData);
			PX_ASSERT(collDataIndices);
			PX_ASSERT(numCollDataIndices > 0);

			//init
			const PxU32* collDataIndexIt = collDataIndices_;
			for(PxU32 i=0; i < numCollDataIndices_; ++i, ++collDataIndexIt)
			{
				PxsParticleCollData& collisionShapeData = collData_[*collDataIndexIt];
				collisionShapeData.localDcNum = 0.0f;
				collisionShapeData.localSurfaceNormal = PxVec3(0);
				collisionShapeData.localSurfacePos = PxVec3(0);
			}			
		}
		virtual ~PxcContactCellMeshCallback() {}

		virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
			const PxRaycastHit& hit, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2, PxReal&, const PxU32*)
		{
			PxVec3 verts[3] = {v0, v1, v2};
			collideCellWithMeshTriangles(collData, collDataIndices, numCollDataIndices, meshData,
				meshScaling, verts, 1, proxRadius, shape2World);

			if (cache)
				cache->add(&hit.faceIndex, 1);

			return true;
		}

private:
	PxcContactCellMeshCallback& operator=(const PxcContactCellMeshCallback&);
};


void testBoundsMesh(	
	const Gu::InternalTriangleMeshData& meshData,
	const PxTransform& world2Shape,
	const PxTransform& s2w,
	const Cm::FastVertex2ShapeScaling& meshScaling,
	bool idtScaleMesh,
	const PxBounds3& worldBounds,
	PxcContactCellMeshCallback& callback)
{
	// Find colliding triangles.
	// Setup an OBB for the fluid particle cell (in local space of shape)
	// assuming uniform scaling in most cases, using the pose as box rotation
	// if scaling is non-uniform, the bounding box is conservative

	PxBounds3 boundsInMesh;
	PX_ASSERT(!worldBounds.isEmpty());
	boundsInMesh = PxBounds3::transformFast(world2Shape, worldBounds);

	Gu::Box vertexSpaceOBB(boundsInMesh.getCenter(), boundsInMesh.getExtents(), PxMat33(PxIdentity));

	if(!idtScaleMesh)
		meshScaling.transformQueryBounds(vertexSpaceOBB.center, vertexSpaceOBB.extents);

	// Set collider flags (has to be done each time again!)
	Gu::RTreeMidphaseData hmd;
	meshData.mCollisionModel.getRTreeMidphaseData(hmd);	
	MPT_SET_CONTEXT("flui", s2w, meshScaling); PX_UNUSED(s2w);
	Gu::MeshRayCollider::collideOBB(vertexSpaceOBB, true, hmd, callback);
}

void collideWithMeshTriangles(PxsParticleCollData& collisionShapeData, const Gu::InternalTriangleMeshData& /*meshData*/, const Cm::FastVertex2ShapeScaling& scale,
											  const PxVec3* triangleVerts, PxU32 numTriangles, PxReal proxRadius, const PxTransform& shape2World)
{	
	bool hasCC = ((collisionShapeData.localFlags & PXS_FLUID_COLL_FLAG_CC) || (collisionShapeData.localFlags & PXS_FLUID_COLL_FLAG_L_CC));

	PxVec3 tmpSurfaceNormal(0.0f);
	PxVec3 tmpSurfacePos(0.0f);
	PxVec3 tmpProxSurfaceNormal(0.0f);
	PxVec3 tmpProxSurfacePos(0.0f);
	PxReal tmpCCTime(0.0f);
	PxReal tmpDistOldToSurface(0.0f);
	
	for (PxU32 i = 0; i < numTriangles; ++i)
	{
		PxVec3 v0 = scale * triangleVerts[i*3];
		PxVec3 v1 = scale * triangleVerts[i*3 + 1];
		PxVec3 v2 = scale * triangleVerts[i*3 + 2];

		PxU32 tmpFlags = collideWithMeshTriangle(tmpSurfaceNormal, tmpSurfacePos, tmpProxSurfaceNormal, tmpProxSurfacePos,
			tmpCCTime, tmpDistOldToSurface, collisionShapeData.localOldPos, collisionShapeData.localNewPos, 
			v0, v1-v0, v2-v0, hasCC, collisionShapeData.restOffset, proxRadius);

		updateCollShapeData(collisionShapeData, hasCC, tmpFlags, tmpCCTime, tmpDistOldToSurface, tmpSurfaceNormal,
			tmpSurfacePos, tmpProxSurfaceNormal, tmpProxSurfacePos, shape2World);
	}	
}

void collideCellWithMeshTriangles(PxsParticleCollData* collData, const PxU32* collDataIndices,
									PxU32 numCollDataIndices, const Gu::InternalTriangleMeshData& meshData, const Cm::FastVertex2ShapeScaling& scale,
									const PxVec3* triangleVerts, PxU32 numTriangles, PxReal proxRadius, const PxTransform& shape2World)
{
	PX_ASSERT(collData);
	PX_ASSERT(collDataIndices);
	PX_ASSERT(numCollDataIndices > 0);
	PX_ASSERT(triangleVerts);

	const PxU32* collDataIndexIt = collDataIndices;
	for(PxU32 i=0; i < numCollDataIndices; ++i, ++collDataIndexIt)
	{
		PxsParticleCollData& collisionShapeData = collData[*collDataIndexIt];
		collideWithMeshTriangles(collisionShapeData, meshData, scale, triangleVerts, numTriangles, proxRadius, shape2World);
	}
}



void physx::collideCellsWithStaticMesh(PxsParticleCollData* collData, const PxsFluidLocalCellHash& localCellHash, const Gu::GeometryUnion& meshShape,
	 const PxTransform& world2Shape, const PxTransform& shape2World, PxReal /*cellSize*/, PxReal /*collisionRange*/, PxReal proxRadius, const PxVec3& /*packetCorner*/)
{
	PX_ASSERT(collData);
	PX_ASSERT(localCellHash.isHashValid);
	PX_ASSERT(localCellHash.numParticles <= PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_COLLISION);
	PX_ASSERT(localCellHash.numHashEntries <= PXS_FLUID_LOCAL_HASH_SIZE_MESH_COLLISION);

	const PxTriangleMeshGeometryLL& meshShapeData = meshShape.get<const PxTriangleMeshGeometryLL>();

	const Gu::InternalTriangleMeshData* meshData = meshShapeData.meshData;
	PX_ASSERT(meshData);

	// mesh bounds in world space (conservative)
	const PxBounds3 shapeBounds = transform(world2Shape.getInverse() * meshShapeData.scale, meshData->mAABB);

	const bool idtScaleMesh = meshShapeData.scale.isIdentity();

	Cm::FastVertex2ShapeScaling meshScaling;
	if(!idtScaleMesh)
		meshScaling.init(meshShapeData.scale);

	// process the particle cells
	for(PxU32 c=0; c < localCellHash.numHashEntries; c++)
	{
		const PxsParticleCell& cell = localCellHash.hashEntries[c];

		if (cell.numParticles == PX_INVALID_U32)
			continue;

		PxBounds3 cellBounds;		


		cellBounds.setEmpty();
		PxBounds3 cellBoundsNew(PxBounds3::empty());
		
		PxU32* it = localCellHash.particleIndices + cell.firstParticle;
		const PxU32* end = it + cell.numParticles;
		for (; it != end; it++)
		{
			const PxsParticleCollData& particle = collData[*it];
			cellBounds.include(particle.oldPos);
			cellBoundsNew.include(particle.newPos);
		}
		PX_ASSERT(!cellBoundsNew.isEmpty());
		cellBoundsNew.fattenFast(proxRadius);
		cellBounds.include(cellBoundsNew);

	
		if(!cellBounds.intersects(shapeBounds)) 
			continue; // early out if (inflated) cell doesn't intersect mesh bounds

		// opcode query: cell bounds against shape bounds in unscaled mesh space
		PxcContactCellMeshCallback callback(collData, &(localCellHash.particleIndices[cell.firstParticle]), cell.numParticles,
											*meshData, meshScaling, proxRadius, NULL, shape2World);
		testBoundsMesh(*meshData, world2Shape, shape2World, meshScaling, idtScaleMesh, cellBounds, callback);		
	}
}

void physx::collideWithStaticMesh(PxU32 numParticles, PxsParticleCollData* collData,
								  PxsFluidParticleOpcodeCache* opcodeCaches, const Gu::GeometryUnion& meshShape,
								  const PxTransform& world2Shape, const PxTransform& shape2World, PxReal /*cellSize*/, PxReal collisionRange, PxReal proxRadius)
{
	PX_ASSERT(collData);
	PX_ASSERT(opcodeCaches);

	const PxTriangleMeshGeometryLL& meshShapeData = meshShape.get<const PxTriangleMeshGeometryLL>();
	
	const bool idtScaleMesh = meshShapeData.scale.isIdentity();
	Cm::FastVertex2ShapeScaling meshScaling;
	if(!idtScaleMesh)
		meshScaling.init(meshShapeData.scale);

	const PxF32 maxCacheBoundsExtent = 4*collisionRange + proxRadius;
	const PxsFluidParticleOpcodeCache::QuantizationParams quantizationParams = 
		PxsFluidParticleOpcodeCache::getQuantizationParams(maxCacheBoundsExtent);

	const Gu::InternalTriangleMeshData* meshData = meshShapeData.meshData;
	PX_ASSERT(meshData);

	bool isSmallMesh = meshData->mNumTriangles <= 0xffff;
	PxU32 cachedTriangleBuffer[PxsFluidParticleOpcodeCache::sMaxCachedTriangles];

	PxVec3 extent(proxRadius);

	for (PxU32 i = 0; i < numParticles; ++i)
	{
		//had to make this non-const to be able to update cache bits
		PxsParticleCollData& particle = collData[i];
		PxsFluidParticleOpcodeCache& cache = opcodeCaches[i];
		
		PxBounds3 bounds;
		{
			bounds = PxBounds3(particle.newPos - extent, particle.newPos + extent); 
			bounds.include(particle.oldPos);
		}
		
		PxU32 numTriangles = 0;
		const PxU32* triangles = NULL;
		bool isCached = cache.read(particle.particleFlags.low, numTriangles, cachedTriangleBuffer, bounds, quantizationParams, &meshShape, isSmallMesh);

		if (isCached)
		{
			triangles = cachedTriangleBuffer;
			if (numTriangles > 0)
			{
				PxVec3 triangleVerts[PxsFluidParticleOpcodeCache::sMaxCachedTriangles*3];
				const PxU32* triangleIndexIt = triangles;
				for (PxU32 j = 0; j < numTriangles; ++j, ++triangleIndexIt)
				{
					Gu::MeshInterface::GetTriangleVerts((PxU32)isSmallMesh, (Cm::MemFetchPtr)meshData->mTriangles, (Cm::MemFetchPtr)meshData->mVertices, *triangleIndexIt,
						triangleVerts[j*3], triangleVerts[j*3 + 1], triangleVerts[j*3 + 2]);
				}	
				
				collData[i].localDcNum = 0.0f;
				collData[i].localSurfaceNormal = PxVec3(0);
				collData[i].localSurfacePos = PxVec3(0);

				collideWithMeshTriangles(collData[i], *meshData, meshScaling, triangleVerts, numTriangles, proxRadius, shape2World);				
			}
		}
		else if ((particle.particleFlags.low & PxvInternalParticleFlag::eGEOM_CACHE_BIT_0) != 0 && (particle.particleFlags.low & PxvInternalParticleFlag::eGEOM_CACHE_BIT_1) != 0)
		{
			// don't update the cache since it's already successfully in use
			PxcContactCellMeshCallback callback(collData, &i, 1, *meshData, meshScaling, proxRadius, NULL, shape2World);
				
			testBoundsMesh(*meshData, world2Shape, shape2World, meshScaling, idtScaleMesh, bounds, callback);		
		}
		else
		{
			// compute new conservative bounds for cache
			PxBounds3 cachedBounds;
			{
				PxVec3 predictedExtent(proxRadius*1.5f);

				//add future newpos + extent
				PxVec3 newPosPredicted = particle.newPos + 3.f*(particle.newPos - particle.oldPos);
				cachedBounds = PxBounds3(newPosPredicted - predictedExtent, newPosPredicted + predictedExtent);
				
				//add next oldpos + extent
				cachedBounds.include(PxBounds3(particle.newPos - predictedExtent, particle.newPos + predictedExtent));
				
				//add old pos
				cachedBounds.include(particle.oldPos);
			}
         
			cache.init(cachedTriangleBuffer);

			//the callback function will call collideWithMeshTriangles()
			PxcContactCellMeshCallback callback(collData, &i, 1, *meshData, meshScaling, proxRadius, &cache, shape2World);

			// opcode query: cache bounds against shape bounds in unscaled mesh space
			testBoundsMesh(*meshData, world2Shape, shape2World, meshScaling, idtScaleMesh, cachedBounds, callback);		
	
			//update cache
			cache.write(particle.particleFlags.low, cachedBounds, quantizationParams, meshShape, isSmallMesh);
		}
	}
}

void physx::collideWithStaticHeightField(PxsParticleCollData* particleCollData, PxU32 numCollData,
										 const Gu::GeometryUnion& heightFieldShape, PxReal proxRadius, const PxTransform& shape2World)
{
	PX_ASSERT(particleCollData);	

	const PxHeightFieldGeometryLL& hfGeom = heightFieldShape.get<const PxHeightFieldGeometryLL>();
	const Gu::HeightFieldUtil hfUtil(hfGeom);

	for(PxU32 p=0; p < numCollData; p++)
	{
		PxsParticleCollData& collData = particleCollData[p];

		PxBounds3 particleBounds = PxBounds3::boundsOfPoints(collData.localOldPos, collData.localNewPos);
		PX_ASSERT(!particleBounds.isEmpty());
		particleBounds.fattenFast(proxRadius);

		PxcHeightFieldAabbTest test(particleBounds, hfUtil);
		PxcHeightFieldAabbTest::Iterator itBegin = test.begin();
		PxcHeightFieldAabbTest::Iterator itEnd = test.end();
		PxVec3 triangle[3];

		collData.localDcNum = 0.0f;
		collData.localSurfaceNormal = PxVec3(0);
		collData.localSurfacePos = PxVec3(0);
		bool hasCC = (collData.localFlags & PXS_FLUID_COLL_FLAG_CC) > 0;

		PxVec3 tmpSurfaceNormal(0.0f);
		PxVec3 tmpSurfacePos(0.0f);
		PxVec3 tmpProxSurfaceNormal(0.0f);
		PxVec3 tmpProxSurfacePos(0.0f);
		PxReal tmpCCTime(collData.ccTime);
		PxReal tmpDistOldToSurface(0.0f);
		
		for (PxcHeightFieldAabbTest::Iterator it = itBegin; it != itEnd; ++it)
		{
			it.getTriangleVertices(triangle);
		
			const PxVec3& origin = triangle[0];
			PxVec3 e0, e1;
			e0 = triangle[1] - origin;
			e1 = triangle[2] - origin;

			PxU32 tmpFlags = collideWithMeshTriangle(tmpSurfaceNormal, tmpSurfacePos, tmpProxSurfaceNormal, tmpProxSurfacePos,
												tmpCCTime, tmpDistOldToSurface, collData.localOldPos, collData.localNewPos,
												origin, e0, e1, hasCC, collData.restOffset, proxRadius);

			updateCollShapeData(collData, hasCC, tmpFlags, tmpCCTime, tmpDistOldToSurface, tmpSurfaceNormal,
								tmpSurfacePos, tmpProxSurfaceNormal, tmpProxSurfacePos, shape2World);
		}
	}
}
