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

#include "CookingPhysX.h"

#include "ClothingCookedParam.h"
#include "ClothingAssetParameters.h"

#include "NiApexSDK.h"
#include "NxParamArray.h"

#include "PxMemoryBuffer.h"

#include "NxFromPx.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxCooking.h"
#include "NxConvexMeshDesc.h"
#include "cloth/NxClothMeshDesc.h"
#include "softbody/NxSoftBodyMeshDesc.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxCooking.h"
#endif


namespace physx
{
namespace apex
{
namespace clothing
{

NxParameterized::Interface* CookingPhysX::execute()
{
#if NX_SDK_VERSION_MAJOR == 3
	PX_ALWAYS_ASSERT_MESSAGE("Cooking with the 3.x PhysX SDK should not happen ever!");
	return NULL;
#else

	ClothingCookedParam* result = DYNAMIC_CAST(ClothingCookedParam*)(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(ClothingCookedParam::staticClassName()));

	NxParamArray<PxU8> deformableCookedData(result, "deformableCookedData", reinterpret_cast<NxParamDynamicArrayStruct*>(&result->deformableCookedData));
	NxParamArray<ClothingCookedParamNS::CookedPhysicalSubmesh_Type> cookedPhysicalSubmeshes(result, "cookedPhysicalSubmeshes", reinterpret_cast<NxParamDynamicArrayStruct*>(&result->cookedPhysicalSubmeshes));

	PxU32 numFailures = 0;
	PxMemoryBuffer memoryBuffer;
	memoryBuffer.setEndianMode(PxFileBuf::ENDIAN_NONE);
	
	PxF32 smallestTriangleArea = PX_MAX_F32;
	PxF32 largestTriangleArea = 0.0f;
	for (PxU32 i = 0; i < mPhysicalMeshes.size(); i++)
	{
		smallestTriangleArea = PxMin(mPhysicalMeshes[i].smallestTriangleArea, smallestTriangleArea);
		largestTriangleArea = PxMax(mPhysicalMeshes[i].largestTriangleArea, largestTriangleArea);
	}

	for (PxU32 i = 0; i < mPhysicalMeshes.size(); i++)
	{
		PxU32 numVirtualParticlesIdx = 0; // used the pick the right entries out of numVirtualParticles when storing the cooking data
		for (PxU32 j = 0; j < mPhysicalSubmeshes.size(); j++)
		{
			if (mPhysicalSubmeshes[j].meshID != mPhysicalMeshes[i].meshID)
			{
				continue;
			}

			memoryBuffer.seekWrite(0);

			NxFromPxStream stream(memoryBuffer);
			bool cookingSuccess = false;

			if (mPhysicalMeshes[i].isTetrahedral)
			{
				NxSoftBodyMeshDesc meshDesc;
				meshDesc.vertices = mPhysicalMeshes[i].vertices;
				meshDesc.vertexStrideBytes = sizeof(PxVec3);
				meshDesc.numVertices = mPhysicalSubmeshes[j].numVertices;

				meshDesc.tetrahedra = mPhysicalMeshes[i].indices;
				meshDesc.tetrahedronStrideBytes = sizeof(PxU32) * 4;
				PX_ASSERT(mPhysicalSubmeshes[j].numIndices % 4 == 0);
				meshDesc.numTetrahedra = mPhysicalSubmeshes[j].numIndices / 4;

				PX_ASSERT(meshDesc.isValid());
				cookingSuccess = NiGetApexSDK()->getCookingInterface()->NxCookSoftBodyMesh(meshDesc, stream);
			}
			else
			{
				// cook
				NxClothMeshDesc meshDesc;

				meshDesc.points = mPhysicalMeshes[i].vertices;
				meshDesc.pointStrideBytes = sizeof(PxVec3);
				meshDesc.numVertices = mPhysicalSubmeshes[j].numVertices;

				meshDesc.triangles = mPhysicalMeshes[i].indices;
				meshDesc.triangleStrideBytes = sizeof(PxU32) * 3;
				PX_ASSERT(mPhysicalSubmeshes[j].numIndices % 3 == 0);
				meshDesc.numTriangles = mPhysicalSubmeshes[j].numIndices / 3;

				PX_ASSERT(meshDesc.isValid());
				cookingSuccess = NiGetApexSDK()->getCookingInterface()->NxCookClothMesh(meshDesc, stream);
			}

			if (cookingSuccess)
			{
				// store the cooking data
				ClothingCookedParamNS::CookedPhysicalSubmesh_Type& submesh = cookedPhysicalSubmeshes.pushBack();
				submesh.physicalMeshId = mPhysicalSubmeshes[j].meshID;
				submesh.submeshId = mPhysicalSubmeshes[j].submeshID;
				submesh.cookedDataOffset = deformableCookedData.size();
				submesh.cookedDataLength = memoryBuffer.getWriteBufferSize();

				deformableCookedData.resize(submesh.cookedDataOffset + submesh.cookedDataLength);
				memcpy(deformableCookedData.begin() + submesh.cookedDataOffset, memoryBuffer.getWriteBuffer(), submesh.cookedDataLength);
			}
			else
			{
				numFailures++;
			}

			numVirtualParticlesIdx++;
		}
	}

	if (numFailures > 0)
	{
		APEX_DEBUG_WARNING("Deformable Cooking Failures, %d of %d submeshes were not cooked properly, behavior may be undefined.", numFailures, mPhysicalSubmeshes.size());
	}

	memoryBuffer.seekWrite(0);

	for (PxU32 i = 0; i < mNumBoneActors; i++)
	{
		if (mBoneActors[i].capsuleRadius > 0.0f)
		{
			continue;
		}

		if (mBoneActors[i].convexVerticesCount == 0)
		{
			continue;
		}

		const PxU32 vertexCount = PxMin(mBoneActors[i].convexVerticesCount, mMaxConvexVertices);

		NxConvexMeshDesc convexDesc;
		convexDesc.numVertices          = vertexCount;
		convexDesc.pointStrideBytes     = sizeof(PxVec3);
		convexDesc.points               = mBoneVertices + mBoneActors[i].convexVerticesStart;
		convexDesc.flags                = NX_CF_COMPUTE_CONVEX;

		NxFromPxStream stream(memoryBuffer);
		bool status = NiGetApexSDK()->getCookingInterface()->NxCookConvexMesh(convexDesc, stream);
		if (!status)
		{
			APEX_DEBUG_WARNING("Convex cooking failed for Bone Actor %d", i);
			memoryBuffer.seekWrite(0);
			break;
		}
	}
	if (memoryBuffer.getWriteBufferSize() > 0)
	{
		NxParamArray<PxU8> cookedData(result, "convexCookedData", reinterpret_cast<NxParamDynamicArrayStruct*>(&result->convexCookedData));
		cookedData.resize(memoryBuffer.getWriteBufferSize());
		memcpy(cookedData.begin(), memoryBuffer.getWriteBuffer(), memoryBuffer.getWriteBufferSize());

	}

	result->cookedDataVersion = getCookingVersion();

	result->actorScale = mScale;

	return result;
#endif
}


}
} // namespace apex
} // namespace physx

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED && NX_SDK_VERSION_MAJOR == 2
