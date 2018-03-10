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

#include "NxApex.h"
#include "ModuleDestructible.h"
#include "DestructiblePreview.h"
#include "DestructibleAsset.h"
#include "DestructiblePreviewParam.h"
#include "NiApexRenderMeshAsset.h"

namespace physx
{
namespace apex
{
namespace destructible
{

DestructiblePreview::DestructiblePreview(NxDestructiblePreview* _api, DestructibleAsset& _asset, const NxParameterized::Interface* params) :
	ApexPreview(),
	m_asset(&_asset),
	m_api(_api),
	m_instancedChunkCount(0)
{
	for (int meshN = 0; meshN < NxDestructibleActorMeshType::Count; ++meshN)
	{
		m_renderMeshActors[meshN] = NULL;
	}

	const DestructiblePreviewParam* destructiblePreviewParams = DYNAMIC_CAST(const DestructiblePreviewParam*)(params);

	PX_ALLOCA(skinnedMaterialNames, const char*, destructiblePreviewParams->overrideSkinnedMaterialNames.arraySizes[0] > 0 ? destructiblePreviewParams->overrideSkinnedMaterialNames.arraySizes[0] : 1);
	for (int i = 0; i < destructiblePreviewParams->overrideSkinnedMaterialNames.arraySizes[0]; ++i)
	{
		skinnedMaterialNames[i] = destructiblePreviewParams->overrideSkinnedMaterialNames.buf[i].buf;
	}

	PX_ASSERT(m_asset->getRenderMeshAsset());
	NxRenderMeshActorDesc renderableMeshDesc;
	renderableMeshDesc.visible = false;
	renderableMeshDesc.indexBufferHint = NxRenderBufferHint::DYNAMIC;
	renderableMeshDesc.keepVisibleBonesPacked = true;
	renderableMeshDesc.overrideMaterials		= destructiblePreviewParams->overrideSkinnedMaterialNames.arraySizes[0] > 0 ? (const char**)skinnedMaterialNames : NULL;
	renderableMeshDesc.overrideMaterialCount	= (physx::PxU32)destructiblePreviewParams->overrideSkinnedMaterialNames.arraySizes[0];
	m_renderMeshActors[NxDestructibleActorMeshType::Skinned] = m_asset->getRenderMeshAsset()->createActor(renderableMeshDesc);
	const physx::PxU32 numParts = m_asset->getRenderMeshAsset()->getPartCount();
	for (physx::PxU32 i = 0; i < numParts; ++i)
	{
		m_renderMeshActors[NxDestructibleActorMeshType::Skinned]->setVisibility(false, (physx::PxU16)i);
	}
	m_renderMeshActors[NxDestructibleActorMeshType::Skinned]->updateBounds();

	m_drawUnexpandedChunksStatically = destructiblePreviewParams->renderUnexplodedChunksStatically;

	if (m_drawUnexpandedChunksStatically)
	{
		PX_ALLOCA(staticMaterialNames, const char*, destructiblePreviewParams->overrideStaticMaterialNames.arraySizes[0] > 0 ? destructiblePreviewParams->overrideStaticMaterialNames.arraySizes[0] : 1);
		for (int i = 0; i < destructiblePreviewParams->overrideStaticMaterialNames.arraySizes[0]; ++i)
		{
			staticMaterialNames[i] = destructiblePreviewParams->overrideStaticMaterialNames.buf[i].buf;
		}

		// Create static render mesh
		renderableMeshDesc.renderWithoutSkinning = true;
		renderableMeshDesc.overrideMaterials		= destructiblePreviewParams->overrideStaticMaterialNames.arraySizes[0] > 0 ? (const char**)staticMaterialNames : NULL;
		renderableMeshDesc.overrideMaterialCount	= (physx::PxU32)destructiblePreviewParams->overrideStaticMaterialNames.arraySizes[0];
		m_renderMeshActors[NxDestructibleActorMeshType::Static] = m_asset->getRenderMeshAsset()->createActor(renderableMeshDesc);
		for (physx::PxU32 i = 0; i < numParts; ++i)
		{
			m_renderMeshActors[NxDestructibleActorMeshType::Static]->setVisibility(false, (physx::PxU16)i);
		}
		m_renderMeshActors[NxDestructibleActorMeshType::Static]->updateBounds();
	}

	// Instanced actors
	physx::Array<physx::PxU16> tempPartToActorMap;
	tempPartToActorMap.resize((physx::PxU32)m_asset->mParams->chunks.arraySizes[0], 0xFFFF);

	m_instancedChunkActorMap.resize((physx::PxU32)m_asset->mParams->chunkInstanceInfo.arraySizes[0]);
	for (physx::PxI32 i = 0; i < m_asset->mParams->chunkInstanceInfo.arraySizes[0]; ++i)
	{
		physx::PxU16 partIndex = m_asset->mParams->chunkInstanceInfo.buf[i].partIndex;
		if (tempPartToActorMap[partIndex] == 0xFFFF)
		{
			tempPartToActorMap[partIndex] = m_instancedChunkCount++;
		}
		m_instancedChunkActorMap[(physx::PxU32)i] = tempPartToActorMap[partIndex];
	}

	m_instancedActorVisiblePart.resize(m_instancedChunkCount);
	for (physx::PxI32 i = 0; i < m_asset->mParams->chunks.arraySizes[0]; ++i)
	{
		DestructibleAssetParametersNS::Chunk_Type& chunk = m_asset->mParams->chunks.buf[i];
		if ((chunk.flags & DestructibleAsset::Instanced) != 0)
		{
			physx::PxU16 partIndex = m_asset->mParams->chunkInstanceInfo.buf[chunk.meshPartIndex].partIndex;
			m_instancedActorVisiblePart[m_instancedChunkActorMap[chunk.meshPartIndex]] = partIndex;
		}
	}

	setChunkVisibility(0, true);

	m_chunkDepth = destructiblePreviewParams->chunkDepth;
	m_explodeAmount = destructiblePreviewParams->explodeAmount;
	setPose(destructiblePreviewParams->globalPose);
	m_userData = reinterpret_cast<void*>(destructiblePreviewParams->userData);
}

DestructiblePreview::~DestructiblePreview()
{
}

// called at the end of 'setPose' which allows for any module specific updates.
void DestructiblePreview::setPose(const physx::PxMat44& pose)
{
	ApexPreview::setPose(pose);
	setExplodeView(m_chunkDepth, m_explodeAmount);
}

void DestructiblePreview::setExplodeView(physx::PxU32 depth, physx::PxF32 explode)
{
	m_chunkDepth = physx::PxClamp(depth, (physx::PxU32)0, m_asset->getDepthCount() - 1);
	const physx::PxF32 newExplodeAmount = physx::PxMax(explode, 0.0f);

	if (m_drawUnexpandedChunksStatically)
	{
		// Using a static mesh for unexploded chunks
		if (m_explodeAmount == 0.0f && newExplodeAmount != 0.0f)
		{
			// Going from unexploded to exploded.  Need to make the static mesh invisible.
			if (m_renderMeshActors[NxDestructibleActorMeshType::Static] != NULL)
			{
				for (physx::PxU32 i = 0; i < m_asset->getChunkCount(); ++i)
				{
					m_renderMeshActors[NxDestructibleActorMeshType::Static]->setVisibility(false, (physx::PxU16)i);
				}
			}
		}
		else if (m_explodeAmount != 0.0f && newExplodeAmount == 0.0f)
		{
			// Going from exploded to unexploded.  Need to make the skinned mesh invisible.
			if (m_renderMeshActors[NxDestructibleActorMeshType::Skinned] != NULL)
			{
				for (physx::PxU32 i = 0; i < m_asset->getChunkCount(); ++i)
				{
					m_renderMeshActors[NxDestructibleActorMeshType::Skinned]->setVisibility(false, (physx::PxU16)i);
				}
			}
		}
	}

	m_explodeAmount = newExplodeAmount;

	for (physx::PxU32 i = 0; i < m_instancedChunkRenderMeshActors.size(); ++i)
	{
		PX_ASSERT(i < m_chunkInstanceBufferData.size());
		m_chunkInstanceBufferData[i].resize(0);
	}

	if (m_asset->getRenderMeshAsset() != NULL)
	{
		physx::PxBounds3 bounds = m_asset->getRenderMeshAsset()->getBounds();
		physx::PxVec3 c = bounds.getCenter();

		for (physx::PxU16 i = 0; i < (physx::PxU16)m_asset->getChunkCount(); ++i)
		{
			setChunkVisibility(i, m_asset->mParams->chunks.buf[i].depth == m_chunkDepth);
		}

		DestructibleAssetParametersNS::Chunk_Type* sourceChunks = m_asset->mParams->chunks.buf;

		mRenderBounds.setEmpty();

		// Iterate over all visible chunks
		const physx::PxU16* indexPtr = mVisibleChunks.usedIndices();
		const physx::PxU16* indexPtrStop = indexPtr + mVisibleChunks.usedCount();
		DestructibleAssetParametersNS::InstanceInfo_Type* instanceDataArray = m_asset->mParams->chunkInstanceInfo.buf;	
		while (indexPtr < indexPtrStop)
		{
			const physx::PxU16 index = *indexPtr++;
			if (index < m_asset->getChunkCount())
			{
				DestructibleAssetParametersNS::Chunk_Type& sourceChunk = sourceChunks[index];
				physx::PxMat44 pose = mPose;
				if ((sourceChunk.flags & DestructibleAsset::Instanced) == 0)
				{
					// Not instanced - only need to set skinned tms from chunks
					if (!m_drawUnexpandedChunksStatically || m_explodeAmount != 0.0f)
					{
						physx::PxBounds3 partBounds = m_asset->getRenderMeshAsset()->getBounds(sourceChunk.meshPartIndex);
						physx::PxVec3 partC = partBounds.getCenter();
						pose.setPosition(pose.getPosition() + m_explodeAmount*pose.rotate(partC-c));
						m_renderMeshActors[NxDestructibleActorMeshType::Skinned]->setTM(pose, sourceChunk.meshPartIndex);
					}
				}
				else
				{
					// Instanced
					PX_ASSERT(sourceChunk.meshPartIndex < m_asset->mParams->chunkInstanceInfo.arraySizes[0]);
					DestructibleAsset::ChunkInstanceBufferDataElement instanceDataElement;
					DestructibleAssetParametersNS::InstanceInfo_Type& instanceData = instanceDataArray[sourceChunk.meshPartIndex];
					const physx::PxU16 instancedActorIndex = m_instancedChunkActorMap[sourceChunk.meshPartIndex];
					physx::Array<DestructibleAsset::ChunkInstanceBufferDataElement>& instanceBufferData = m_chunkInstanceBufferData[instancedActorIndex];
					instanceDataElement.scaledRotation = physx::PxMat33(pose.getBasis(0), pose.getBasis(1), pose.getBasis(2));
					const physx::PxVec3 globalOffset = instanceDataElement.scaledRotation*instanceData.chunkPositionOffset;
					physx::PxBounds3 partBounds = m_asset->getRenderMeshAsset()->getBounds(instanceData.partIndex);
					partBounds.minimum += globalOffset;
					partBounds.maximum += globalOffset;
					physx::PxVec3 partC = partBounds.getCenter();
					instanceDataElement.translation = pose.getPosition() + globalOffset + m_explodeAmount*pose.rotate(partC-c);
					instanceDataElement.uvOffset = instanceData.chunkUVOffset;
					instanceDataElement.localOffset = instanceData.chunkPositionOffset;
					instanceBufferData.pushBack(instanceDataElement);
					// Transform bounds
					physx::PxVec3 center, extents;
					center = partBounds.getCenter();
					extents = partBounds.getExtents();
					center = instanceDataElement.scaledRotation.transform(center) + instanceDataElement.translation;
					extents = physx::PxVec3(physx::PxAbs(instanceDataElement.scaledRotation(0, 0) * extents.x) + physx::PxAbs(instanceDataElement.scaledRotation(0, 1) * extents.y) + physx::PxAbs(instanceDataElement.scaledRotation(0, 2) * extents.z),
											physx::PxAbs(instanceDataElement.scaledRotation(1, 0) * extents.x) + physx::PxAbs(instanceDataElement.scaledRotation(1, 1) * extents.y) + physx::PxAbs(instanceDataElement.scaledRotation(1, 2) * extents.z),
											physx::PxAbs(instanceDataElement.scaledRotation(2, 0) * extents.x) + physx::PxAbs(instanceDataElement.scaledRotation(2, 1) * extents.y) + physx::PxAbs(instanceDataElement.scaledRotation(2, 2) * extents.z));
					mRenderBounds.include(physx::PxBounds3::centerExtents(center, extents));
				}
			}
		}

		m_renderMeshActors[NxDestructibleActorMeshType::Skinned]->updateBounds();
		mRenderBounds.include(m_renderMeshActors[NxDestructibleActorMeshType::Skinned]->getBounds());

		// If a static mesh exists, set its (single) tm from the destructible's tm
		if (m_renderMeshActors[NxDestructibleActorMeshType::Static] != NULL)
		{
			m_renderMeshActors[NxDestructibleActorMeshType::Static]->syncVisibility();
			m_renderMeshActors[NxDestructibleActorMeshType::Static]->setTM(mPose);
			m_renderMeshActors[NxDestructibleActorMeshType::Static]->updateBounds();
			mRenderBounds.include(m_renderMeshActors[NxDestructibleActorMeshType::Static]->getBounds());
		}
	}
}

void DestructiblePreview::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	for (physx::PxU32 i = 0; i < NxDestructibleActorMeshType::Count; ++i)
	{
		if (m_renderMeshActors[i] != NULL)
		{
			NiApexRenderMeshActor* renderMeshActor = (NiApexRenderMeshActor*)m_renderMeshActors[i];
			renderMeshActor->updateRenderResources((i == NxDestructibleActorMeshType::Skinned), rewriteBuffers, userRenderData);
		}
	}

