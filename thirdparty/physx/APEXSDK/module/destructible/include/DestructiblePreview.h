/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEPREVIEW_H__
#define __DESTRUCTIBLEPREVIEW_H__

#include "NxApex.h"
#include "ApexInterface.h"
#include "ApexPreview.h"
#include "NxRenderMesh.h"
#include "NxDestructiblePreview.h"

namespace physx
{
namespace apex
{

class NxApexDestructiblePreviewParam;

namespace destructible
{
class DestructiblePreview : public ApexResource, public ApexPreview
{
public:

	DestructibleAsset*		m_asset;
	NxDestructiblePreview*	m_api;

	physx::PxU32			m_chunkDepth;
	physx::PxF32			m_explodeAmount;

	NxRenderMeshActor*		m_renderMeshActors[NxDestructibleActorMeshType::Count];	// Indexed by NxDestructibleActorMeshType::Enum

	physx::Array<NxRenderMeshActor*>	m_instancedChunkRenderMeshActors;	// One per render mesh actor per instanced chunk
	physx::Array<physx::PxU16>			m_instancedActorVisiblePart;

	physx::Array<physx::PxU16>			m_instancedChunkActorMap;	// from instanced chunk instanceInfo index to actor index

	physx::Array<NxUserRenderInstanceBuffer*>	m_chunkInstanceBuffers;
	physx::Array< physx::Array< DestructibleAsset::ChunkInstanceBufferDataElement > >	m_chunkInstanceBufferData;

	bool					m_drawUnexpandedChunksStatically;

	void*					m_userData;

	void				setExplodeView(physx::PxU32 depth, physx::PxF32 explode);

	void				updateRenderResources(bool rewriteBuffers, void* userRenderData);

	NxRenderMeshActor*	getRenderMeshActor() const
	{
		return m_renderMeshActors[(!m_drawUnexpandedChunksStatically || m_explodeAmount != 0.0f) ? NxDestructibleActorMeshType::Skinned : NxDestructibleActorMeshType::Static];
	}

	void				updateRenderResources(void* userRenderData);
	void				dispatchRenderResources(NxUserRenderer& renderer);

	// ApexPreview methods
	void				setPose(const physx::PxMat44& pose);
	NxApexRenderable*	getRenderable()
	{
		return DYNAMIC_CAST(NxApexRenderable*)(m_api);
	}
	void				release();
	void				destroy();

	DestructiblePreview(NxDestructiblePreview* _api, DestructibleAsset& _asset, const NxParameterized::Interface* params);
	virtual				~DestructiblePreview();

protected:
	void					setChunkVisibility(physx::PxU16 index, bool visibility)
	{
		PX_ASSERT((physx::PxI32)index < m_asset->mParams->chunks.arraySizes[0]);
		if (visibility)
		{
			mVisibleChunks.use(index);
		}
		else
		{
			mVisibleChunks.free(index);
		}
		DestructibleAssetParametersNS::Chunk_Type& sourceChunk = m_asset->mParams->chunks.buf[index];
		if ((sourceChunk.flags & DestructibleAsset::Instanced) == 0)
		{
			// Not instanced - need to choose the static or dynamic mesh, and set visibility for the render mesh actor
			const NxDestructibleActorMeshType::Enum typeN = (m_explodeAmount != 0.0f || !m_drawUnexpandedChunksStatically) ?
					NxDestructibleActorMeshType::Skinned : NxDestructibleActorMeshType::Static;
			m_renderMeshActors[typeN]->setVisibility(visibility, sourceChunk.meshPartIndex);
		}
	}


	void				setInstancedChunkCount(physx::PxU32 count);

	NxIndexBank<physx::PxU16>	mVisibleChunks;
	physx::PxU16				m_instancedChunkCount;
};

}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLEACTOR_H__
