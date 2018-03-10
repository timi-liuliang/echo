/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#include "DestructibleActor.h"

#include "Actor.h"
#include "Compound.h"
#include "Convex.h"

#include "Renderable.h"

namespace physx
{
namespace fracture
{

Renderable::Renderable():
	mVertexBuffer(NULL),
	mIndexBuffer(NULL),
	mBoneBuffer(NULL),
	mVertexBufferSize(0),
	mIndexBufferSize(0),
	mBoneBufferSize(0),
	mVertexBufferSizeLast(0),
	mIndexBufferSizeLast(0),
	mBoneBufferSizeLast(0),
	mFullBufferDirty(true),
	valid(false)
{

}

Renderable::~Renderable()
{
	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();
	if (mVertexBuffer != NULL )
	{
		rrm->releaseVertexBuffer(*mVertexBuffer);
	}
	if (mIndexBuffer != NULL )
	{
		rrm->releaseIndexBuffer(*mIndexBuffer);
	}
	if (mBoneBuffer != NULL)
	{
		rrm->releaseBoneBuffer(*mBoneBuffer);
	}
	for (PxU32 k = 0; k < mConvexGroups.size(); k++)
	{
		ConvexGroup& g = mConvexGroups[k];
		for (PxU32 j = 0; j < g.mSubMeshes.size(); j++)
		{
			SubMesh& s = g.mSubMeshes[j];
			if(s.renderResource != NULL)
			{
				rrm->releaseResource(*s.renderResource);
				s.renderResource = NULL;
			}
		}
	}
}

void Renderable::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();
	NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();

	PX_ASSERT(rrm != NULL && nrp != NULL);
	if (rrm == NULL || nrp == NULL || mConvexGroups.empty())
	{
		valid = false;
		return;
	}

	if (rewriteBuffers)
	{
		mFullBufferDirty = true;
	}

