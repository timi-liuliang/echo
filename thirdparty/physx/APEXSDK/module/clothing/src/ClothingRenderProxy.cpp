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

#include "ClothingRenderProxy.h"

#include "ClothingAsset.h"
#include "ClothingActorParam.h"
#include "NxRenderMeshActorDesc.h"
#include "NiApexRenderMeshAsset.h"
#include "ClothingScene.h"

namespace physx
{
namespace apex
{
namespace clothing
{

ClothingRenderProxy::ClothingRenderProxy(NiApexRenderMeshAsset* rma, bool useFallbackSkinning, bool useCustomVertexBuffer, const HashMap<PxU32, ApexSimpleString>& overrideMaterials, const PxVec3* morphTargetNewPositions, const PxU32* morphTargetVertexOffsets, ClothingScene* scene) : 
renderingDataPosition(NULL),
renderingDataNormal(NULL),
renderingDataTangent(NULL),
mBounds(),
mPose(PxMat44(PxIdentity)),
mRenderMeshActor(NULL),
mRenderMeshAsset(rma),
mScene(scene),
mUseFallbackSkinning(useFallbackSkinning),
mMorphTargetNewPositions(morphTargetNewPositions),
mTimeInPool(0)
{
	// create renderMeshActor
	NxRenderMeshActorDesc desc;
	desc.keepVisibleBonesPacked = false;
	desc.forceFallbackSkinning = mUseFallbackSkinning;

	// prepare material names array and copy the map with override names
	const PxU32 numSubmeshes = rma->getSubmeshCount();
	Array<const char*> overrideMaterialNames;
	for (PxU32 si = 0; si < numSubmeshes; ++si)
	{
		const Pair<const PxU32, ApexSimpleString>* overrideMat = overrideMaterials.find(si);
		if (overrideMat != NULL)
		{
			overrideMaterialNames.pushBack(overrideMat->second.c_str());
			mOverrideMaterials[si] = overrideMat->second;
		}
		else
		{
			overrideMaterialNames.pushBack(rma->getMaterialName(si));
		}
	}

	desc.overrideMaterialCount = numSubmeshes;
	desc.overrideMaterials = &overrideMaterialNames[0];
	mRenderMeshActor = DYNAMIC_CAST(NiApexRenderMeshActor*)(mRenderMeshAsset->createActor(desc));

	// Necessary for clothing
	mRenderMeshActor->setSkinningMode(NxRenderMeshActorSkinningMode::AllBonesPerPart);

	if (useCustomVertexBuffer)
	{
		// get num verts and check if we need tangents
		ClothingGraphicalMeshAssetWrapper meshAsset(rma);
		PxU32 numRenderVertices = meshAsset.getNumTotalVertices();
		bool renderTangents = meshAsset.hasChannel(NULL, NxRenderVertexSemantic::TANGENT);

		// allocate aligned buffers and init to 0
		const PxU32 alignedNumRenderVertices = (numRenderVertices + 15) & 0xfffffff0;
		const PxU32 renderingDataSize = sizeof(PxVec3) * alignedNumRenderVertices * 2 + sizeof(PxVec4) * alignedNumRenderVertices * (renderTangents ? 1 : 0);
		renderingDataPosition = (PxVec3*)PX_ALLOC(renderingDataSize, PX_DEBUG_EXP("SimulationAbstract::renderingDataPositions"));
		renderingDataNormal = renderingDataPosition + alignedNumRenderVertices;
		if (renderTangents)
		{
			renderingDataTangent = reinterpret_cast<PxVec4*>(renderingDataNormal + alignedNumRenderVertices);
			PX_ASSERT(((size_t)renderingDataTangent & 0xf) == 0);
		}
		physx::PxMemSet(renderingDataPosition, 0, renderingDataSize);

		// update rma to use the custom buffers
		PxU32 submeshOffset = 0;
		for (PxU32 i = 0; i < meshAsset.getSubmeshCount(); i++)
		{
			PxVec3* position =  renderingDataPosition	+ (renderingDataPosition != NULL ? submeshOffset : 0);
			PxVec3* normal =    renderingDataNormal		+ (renderingDataNormal != NULL	? submeshOffset : 0);
			PxVec4* tangent =   renderingDataTangent	+ (renderingDataTangent != NULL	? submeshOffset : 0);
			mRenderMeshActor->addVertexBuffer(i, true, position, normal, tangent);

			// morph targets
			if (mMorphTargetNewPositions != NULL)
			{
				const PxVec3* staticPosition = mMorphTargetNewPositions + morphTargetVertexOffsets[i];
				mRenderMeshActor->setStaticPositionReplacement(i, staticPosition);
			}

			submeshOffset += meshAsset.getNumVertices(i);
		}
	}
}



ClothingRenderProxy::~ClothingRenderProxy()
{
	mRMALock.lock();
	if (mRenderMeshActor != NULL)
	{
		mRenderMeshActor->release();
		mRenderMeshActor = NULL;
	}
	mRMALock.unlock();

	if (renderingDataPosition != NULL)
	{
		PX_FREE(renderingDataPosition);
		renderingDataPosition = NULL;
		renderingDataNormal = NULL;
		renderingDataTangent = NULL;
	}
}



// from NxApexInterface
void ClothingRenderProxy::release()
{
	NX_WRITE_ZONE();
	setTimeInPool(1);
	if(mScene == NULL || mRenderMeshActor == NULL)
	{
		PX_DELETE(this);
	}
}



// from NxApexRenderable
void ClothingRenderProxy::dispatchRenderResources(NxUserRenderer& api)
{
	mRMALock.lock();
	if (mRenderMeshActor != NULL)
	{
		mRenderMeshActor->dispatchRenderResources(api, mPose);
	}
	mRMALock.unlock();
}



// from NxApexRenderDataProvider.h
void ClothingRenderProxy::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	URR_SCOPE;
	
