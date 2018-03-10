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

namespace physx
{

void collideWithBox(PxsParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& boxShape,
					PxReal proxRadius)
{
	PX_ASSERT(particleCollData);
	PX_ASSERT(boxShape.getType() == PxGeometryType::eBOX);

	const PxBoxGeometry &boxShapeData = boxShape.get<const PxBoxGeometry>();

	PxVec3 boxExtent = boxShapeData.halfExtents;
	PxBounds3 shapeBounds(boxExtent * -1.0f, boxExtent);
	PX_ASSERT(!shapeBounds.isEmpty());
	shapeBounds.fattenFast(proxRadius);

	// Box to convex conversion.
	PxPlane planes[6];
	PxVec3 normal;

	normal = PxVec3(1.0f, 0.0f, 0.0f);
	planes[0].n = normal;
	planes[0].d = -boxExtent.x;

	normal = PxVec3(-1.0f, 0.0f, 0.0f);
	planes[1].n = normal;
	planes[1].d = -boxExtent.x;

	normal = PxVec3(0.0f, 1.0f, 0.0f);
	planes[2].n = normal;
	planes[2].d = -boxExtent.y;

	normal = PxVec3(0.0f, -1.0f, 0.0f);
	planes[3].n = normal;
	planes[3].d = -boxExtent.y;

	normal = PxVec3(0.0f, 0.0f, 1.0f);
	planes[4].n = normal;
	planes[4].d = -boxExtent.z;

	normal = PxVec3(0.0f, 0.0f, -1.0f);
	planes[5].n = normal;
	planes[5].d = -boxExtent.z;

#if PXS_FLUID_USE_SIMD_CONVEX_COLLISION
	PxsParticleCollDataV4 collDataV4;
	PxU32 v4Count = 0;

	for(PxU32 p=0; p < numCollData; p++)
	{
		PxsParticleCollData& collData = particleCollData[p];

		PxBounds3 particleBounds = PxBounds3::boundsOfPoints(collData.localOldPos, collData.localNewPos);
		if (particleBounds.intersects(shapeBounds))
		{			
			collDataV4.localOldPos[v4Count].v3 = collData.localOldPos;
			collDataV4.localOldPos[v4Count].pad = 0;
			collDataV4.localNewPos[v4Count].v3 = collData.localNewPos;
			collDataV4.localNewPos[v4Count].pad = 0;	
			collDataV4.localFlags[v4Count] = collData.localFlags;				
			collDataV4.restOffset[v4Count] = collData.restOffset;
			collDataV4.ccTime[v4Count] = collData.ccTime;
			collDataV4.collData[v4Count] = &collData;
			v4Count++;		
		}

		if(v4Count == 4)
		{
			//sschirm: not processing with less than 4 elements to avoid uninitialized data reads
			collideWithConvexPlanesSIMD(collDataV4, planes, 6, proxRadius);
			for(PxU32 j =0 ; j < v4Count; j++)
			{
				PxsParticleCollData* collData1 = collDataV4.collData[j];						
				PxU32 stateFlag = collDataV4.localFlags[j];
				if(stateFlag)
				{
					collData1->localFlags |= stateFlag;
					collData1->ccTime = collDataV4.ccTime[j];							
					collData1->localSurfaceNormal = collDataV4.localSurfaceNormal[j].v3;
					collData1->localSurfacePos = collDataV4.localSurfacePos[j].v3;
				}
			}
			v4Count = 0;
		}
		else if (v4Count > 0 && (p == numCollData-1))
		{
			for (PxU32 j = 0; j < v4Count; j++)
			{
				collideWithConvexPlanes(*collDataV4.collData[j], planes, 6, proxRadius);
			}
		}
	}
#else
	for(PxU32 p=0; p < numCollData; p++)
	{
		PxsParticleCollData& collData = particleCollData[p];
		PxBounds3 particleBounds = PxBounds3::boundsOfPoints(collData.localOldPos, collData.localNewPos);
		if (particleBounds.intersects(shapeBounds))
		{
			collideWithConvexPlanes(collData, planes, 6, proxRadius);
		}
	}
#endif
}

}
