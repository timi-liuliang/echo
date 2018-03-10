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


#ifndef PXS_FLUID_COLLISION_METHODS_H
#define PXS_FLUID_COLLISION_METHODS_H

#include "PxsFluidConfig.h"
#include "PxVec3.h"
#include "PxsFluidCollisionData.h"
#include "PxsFluidSpatialHash.h"
#include "PxsFluidParticleOpcodeCache.h"
#include "GuGeometryUnion.h"

namespace physx
{

/*!
Collision routines for fluid particles
*/

void collideWithPlane(PxsParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& planeShape,
					  PxReal proxRadius);

void collideWithConvexPlanes(PxsParticleCollData& collData, const PxPlane* planes, const PxU32 numPlanes, const PxReal proxRadius);
void collideWithConvexPlanesSIMD(PxsParticleCollDataV4& collDataV4, const PxPlane* convexPlanes, PxU32 numPlanes, const PxReal proxRadius);

/**
input scaledPlaneBuf needs a capacity of the number of planes in convexShape
*/
void collideWithConvex(PxPlane* scaledPlaneBuf, PxsParticleCollData* particleCollData, PxU32 numCollData,
					   const Gu::GeometryUnion& convexShape, const PxReal proxRadius);

void collideWithBox(PxsParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& boxShape,
					PxReal proxRadius);

void collideWithCapsule(PxsParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& capsuleShape,
						PxReal proxRadius);

void collideWithSphere(PxsParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& sphereShape,
					   PxReal proxRadius);

void collideCellsWithStaticMesh(PxsParticleCollData* particleCollData, const PxsFluidLocalCellHash& localCellHash,
								const Gu::GeometryUnion& meshShape, const PxTransform& world2Shape, const PxTransform& shape2World,
								PxReal cellSize, PxReal collisionRange, PxReal proxRadius, const PxVec3& packetCorner);

void collideWithStaticMesh(PxU32 numParticles, PxsParticleCollData* particleCollData,
						   PxsFluidParticleOpcodeCache* opcodeCaches, const Gu::GeometryUnion& meshShape,
						   const PxTransform& world2Shape, const PxTransform& shape2World, PxReal cellSize, PxReal collisionRange,
						   PxReal proxRadius);

void collideWithStaticHeightField(PxsParticleCollData* particleCollData, PxU32 numCollData,
								  const Gu::GeometryUnion& heightFieldShape, PxReal proxRadius, const PxTransform& shape2World);

}

#endif //PXS_FLUID_COLLISION_METHODS_H
