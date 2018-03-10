/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_RENDER_PROXY_H
#define CLOTHING_RENDER_PROXY_H

#include "NxClothingRenderProxy.h"
#include "PxMat44.h"
#include "PxBounds3.h"
#include "PsHashMap.h"
#include "ApexString.h"
#include "ApexRWLockable.h"

namespace physx
{
namespace apex
{
class NiApexRenderMeshActor;
class NiApexRenderMeshAsset;

namespace clothing
{
class ClothingActorParam;
class ClothingScene;


class ClothingRenderProxy : public NxClothingRenderProxy, public UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingRenderProxy(NiApexRenderMeshAsset* rma, bool useFallbackSkinning, bool useCustomVertexBuffer, const physx::shdfnd::HashMap<PxU32, ApexSimpleString>& overrideMaterials, const PxVec3* morphTargetNewPositions, const PxU32* morphTargetVertexOffsets, ClothingScene* scene);
	virtual ~ClothingRenderProxy();

	// from NxApexInterface
	virtual void release();

	// from NxApexRenderable
	virtual void dispatchRenderResources(NxUserRenderer& api);
	virtual PxBounds3 getBounds() const
	{
		return mBounds;
	}
	void setBounds(const PxBounds3& bounds)
	{
		mBounds = bounds;
	}

	// from NxApexRenderDataProvider.h
	virtual void lockRenderResources();
	virtual void unlockRenderResources();
	virtual void updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0);

	void setPose(const PxMat44& pose)
	{
		mPose = pose;
	}

	// from NxClothingRenderProxy.h
	virtual bool hasSimulatedData() const;

	NiApexRenderMeshActor* getRenderMeshActor();
	NiApexRenderMeshAsset* getRenderMeshAsset();

	bool usesFallbackSkinning() const
	{
		return mUseFallbackSkinning;
	}

	bool usesCustomVertexBuffer() const
	{
		return renderingDataPosition != NULL;
	}

	const PxVec3* getMorphTargetBuffer() const
	{
		return mMorphTargetNewPositions;
	}

	void setOverrideMaterial(PxU32 i, const char* overrideMaterialName);
	bool overrideMaterialsEqual(const physx::shdfnd::HashMap<PxU32, ApexSimpleString>& overrideMaterials);

	PxU32 getTimeInPool()  const;
	void setTimeInPool(PxU32 time);

	void notifyAssetRelease();

	PxVec3* renderingDataPosition;
	PxVec3* renderingDataNormal;
	PxVec4* renderingDataTangent;

private:
	NiApexRenderMeshActor* createRenderMeshActor(NiApexRenderMeshAsset* renderMeshAsset, ClothingActorParam* actorDesc);

	PxBounds3 mBounds;
	PxMat44 mPose;

	NiApexRenderMeshActor* mRenderMeshActor;
	NiApexRenderMeshAsset* mRenderMeshAsset;

	ClothingScene* mScene;

	bool mUseFallbackSkinning;
	HashMap<PxU32, ApexSimpleString> mOverrideMaterials;
	const PxVec3* mMorphTargetNewPositions; // just to compare, only read it in constructor (it may be released)

	PxU32 mTimeInPool;

	Mutex mRMALock;
};

}
} // namespace apex
} // namespace physx

#endif // CLOTHING_RENDER_PROXY_H