	// Resize buffers if necessary: TODO: intelligently oversize
	// vertex buffer
	if (mVertexBufferSize > mVertexBufferSizeLast)
	{
		if (mVertexBuffer != NULL )
		{
			rrm->releaseVertexBuffer(*mVertexBuffer);
		}
		{
			NxUserRenderVertexBufferDesc desc;
			desc.uvOrigin = physx::apex::NxTextureUVOrigin::ORIGIN_BOTTOM_LEFT;
			desc.hint = NxRenderBufferHint::DYNAMIC;
			desc.maxVerts = mVertexBufferSize;
			desc.buffersRequest[NxRenderVertexSemantic::POSITION]   = NxRenderDataFormat::FLOAT3;
			desc.buffersRequest[NxRenderVertexSemantic::NORMAL]     = NxRenderDataFormat::FLOAT3;
			desc.buffersRequest[NxRenderVertexSemantic::TEXCOORD0]  = NxRenderDataFormat::FLOAT2;
			desc.buffersRequest[NxRenderVertexSemantic::BONE_INDEX] = NxRenderDataFormat::USHORT1;
			mVertexBuffer = rrm->createVertexBuffer(desc);
			PX_ASSERT(mVertexBuffer);
		}
		mFullBufferDirty = true;
	}
	// index buffer
	if (mIndexBufferSize > mIndexBufferSizeLast)
	{
		if (mIndexBuffer != NULL )
		{
			rrm->releaseIndexBuffer(*mIndexBuffer);
		}
		NxUserRenderIndexBufferDesc desc;
		desc.hint = NxRenderBufferHint::DYNAMIC;
		desc.maxIndices = mIndexBufferSize;
		desc.format = NxRenderDataFormat::UINT1;
		mIndexBuffer = rrm->createIndexBuffer(desc);
		PX_ASSERT(mIndexBuffer);
		mFullBufferDirty = true;
	}
	// bone buffer
	if (mBoneBufferSize > mBoneBufferSizeLast)
	{
		if (mBoneBuffer != NULL)
		{
			rrm->releaseBoneBuffer(*mBoneBuffer);
		}
		NxUserRenderBoneBufferDesc desc;
		desc.hint = NxRenderBufferHint::DYNAMIC;
		desc.maxBones = mBoneBufferSize;
		desc.buffersRequest[NxRenderBoneSemantic::POSE] = NxRenderDataFormat::FLOAT3x4;
		mBoneBuffer = rrm->createBoneBuffer(desc);
		PX_ASSERT(mBoneBuffer);
		mFullBufferDirty = true;
	}
	// Fill buffers
	if (mFullBufferDirty)
	{
		PxU32 vertexIdx = 0;
		PxU32 indexIdx = 0;
		PxU32 boneIdx = 0;
		for (PxU32 k = 0; k < mConvexGroups.size(); k++)
		{
			ConvexGroup& g = mConvexGroups[k];
			for (PxU32 j = 0; j < g.mSubMeshes.size(); j++)
			{
				SubMesh& s = g.mSubMeshes[j];
				if(s.renderResource != NULL)
				{
					rrm->releaseResource(*s.renderResource);
					s.renderResource = NULL;
				}
				NxUserRenderResourceDesc desc;
				desc.primitives = NxRenderPrimitiveType::TRIANGLES;
				// configure vertices
				desc.vertexBuffers = &mVertexBuffer;
				desc.numVertexBuffers = 1;
				desc.numVerts = g.mVertexCache.size();
				desc.firstVertex = vertexIdx;
				// configure indices;
				desc.indexBuffer = mIndexBuffer;
				desc.firstIndex = indexIdx;
				desc.numIndices = s.mIndexCache.size();
				// configure bones;
				desc.boneBuffer = mBoneBuffer;
				desc.numBones = g.mBoneCache.size();
				desc.firstBone = boneIdx;
				// configure other info
				desc.material = nrp->getResource(mMaterialInfo[j].mMaterialID);
				desc.submeshIndex = j;
				desc.userRenderData = userRenderData;
				// create 
				s.renderResource = rrm->createResource(desc);
				PX_ASSERT(s.renderResource);
				// copy indices into buffer
				PX_ASSERT(indexIdx+s.mIndexCache.size() <= mIndexBufferSize);
				mIndexBuffer->writeBuffer(s.mIndexCache.begin(),sizeof(*s.mIndexCache.begin()),indexIdx,s.mIndexCache.size());
				indexIdx += s.mIndexCache.size();
			}
			// copy vertices and bones
			{
				NxApexRenderVertexBufferData data;
				data.setSemanticData(NxRenderVertexSemantic::POSITION,  g.mVertexCache.begin(),   sizeof(*g.mVertexCache.begin()),   NxRenderDataFormat::FLOAT3);
				data.setSemanticData(NxRenderVertexSemantic::NORMAL,    g.mNormalCache.begin(),   sizeof(*g.mNormalCache.begin()),   NxRenderDataFormat::FLOAT3);
				data.setSemanticData(NxRenderVertexSemantic::TEXCOORD0, g.mTexcoordCache.begin(), sizeof(*g.mTexcoordCache.begin()), NxRenderDataFormat::FLOAT2);
				data.setSemanticData(NxRenderVertexSemantic::BONE_INDEX,g.mBoneIndexCache.begin(),sizeof(*g.mBoneIndexCache.begin()),NxRenderDataFormat::USHORT1);
				PX_ASSERT(vertexIdx + g.mVertexCache.size() <= mVertexBufferSize);
				mVertexBuffer->writeBuffer(data,vertexIdx,g.mVertexCache.size());
			}
			{
				NxApexRenderBoneBufferData data;
				data.setSemanticData(NxRenderBoneSemantic::POSE,g.mBoneCache.begin(),sizeof(*g.mBoneCache.begin()),NxRenderDataFormat::FLOAT3x4);
				PX_ASSERT(boneIdx + g.mBoneCache.size() <= mBoneBufferSize);
				mBoneBuffer->writeBuffer(data,boneIdx,g.mBoneCache.size());
			}
			vertexIdx += g.mVertexCache.size();
			boneIdx += g.mBoneCache.size();
		}
		mFullBufferDirty = false;
	}
	else // Bones only
	{
		PxU32 boneIdx = 0;
		for (PxU32 k = 0; k < mConvexGroups.size(); k++)
		{
			ConvexGroup& g = mConvexGroups[k];
			{
				NxApexRenderBoneBufferData data;
				data.setSemanticData(NxRenderBoneSemantic::POSE,g.mBoneCache.begin(),sizeof(*g.mBoneCache.begin()),NxRenderDataFormat::FLOAT3x4);
				mBoneBuffer->writeBuffer(data,boneIdx,g.mBoneCache.size());
			}
			boneIdx += g.mBoneCache.size();
		}
	}
	mVertexBufferSizeLast = mVertexBufferSize;
	mIndexBufferSizeLast = mIndexBufferSize;
	mBoneBufferSizeLast = mBoneBufferSize;
	valid = true;
}

void Renderable::dispatchRenderResources(NxUserRenderer& api)
{
	if (!valid)
	{
		return;
	}
	NxApexRenderContext ctx;
	ctx.local2world = PxMat44::createIdentity();
	ctx.world2local = PxMat44::createIdentity();
	for (PxU32 k = 0; k < mConvexGroups.size(); k++)
	{
		ConvexGroup& g = mConvexGroups[k];
		for (PxU32 j = 0; j < g.mSubMeshes.size(); j++)
		{
			SubMesh& s = g.mSubMeshes[j];
			if(s.renderResource && !s.mIndexCache.empty())
			{
				ctx.renderResource = s.renderResource;
				api.renderResource(ctx);
			}
		}
	}
}

// -----------------------Cache Update-----------------------------------
void Renderable::updateRenderCacheFull(Actor* actor)
{
	mVertexBufferSize = 0;
	mIndexBufferSize = 0;
	mBoneBufferSize = 0;
	// Resize SubMeshes if necessary
	const PxU32 numSubMeshes = actor->mActor->getRenderSubmeshCount();
	if( numSubMeshes == 0 )
	{
		return;
	}
	if( numSubMeshes != mMaterialInfo.size() )
	{
		mMaterialInfo.resize(numSubMeshes);
	}
	// grab material information
	if (NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider())
	{
		if (NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager())
		{
			NxResID materialNS = NiGetApexSDK()->getMaterialNameSpace();
			for(PxU32 i = 0; i < numSubMeshes; i++)
			{
				if(mMaterialInfo[i].mMaxBones == 0)
				{
					mMaterialInfo[i].mMaterialID = nrp->createResource(materialNS,actor->mActor->getAsset()->getRenderMeshAsset()->getMaterialName(i),false);
					mMaterialInfo[i].mMaxBones = rrm->getMaxBonesForMaterial(nrp->getResource(mMaterialInfo[i].mMaterialID));
				}
			}
		}
	}
	// Find bone limit
	PxU32 maxBones = mMaterialInfo[0].mMaxBones;
	for (PxU32 i = 1; i < mMaterialInfo.size(); i++)
	{
		if (mMaterialInfo[i].mMaxBones < maxBones)
		{
			maxBones = mMaterialInfo[i].mMaxBones;
		}
	}
	//maxBones = 1; // TEMPORARY: FIXES TEXTURE MAPPING PROBLEM
	//maxBones = rand(1,maxBones-1);
	// Count Convexes
	PxU32 numConvexes = 0;
	const Array<base::Compound*>& compounds = actor->getCompounds();
	for (PxU32 k = 0; k < compounds.size(); k++)
	{
		const Array<base::Convex*>& convexes = compounds[k]->getConvexes();
		numConvexes += convexes.size();
	}
	mBoneBufferSize += numConvexes;
	
	//maxBones is 0 when VTF rendering method is used for destructible(only)
	if(maxBones == 0)
		maxBones = mBoneBufferSize;

	// Create more groups if necessary
	PxU32 numGroups = numConvexes/maxBones + ((numConvexes%maxBones)?1:0);
	if (numGroups != mConvexGroups.size())
	{
		mConvexGroups.resize(numGroups);
	}
	// Resize convex caches and subMeshes if necessary
	for (PxU32 k = 0; k < mConvexGroups.size(); k++)
	{
		ConvexGroup& g = mConvexGroups[k];
		g.mConvexCache.clear();
		if( g.mConvexCache.capacity() <= maxBones )
		{
			g.mConvexCache.reserve(maxBones);
		}
		if( g.mSubMeshes.size() != mMaterialInfo.size())
		{
			g.mSubMeshes.resize(mMaterialInfo.size());
		}
	}
	// Populate convex cache
	{
		PxU32 idx = 0;
		const Array<base::Compound*>& compounds = actor->getCompounds();
		for (PxU32 k = 0; k < compounds.size(); k++)
		{
			const Array<base::Convex*>& convexes = compounds[k]->getConvexes();
			for (PxU32 j = 0; j < convexes.size(); j++)
			{
				mConvexGroups[idx/maxBones].mConvexCache.pushBack((Convex*)convexes[j]);
				idx++;
			}
		}
	}
	// Fill other caches
	for (PxU32 k = 0; k < mConvexGroups.size(); k++)
	{
		ConvexGroup& g = mConvexGroups[k];
		// Calculate number of vertices
		PxU32 numVertices = 0;
		for (PxU32 j = 0; j < g.mConvexCache.size(); j++)
		{
			numVertices += g.mConvexCache[j]->getVisVertices().size();
		}
		mVertexBufferSize += numVertices;
		// Resize if necessary
		g.mVertexCache.clear();
		g.mNormalCache.clear();
		g.mTexcoordCache.clear();
		g.mBoneIndexCache.clear();
		if (numVertices >= g.mVertexCache.capacity())
		{
			g.mVertexCache.reserve(numVertices);
			g.mNormalCache.reserve(numVertices);
			g.mBoneIndexCache.reserve(numVertices);
			g.mTexcoordCache.reserve(numVertices);
		}
		g.mBoneCache.clear();
		if (maxBones >= g.mBoneCache.capacity())
		{
			g.mBoneCache.reserve(maxBones);
		}
		// Calculate index buffer sizes
		for (PxU32 i = 0; i < g.mSubMeshes.size(); i++)
		{
			PxU32 numIndices = 0;
			for (PxU32 j = 0; j < g.mConvexCache.size(); j++)
			{
				numIndices += g.mConvexCache[j]->getVisTriIndices().size();			
			}	
			g.mSubMeshes[i].mIndexCache.clear();
			if (numIndices >= g.mSubMeshes[i].mIndexCache.capacity())
			{
				g.mSubMeshes[i].mIndexCache.reserve(numIndices);
			}
			mIndexBufferSize += numIndices;
		}
		// Fill for each convex
		for (PxU32 j = 0; j < g.mConvexCache.size(); j++)
		{
			Convex* c = g.mConvexCache[j];
			PxU32 off = g.mVertexCache.size();
			// fill vertices
			const Array<PxVec3>& verts = c->getVisVertices();
			const Array<PxVec3>& norms = c->getVisNormals();
			const Array<PxF32>& texcs = c->getVisTexCoords();
			PX_ASSERT(verts.size() == norms.size() && verts.size() == (texcs.size()/2));
			for (PxU32 i = 0; i < verts.size(); i++)
			{
				g.mVertexCache.pushBack(verts[i]);
				g.mNormalCache.pushBack(norms[i]);
				g.mTexcoordCache.pushBack(PxVec2(texcs[2*i],texcs[2*i+1]));
				g.mBoneIndexCache.pushBack((PxU16)j);
			}
			// fill indicies for each submesh
			for (PxU32 i = 0; i < g.mSubMeshes.size(); i++)
			{
				const Array<PxI32>& indices = c->getVisTriIndices();
				PX_ASSERT(indices.size()%3 == 0);
				for (PxU32 a = 0; a < indices.size()/3; a++)
				{
					PxU32 subMeshID = 0;														// <<<--- TODO: acquire subMeshID for triangle
					if (subMeshID == i)
					{
						g.mSubMeshes[i].mIndexCache.pushBack(indices[3*a+0]+off);
						g.mSubMeshes[i].mIndexCache.pushBack(indices[3*a+1]+off);
						g.mSubMeshes[i].mIndexCache.pushBack(indices[3*a+2]+off);
					}					
				}
			}
			g.mBoneCache.pushBack((PxMat34Legacy)c->getGlobalPose());
		}
	}
}

void Renderable::updateRenderCache(Actor* actor)
{
	if( actor == NULL ) return;
	actor->mScene->getScene()->lockRead();
	//actor->mRenderResourcesDirty = true;
	if( actor->mRenderResourcesDirty)
	{
		updateRenderCacheFull(actor);
		mFullBufferDirty = true;
		actor->mRenderResourcesDirty = false;
	}
	// Fill other caches
	for (PxU32 k = 0; k < mConvexGroups.size(); k++)
	{
		ConvexGroup& g = mConvexGroups[k];
		g.mBoneCache.clear();
		// Fill bones for each convex
		for (PxU32 j = 0; j < g.mConvexCache.size(); j++)
		{
			Convex* c = g.mConvexCache[j];
			g.mBoneCache.pushBack((PxMat34Legacy)c->getGlobalPose());
		}
	}
	actor->mScene->getScene()->unlockRead();
}

PxBounds3 Renderable::getBounds() const
{
	PxBounds3 bounds;
	bounds.setEmpty();
	for (PxU32 k = 0; k < mConvexGroups.size(); k++)
	{
		const ConvexGroup& g = mConvexGroups[k];
		for (PxU32 j = 0; j < g.mConvexCache.size(); j++)
		{
			const Convex* c = g.mConvexCache[j];
			bounds.include(c->getBounds());
		}
	}
	return bounds;
}

}
}
#endif