/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEHELPERS_H__
#define __DESTRUCTIBLEHELPERS_H__

#include "NxApex.h"
#include "ApexSDKHelpers.h"
#include "PsUserAllocated.h"
#include "PxMemoryBuffer.h"

#include "NxDestructibleAsset.h"
#include "NxFromPx.h"
#include "NxPhysicsSDK.h"

/*
	For managing mesh cooking at various scales
 */

namespace physx
{
namespace apex
{
namespace destructible
{

class NxConvexHullAtScale
{
public:
	NxConvexHullAtScale() : scale(1), convexMesh(NULL) {}
	NxConvexHullAtScale(const physx::PxVec3& inScale) : scale(inScale), convexMesh(NULL) {}

	physx::PxVec3		scale;
	NxConvexMesh* 		convexMesh;
	physx::Array<PxU8>	cookedHullData;
};

class NxMultiScaledConvexHull : public physx::UserAllocated
{
public:
	NxConvexHullAtScale* 	getConvexHullAtScale(const physx::PxVec3& scale, physx::PxF32 tolerance = 0.0001f)
	{
		// Find mesh at scale.  If not found, create one.
		for (physx::PxU32 index = 0; index < meshes.size(); ++index)
		{
			if (PxVec3equals(meshes[index].scale, scale, tolerance))
			{
				return &meshes[index];
			}
		}
		meshes.insert();
		return new(&meshes.back()) NxConvexHullAtScale(scale);
	}

	void	releaseConvexMeshes()
	{
		for (physx::PxU32 index = 0; index < meshes.size(); ++index)
		{
			if (meshes[index].convexMesh != NULL)
			{
				NxGetApexSDK()->getPhysXSDK()->releaseConvexMesh(*meshes[index].convexMesh);
				meshes[index].convexMesh = NULL;
			}
		}
	}

	Array<NxConvexHullAtScale> meshes;
};


}
}
} // end namespace physx::apex

#endif	// __DESTRUCTIBLEHELPERS_H__
