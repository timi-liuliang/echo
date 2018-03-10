/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#include "MinPhysxSdkVersion.h"
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

#include "SimulationAbstract.h"

// for NUM_VERTICES_PER_CACHE_BLOCK
#include "ClothingAsset.h"


namespace physx
{
namespace apex
{
namespace clothing
{

void SimulationAbstract::init(PxU32 numVertices, PxU32 numIndices, bool writebackNormals)
{
	sdkNumDeformableVertices = numVertices;
	sdkNumDeformableIndices = numIndices;

	const PxU32 alignedNumVertices = (numVertices + 15) & 0xfffffff0;
	const PxU32 writeBackDataSize = (sizeof(PxVec3) * alignedNumVertices) * (writebackNormals ? 2 : 1);

	PX_ASSERT(sdkWritebackPosition == NULL);
	PX_ASSERT(sdkWritebackNormal == NULL);
	sdkWritebackPosition = (PxVec3*)PX_ALLOC(writeBackDataSize, PX_DEBUG_EXP("SimulationAbstract::writebackData"));
	sdkWritebackNormal = writebackNormals ? sdkWritebackPosition + alignedNumVertices : NULL;

	const PxU32 allocNumVertices = (((numVertices + NUM_VERTICES_PER_CACHE_BLOCK - 1) / NUM_VERTICES_PER_CACHE_BLOCK)) * NUM_VERTICES_PER_CACHE_BLOCK;
	PX_ASSERT(skinnedPhysicsPositions == NULL);
	PX_ASSERT(skinnedPhysicsNormals == NULL);
	skinnedPhysicsPositions = (PxVec3*)PX_ALLOC(sizeof(PxVec3) * allocNumVertices * 2, PX_DEBUG_EXP("SimulationAbstract::skinnedPhysicsPositions"));
	skinnedPhysicsNormals = skinnedPhysicsPositions + allocNumVertices;
}



void SimulationAbstract::initSimulation(const tSimParams& s)
{
	simulation = s;
}


}
} // namespace apex
} // namespace physx

#endif //NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED && NX_SDK_VERSION_MAJOR == 2
