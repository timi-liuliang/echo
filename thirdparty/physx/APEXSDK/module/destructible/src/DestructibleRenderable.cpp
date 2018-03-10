/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxDestructibleRenderable.h"
#include "NxRenderMeshActor.h"
#include "DestructibleActor.h"
#include "ModulePerfScope.h"

namespace physx
{
namespace apex
{
namespace destructible
{

DestructibleRenderable::DestructibleRenderable(NxRenderMeshActor* renderMeshActors[NxDestructibleActorMeshType::Count], DestructibleAsset* asset, PxI32 listIndex)
: mAsset(asset)
, mListIndex(listIndex)
, mRefCount(1)	// Ref count initialized to 1, assuming that whoever calls this constructor will store a reference
{
	for (physx::PxU32 i = 0; i < NxDestructibleActorMeshType::Count; ++i)
	{
		mRenderMeshActors[i] = renderMeshActors[i];
	}
}

DestructibleRenderable::~DestructibleRenderable()
{
	NX_WRITE_ZONE();
	for (physx::PxU32 i = 0; i < NxDestructibleActorMeshType::Count; ++i)
	{
		if (mRenderMeshActors[i])
		{
			mRenderMeshActors[i]->release();
			mRenderMeshActors[i] = NULL;
		}
	}
}

void DestructibleRenderable::release()
{
	bool triggerDelete = false;
	lockRenderResources();
	if (mRefCount > 0)
	{
		triggerDelete = !(--mRefCount);
	}
	unlockRenderResources();
	if (triggerDelete)
	{
		delete this;
	}
}

void DestructibleRenderable::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	PX_PROFILER_PERF_SCOPE("DestructibleRenderableUpdateRenderResources");

	URR_SCOPE;

	for (PxU32 typeN = 0; typeN < NxDestructibleActorMeshType::Count; ++typeN)
	{
		NiApexRenderMeshActor* renderMeshActor = (NiApexRenderMeshActor*)mRenderMeshActors[typeN];
		if (renderMeshActor != NULL)
		{
			renderMeshActor->updateRenderResources((typeN == NxDestructibleActorMeshType::Skinned), rewriteBuffers, userRenderData);
		}
	}

	// Render instanced meshes
	if (mAsset->m_instancingRepresentativeActorIndex == -1)
	{
		mAsset->m_instancingRepresentativeActorIndex = (physx::PxI32)mListIndex;	// using this actor as our representative
	}
	if ((physx::PxI32)mListIndex == mAsset->m_instancingRepresentativeActorIndex)	// doing it this way, in case (for some reason) someone wants to call this fn twice per frame
	{
		mAsset->updateChunkInstanceRenderResources(rewriteBuffers, userRenderData);
		
		mAsset->createScatterMeshInstanceInfo();

		for (physx::PxU32 i = 0; i < mAsset->m_scatterMeshInstanceInfo.size(); ++i)
		{
			DestructibleAsset::ScatterMeshInstanceInfo& info = mAsset->m_scatterMeshInstanceInfo[i];
			NiApexRenderMeshActor* renderMeshActor = (NiApexRenderMeshActor*)info.m_actor;
			if (renderMeshActor != NULL)
			{
				NxApexRenderInstanceBufferData data;
				physx::Array<DestructibleAsset::ScatterInstanceBufferDataElement>& instanceBufferData = info.m_instanceBufferData;
				const physx::PxU32 instanceBufferSize = instanceBufferData.size();

				if (info.m_instanceBuffer != NULL && instanceBufferSize > 0)
				{
					// If a new actor has added instances for this scatter mesh, recreate a larger instance buffer
					if (info.m_IBSize < instanceBufferSize)
					{
						NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();
						rrm->releaseInstanceBuffer(*info.m_instanceBuffer);

						NxUserRenderInstanceBufferDesc instanceBufferDesc = mAsset->getScatterMeshInstanceBufferDesc();
						instanceBufferDesc.maxInstances = instanceBufferSize;
						
						info.m_instanceBuffer = rrm->createInstanceBuffer(instanceBufferDesc);
						info.m_IBSize = instanceBufferSize;
						
						renderMeshActor->setInstanceBuffer(info.m_instanceBuffer);
						renderMeshActor->setMaxInstanceCount(instanceBufferSize);
					}

					info.m_instanceBuffer->writeBuffer(&instanceBufferData[0], 0, instanceBufferSize);
				}
				renderMeshActor->setInstanceBufferRange(0, instanceBufferSize);
				renderMeshActor->updateRenderResources(false, rewriteBuffers, userRenderData);
			}
		}
	}

#if APEX_RUNTIME_FRACTURE
	mRTrenderable.updateRenderResources(rewriteBuffers,userData);
#endif
}

void DestructibleRenderable::dispatchRenderResources(NxUserRenderer& renderer)
{
	PX_PROFILER_PERF_SCOPE("DestructibleRenderableDispatchRenderResources");

	for (PxU32 typeN = 0; typeN < NxDestructibleActorMeshType::Count; ++typeN)
	{
		NxRenderMeshActor* renderMeshActor = mRenderMeshActors[typeN];
		if (renderMeshActor != NULL)
		{
			renderMeshActor->dispatchRenderResources(renderer);
		}
	}

	// Render instanced meshes
	if ((physx::PxI32)mListIndex == mAsset->m_instancingRepresentativeActorIndex)
	{
		for (physx::PxU32 i = 0; i < mAsset->m_instancedChunkRenderMeshActors.size(); ++i)
		{
			if (mAsset->m_instancedChunkRenderMeshActors[i] != NULL)
			{
				mAsset->m_instancedChunkRenderMeshActors[i]->dispatchRenderResources(renderer);
			}
		}

		for (physx::PxU32 i = 0; i < mAsset->m_scatterMeshInstanceInfo.size(); ++i)
		{
			DestructibleAsset::ScatterMeshInstanceInfo& scatterMeshInstanceInfo = mAsset->m_scatterMeshInstanceInfo[i];
			if (scatterMeshInstanceInfo.m_actor != NULL)
			{
				scatterMeshInstanceInfo.m_actor->dispatchRenderResources(renderer);
			}
		}
	}
#if APEX_RUNTIME_FRACTURE
	mRTrenderable.dispatchRenderResources(renderer);
#endif
}

DestructibleRenderable* DestructibleRenderable::incrementReferenceCount()
{
	DestructibleRenderable* returnValue = NULL;
	lockRenderResources();
	if (mRefCount > 0)
	{
		++mRefCount;
		returnValue = this;
	}
	unlockRenderResources();
	return returnValue;
}

}
}
} // end namespace physx::apex