	mRMALock.lock();
	if (mRenderMeshActor != NULL)
	{
		mRenderMeshActor->updateRenderResources(renderingDataPosition == NULL, rewriteBuffers, userRenderData);
	}
	mRMALock.unlock();
}


void ClothingRenderProxy::lockRenderResources()
{
	// no need to lock anything, as soon as the user can access the proxy, we don't write it anymore
	// until he calls release
}


void ClothingRenderProxy::unlockRenderResources()
{
}



bool ClothingRenderProxy::hasSimulatedData() const
{
	NX_READ_ZONE();
	return renderingDataPosition != NULL;
}



NiApexRenderMeshActor* ClothingRenderProxy::getRenderMeshActor()
{
	return mRenderMeshActor;
}



NiApexRenderMeshAsset* ClothingRenderProxy::getRenderMeshAsset()
{
	return mRenderMeshAsset;
}



void ClothingRenderProxy::setOverrideMaterial(PxU32 submeshIndex, const char* overrideMaterialName)
{
	mOverrideMaterials[submeshIndex] = overrideMaterialName;
	mRMALock.lock();
	if (mRenderMeshActor != NULL)
	{
		mRenderMeshActor->setOverrideMaterial(submeshIndex, overrideMaterialName);
	}
	mRMALock.unlock();
}



bool ClothingRenderProxy::overrideMaterialsEqual(const HashMap<PxU32, ApexSimpleString>& overrideMaterials)
{
	PxU32 numEntries = mOverrideMaterials.size();
	if (overrideMaterials.size() != numEntries)
		return false;

	for(HashMap<PxU32, ApexSimpleString>::Iterator iter = mOverrideMaterials.getIterator(); !iter.done(); ++iter)
	{
		PxU32 submeshIndex = iter->first;
		const Pair<const PxU32, ApexSimpleString>* overrideMat = overrideMaterials.find(submeshIndex);

		// submeshIndex not found
		if (overrideMat == NULL)
			return false;

		// name is different
		if (overrideMat->second != iter->second)
			return false;
	}

	return true;
}


PxU32 ClothingRenderProxy::getTimeInPool() const
{
	return mTimeInPool;
}


void ClothingRenderProxy::setTimeInPool(PxU32 time)
{
	mTimeInPool = time;
}


void ClothingRenderProxy::notifyAssetRelease()
{
	mRMALock.lock();
	if (mRenderMeshActor != NULL)
	{
		mRenderMeshActor->release();
		mRenderMeshActor = NULL;
	}
	mRenderMeshAsset = NULL;
	mRMALock.unlock();
}

}
} // namespace apex
} // namespace physx

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