	setInstancedChunkCount(m_instancedChunkCount);

	for (physx::PxU32 i = 0; i < m_instancedChunkRenderMeshActors.size(); ++i)
	{
		PX_ASSERT(i < m_chunkInstanceBufferData.size());
		NiApexRenderMeshActor* renderMeshActor = (NiApexRenderMeshActor*)m_instancedChunkRenderMeshActors[i];
		if (renderMeshActor != NULL)
		{
			physx::Array<DestructibleAsset::ChunkInstanceBufferDataElement>& instanceBufferData = m_chunkInstanceBufferData[i];
			const physx::PxU32 instanceBufferSize = instanceBufferData.size();
			if (instanceBufferSize > 0)
			{
				m_chunkInstanceBuffers[i]->writeBuffer(&instanceBufferData[0], 0, instanceBufferSize);
			}
			renderMeshActor->setInstanceBufferRange(0, instanceBufferSize);
			renderMeshActor->updateRenderResources(false, rewriteBuffers, userRenderData);
		}
	}
}

void DestructiblePreview::dispatchRenderResources(NxUserRenderer& renderer)
{
	for (physx::PxU32 i = 0; i < NxDestructibleActorMeshType::Count; ++i)
	{
		if (m_renderMeshActors[i] != NULL)
		{
			m_renderMeshActors[i]->dispatchRenderResources(renderer);
		}
	}

	for (physx::PxU32 i = 0; i < m_instancedChunkRenderMeshActors.size(); ++i)
	{
		PX_ASSERT(i < m_chunkInstanceBufferData.size());
		if (m_instancedChunkRenderMeshActors[i] != NULL)
		{
			if (m_chunkInstanceBufferData[i].size() > 0)
			{
				m_instancedChunkRenderMeshActors[i]->dispatchRenderResources(renderer);
			}
		}
	}
}

void DestructiblePreview::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	destroy();
}

void DestructiblePreview::destroy()
{
	ApexPreview::destroy();

	for (physx::PxU32 i = 0; i < NxDestructibleActorMeshType::Count; ++i)
	{
		if (m_renderMeshActors[i] != NULL)
		{
			m_renderMeshActors[i]->release();
		}
	}

	setInstancedChunkCount(0);
}

void DestructiblePreview::setInstancedChunkCount(physx::PxU32 count)
{
	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();

	const physx::PxU32 oldCount = m_instancedChunkRenderMeshActors.size();
	for (physx::PxU32 i = count; i < oldCount; ++i)
	{
		if (m_instancedChunkRenderMeshActors[i] != NULL)
		{
			m_instancedChunkRenderMeshActors[i]->release();
			m_instancedChunkRenderMeshActors[i] = NULL;
		}
		if (m_chunkInstanceBuffers[i] != NULL)
		{
			rrm->releaseInstanceBuffer(*m_chunkInstanceBuffers[i]);
			m_chunkInstanceBuffers[i] = NULL;
		}
	}
	m_instancedChunkRenderMeshActors.resize(count);
	m_chunkInstanceBuffers.resize(count);
	m_chunkInstanceBufferData.resize(count);
	for (physx::PxU32 index = oldCount; index < count; ++index)
	{
		m_instancedChunkRenderMeshActors[index] = NULL;
		m_chunkInstanceBuffers[index] = NULL;
		m_chunkInstanceBufferData[index].reset();

		// Find out how many potential instances there are
		physx::PxU32 maxInstanceCount = 0;
		for (physx::PxI32 i = 0; i < m_asset->mParams->chunkInstanceInfo.arraySizes[0]; ++i)
		{
			if (m_asset->mParams->chunkInstanceInfo.buf[i].partIndex == m_instancedActorVisiblePart[index])
			{
				++maxInstanceCount;
			}
		}

		// Create instance buffer
		NxUserRenderInstanceBufferDesc instanceBufferDesc;
		instanceBufferDesc.maxInstances = maxInstanceCount;
		instanceBufferDesc.hint = NxRenderBufferHint::DYNAMIC;
		instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::POSITION_FLOAT3] = DestructibleAsset::ChunkInstanceBufferDataElement::translationOffset();
		instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3] = DestructibleAsset::ChunkInstanceBufferDataElement::scaledRotationOffset();
		instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::UV_OFFSET_FLOAT2] = DestructibleAsset::ChunkInstanceBufferDataElement::uvOffsetOffset();
		instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::LOCAL_OFFSET_FLOAT3] = DestructibleAsset::ChunkInstanceBufferDataElement::localOffsetOffset();
		instanceBufferDesc.stride = sizeof(DestructibleAsset::ChunkInstanceBufferDataElement);
		m_chunkInstanceBuffers[index] = rrm->createInstanceBuffer(instanceBufferDesc);

		// Instance buffer data
		m_chunkInstanceBufferData[index].reserve(maxInstanceCount);
		m_chunkInstanceBufferData[index].resize(0);

		// Create actor
		if (m_asset->renderMeshAsset != NULL)
		{
			NxRenderMeshActorDesc renderableMeshDesc;
			renderableMeshDesc.maxInstanceCount = maxInstanceCount;
			renderableMeshDesc.renderWithoutSkinning = true;
			renderableMeshDesc.visible = false;
			m_instancedChunkRenderMeshActors[index] = m_asset->renderMeshAsset->createActor(renderableMeshDesc);
			m_instancedChunkRenderMeshActors[index]->setInstanceBuffer(m_chunkInstanceBuffers[index]);
			m_instancedChunkRenderMeshActors[index]->setVisibility(true, m_instancedActorVisiblePart[index]);
		}
	}
}

}
}
} // end namespace physx::apex

#endif
