/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexRenderMeshActor.h"
#include "FrameworkPerfScope.h"
#include "PsAlloca.h"
#include "NiApexRenderDebug.h"
#include "NxFromPx.h"
#include "DebugRenderParams.h"

#include "NxApexRenderContext.h"
#include "NxRenderMeshActorDesc.h"
#include "NxUserRenderBoneBuffer.h"
#include "NxUserRenderBoneBufferDesc.h"
#include "NxUserRenderIndexBuffer.h"
#include "NxUserRenderIndexBufferDesc.h"
#include "NxUserRenderResource.h"
#include "NxUserRenderResourceDesc.h"
#include "NxUserRenderVertexBuffer.h"
#include "NxUserRenderer.h"

#include "foundation/PxMemory.h"

#define VERBOSE 0

namespace physx
{
namespace apex
{
// ApexRenderMeshActor methods

ApexRenderMeshActor::ApexRenderMeshActor(const NxRenderMeshActorDesc& desc, ApexRenderMeshAsset& asset, NxResourceList& list) :
	mRenderMeshAsset(&asset),
	mIndexBufferHint(NxRenderBufferHint::STATIC),
	mMaxInstanceCount(0),
	mInstanceCount(0),
	mInstanceOffset(0),
	mInstanceBuffer(NULL),
	mRenderResource(NULL),
	mRenderWithoutSkinning(false),
	mForceBoneIndexChannel(false),
	mApiVisibilityChanged(false),
	mPartVisibilityChanged(false),
	mInstanceCountChanged(false),
	mKeepVisibleBonesPacked(false),
	mForceFallbackSkinning(false),
	mBonePosesDirty(false),
	mOneUserVertexBufferChanged(false),
	mBoneBufferInUse(false),
	mReleaseResourcesIfNothingToRender(true),
	mCreateRenderResourcesAfterInit(false),
	mBufferVisibility(false),
	mKeepPreviousFrameBoneBuffer(false),
	mPreviousFrameBoneBufferValid(false),
	mSkinningMode(NxRenderMeshActorSkinningMode::Default)
{
#if ENABLE_INSTANCED_MESH_CLEANUP_HACK
	mOrderedInstanceTemp     = 0;
	mOrderedInstanceTempSize = 0;
#endif
	list.add(*this);
	init(desc, (physx::PxU16) asset.getPartCount(), (physx::PxU16) asset.getBoneCount());
}

ApexRenderMeshActor::~ApexRenderMeshActor()
{
#if ENABLE_INSTANCED_MESH_CLEANUP_HACK
	if (mOrderedInstanceTemp)
	{
		PX_FREE(mOrderedInstanceTemp);
		mOrderedInstanceTemp = 0;
	}
#endif
}

void ApexRenderMeshActor::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	mRenderMeshAsset->releaseActor(*this);
}

void ApexRenderMeshActor::destroy()
{
	ApexActor::destroy();

	releaseRenderResources();

	// Release named resources
	NiResourceProvider* resourceProvider = NiGetApexSDK()->getInternalResourceProvider();
	if (resourceProvider != NULL)
	{
		for (PxU32 i = 0; i < mSubmeshData.size(); i++)
		{
			if (mSubmeshData[i].materialID != mRenderMeshAsset->mMaterialIDs[i])
			{
				resourceProvider->releaseResource(mSubmeshData[i].materialID);
			}
			mSubmeshData[i].materialID = INVALID_RESOURCE_ID;
			mSubmeshData[i].material = NULL;
			mSubmeshData[i].isMaterialPointerValid = false;
		}
	}

	delete this;
}


void ApexRenderMeshActor::loadMaterial(SubmeshData& submeshData)
{
	NiResourceProvider* resourceProvider = NiGetApexSDK()->getInternalResourceProvider();
	submeshData.material = resourceProvider->getResource(submeshData.materialID);
	submeshData.isMaterialPointerValid = true;

	if (submeshData.material != NULL)
	{
		NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();
		submeshData.maxBonesPerMaterial = rrm->getMaxBonesForMaterial(submeshData.material);

		if (submeshData.maxBonesPerMaterial == 0)
		{
			submeshData.maxBonesPerMaterial = mRenderMeshAsset->getBoneCount();
		}
	}
}


void ApexRenderMeshActor::init(const NxRenderMeshActorDesc& desc, PxU16 partCount, PxU16 boneCount)
{
	// TODO - LRR - This happened once, it shouldn't happen any more, let me know if it does
	//PX_ASSERT(boneCount != 0);
	//if (boneCount == 0)
	//	boneCount = partCount;

	mRenderWithoutSkinning = desc.renderWithoutSkinning;
	mForceBoneIndexChannel = desc.forceBoneIndexChannel;

	if (mRenderWithoutSkinning)
	{
		boneCount = 1;
	}

	const PxU32 oldBoneCount = mTransforms.size();
	mTransforms.resize(boneCount);
	for (PxU32 i = oldBoneCount; i < boneCount; ++i)
	{
		mTransforms[i] = PxMat44::createIdentity();
	}

	mVisiblePartsForAPI.reserve(partCount);
	mVisiblePartsForAPI.lockCapacity(true);
	mVisiblePartsForRendering.reserve(partCount);
	mVisiblePartsForRendering.reset();	// size to 0, without reallocating

	mBufferVisibility = desc.bufferVisibility;

	mKeepPreviousFrameBoneBuffer = desc.keepPreviousFrameBoneBuffer;
	mPreviousFrameBoneBufferValid = false;

	mApiVisibilityChanged = true;
	mPartVisibilityChanged = !mBufferVisibility;

	for (physx::PxU32 i = 0; i < partCount; ++i)
	{
		setVisibility(desc.visible, (physx::PxU16) i);
	}

	mIndexBufferHint  = desc.indexBufferHint;
	mMaxInstanceCount = desc.maxInstanceCount;
	mInstanceCount    = 0;
	mInstanceOffset   = 0;
	mInstanceBuffer   = NULL;

	if (desc.keepVisibleBonesPacked && !mRenderWithoutSkinning)
	{
		if (mRenderMeshAsset->getBoneCount() == mRenderMeshAsset->getPartCount())
		{
			mKeepVisibleBonesPacked = true;
		}
		else
		{
			APEX_INVALID_PARAMETER("NxRenderMeshActorDesc::keepVisibleBonesPacked is only allowed when the number of bones (%d) equals the number of parts (%d)\n",
			                       mRenderMeshAsset->getBoneCount(), mRenderMeshAsset->getPartCount());
		}
	}

	if (desc.forceFallbackSkinning)
	{
		if (mKeepVisibleBonesPacked)
		{
			APEX_INVALID_PARAMETER("NxRenderMeshActorDesc::forceFallbackSkinning can not be used when NxRenderMeshActorDesc::keepVisibleBonesPacked is also used!\n");
		}
		else
		{
			mForceFallbackSkinning = true;
		}
	}

	NiResourceProvider* resourceProvider = NiGetApexSDK()->getInternalResourceProvider();
	NxResID materialNS = NiGetApexSDK()->getMaterialNameSpace();

	bool loadMaterialsOnActorCreation = !NiGetApexSDK()->getRMALoadMaterialsLazily();

	if (mRenderWithoutSkinning)
	{
		// make sure that createRenderResources() is called in this special case!
		mCreateRenderResourcesAfterInit = true;
	}

	mSubmeshData.reserve(mRenderMeshAsset->getSubmeshCount());
	for (PxU32 submeshIndex = 0; submeshIndex < mRenderMeshAsset->getSubmeshCount(); submeshIndex++)
	{
		SubmeshData submeshData;

		// Resolve override material names using the NRP...
		if (submeshIndex < desc.overrideMaterialCount && resourceProvider != NULL)
		{
			submeshData.materialID = resourceProvider->createResource(materialNS, desc.overrideMaterials[submeshIndex]);;
		}
		else
		{
			submeshData.materialID = mRenderMeshAsset->mMaterialIDs[submeshIndex];
		}

		submeshData.maxBonesPerMaterial = 0;

		if (loadMaterialsOnActorCreation)
		{
			loadMaterial(submeshData);
		}

		mSubmeshData.pushBack(submeshData);
	}
}

bool ApexRenderMeshActor::setVisibility(bool visible, PxU16 partIndex)
{
	NX_WRITE_ZONE();
	PxU32 oldRank = PX_MAX_U32, newRank = PX_MAX_U32;

	bool changed;

	if (visible)
	{
		changed = mVisiblePartsForAPI.useAndReturnRanks(partIndex, newRank, oldRank);
		if (changed)
		{
			mApiVisibilityChanged = true;
			if (!mBufferVisibility)
			{
				mPartVisibilityChanged = true;
			}
			if (mKeepVisibleBonesPacked && newRank != oldRank)
			{
				mTMSwapBuffer.pushBack(newRank << 16 | oldRank);
			}
		}
	}
	else
	{
		changed = mVisiblePartsForAPI.freeAndReturnRanks(partIndex, newRank, oldRank);
		if (changed)
		{
			mApiVisibilityChanged = true;
			if (!mBufferVisibility)
			{
				mPartVisibilityChanged = true;
			}
			if (mKeepVisibleBonesPacked && newRank != oldRank)
			{
				mTMSwapBuffer.pushBack(newRank << 16 | oldRank);
			}
		}
	}

	return changed;
}

bool ApexRenderMeshActor::getVisibilities(physx::PxU8* visibilityArray, physx::PxU32 visibilityArraySize) const
{
	NX_READ_ZONE();
	PxU8 changed = 0;
	const PxU32 numParts = physx::PxMin(mRenderMeshAsset->getPartCount(), visibilityArraySize);
	for (PxU32 index = 0; index < numParts; ++index)
	{
		const PxU8 newVisibility = (PxU8)isVisible((PxU16) index);
		changed |= newVisibility ^(*visibilityArray);
		*visibilityArray++ = newVisibility;
	}
	return changed != 0;
}

void ApexRenderMeshActor::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	updateRenderResources(!mRenderWithoutSkinning, rewriteBuffers, userRenderData);
}

void ApexRenderMeshActor::updateRenderResources(bool useBones, bool rewriteBuffers, void* userRenderData)
{
	URR_SCOPE;

#if VERBOSE > 1
	printf("updateRenderResources(useBones=%s, rewriteBuffers=%s, userRenderData=0x%p)\n", useBones ? "true" : "false", rewriteBuffers ? "true" : "false", userRenderData);
#endif

	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();

	// fill out maxBonesPerMaterial (if it hasn't already been filled out). Also create fallback skinning if necessary.
	for (PxU32 submeshIndex = 0; submeshIndex < mSubmeshData.size(); submeshIndex++)
	{
		SubmeshData& submeshData = mSubmeshData[submeshIndex];

		if (submeshData.maxBonesPerMaterial == 0 && rrm != NULL)
		{
			if (!submeshData.isMaterialPointerValid)
			{
				// this should only be reached, when renderMeshActorLoadMaterialsLazily is true.
				// URR may not be called asynchronously in that case (for example in a render thread)
				NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
				submeshData.material = nrp->getResource(submeshData.materialID);
				submeshData.isMaterialPointerValid = true;
			}

			submeshData.maxBonesPerMaterial = rrm->getMaxBonesForMaterial(submeshData.material);

			if (submeshData.maxBonesPerMaterial == 0)
			{
				submeshData.maxBonesPerMaterial = mRenderMeshAsset->getBoneCount();
			}
		}

		bool needsFallbackSkinning = mForceFallbackSkinning || submeshData.maxBonesPerMaterial < mTransforms.size();
		if (needsFallbackSkinning && !mKeepVisibleBonesPacked && submeshData.fallbackSkinningMemory == NULL)
		{
			createFallbackSkinning(submeshIndex);
		}
	}

	PX_PROFILER_PERF_SCOPE("ApexRenderMesh::updateRenderResources");

	const bool invisible = visiblePartCount() == 0;
	const bool instanceless = mMaxInstanceCount > 0 && mInstanceCount == 0;
	if ((mReleaseResourcesIfNothingToRender && ((mPartVisibilityChanged && invisible) || (mInstanceCountChanged && instanceless))) || rewriteBuffers)
	{
		// First send out signals that the resource is no longer needed.
		for (physx::PxU32 submeshIndex = 0; submeshIndex < mSubmeshData.size(); ++submeshIndex)
		{
			SubmeshData& submeshData = mSubmeshData[submeshIndex];
			for (physx::PxU32 i = 0; i < submeshData.renderResources.size(); ++i)
			{
				NxUserRenderResource* renderResource = submeshData.renderResources[i].resource;
				if (renderResource != NULL)
				{
					if (renderResource->getBoneBuffer() != NULL)
					{
						renderResource->setBoneBufferRange(0, 0);
					}
					if (renderResource->getInstanceBuffer() != NULL)
					{
						renderResource->setInstanceBufferRange(0, 0);
					}
				}
			}
		}

		// Now release the resources
		releaseRenderResources();
		mPartVisibilityChanged = false;
		mBonePosesDirty = false;
		mInstanceCountChanged = false;

		// Rewrite buffers condition
		if (rewriteBuffers)
		{
			mCreateRenderResourcesAfterInit = true; // createRenderResources
			mPartVisibilityChanged = true; // writeBuffer for submesh data
		}

		return;
	}

	if (mCreateRenderResourcesAfterInit || mOneUserVertexBufferChanged || mPartVisibilityChanged || mBoneBufferInUse != useBones)
	{
		createRenderResources(useBones, userRenderData);
		mCreateRenderResourcesAfterInit = false;
	}
	if (mRenderResource)
	{
		mRenderResource->setInstanceBufferRange(mInstanceOffset, mInstanceCount);
	}

	PX_ASSERT(mSubmeshData.size() == mRenderMeshAsset->getSubmeshCount());

	for (PxU32 submeshIndex = 0; submeshIndex < mRenderMeshAsset->getSubmeshCount(); ++submeshIndex)
	{
		SubmeshData& submeshData = mSubmeshData[submeshIndex];

		if (submeshData.indexBuffer == NULL)
		{
			continue;
		}

		if (mPartVisibilityChanged || submeshData.staticColorReplacementDirty)
		{
			updatePartVisibility(submeshIndex, useBones, userRenderData);
			submeshData.staticColorReplacementDirty = false;
		}
		if (mBonePosesDirty)
		{
			if (submeshData.userDynamicVertexBuffer && !submeshData.userSpecifiedData)
			{
				updateFallbackSkinning(submeshIndex);
			}

			// Set up the previous bone buffer, if requested and available.  If we're packing bones, we need to do a remapping
			PX_ASSERT(!mPreviousFrameBoneBufferValid || mTransformsLastFrame.size() == mTransforms.size());
			const physx::PxU32 tmBufferSize = mKeepVisibleBonesPacked ? getRenderVisiblePartCount() : mTransforms.size();
			if (mPreviousFrameBoneBufferValid && mTransformsLastFrame.size() == mTransforms.size() && mKeepVisibleBonesPacked)
			{
				mRemappedPreviousBoneTMs.resize(tmBufferSize);
				for (physx::PxU32 tmNum = 0; tmNum < tmBufferSize; ++tmNum)
				{
					mRemappedPreviousBoneTMs[tmNum] = mTransformsLastFrame[mVisiblePartsForAPILastFrame.getRank(mVisiblePartsForAPI.usedIndices()[tmNum])];
				}
			}
			else
			{
				mRemappedPreviousBoneTMs.resize(0);
			}

			updateBonePoses(submeshIndex);

			// move this under the render lock because the fracture buffer processing accesses these arrays
			// this used to be at the end of dispatchRenderResources
			if (mKeepPreviousFrameBoneBuffer)
			{
				PX_ASSERT(mTransforms.size() != 0);
				mTransformsLastFrame = mTransforms;
				mVisiblePartsForAPILastFrame = mVisiblePartsForAPI;
				mPreviousFrameBoneBufferValid = true;
			}
		}

		if (submeshData.userDynamicVertexBuffer && (submeshData.fallbackSkinningDirty || submeshData.userVertexBufferAlwaysDirty))
		{
			writeUserBuffers(submeshIndex);
			submeshData.fallbackSkinningDirty = false;
		}

		if (mMaxInstanceCount)
		{
			updateInstances(submeshIndex);
		}

		if (!submeshData.isMaterialPointerValid)
		{
			// this should only be reached, when renderMeshActorLoadMaterialsLazily is true.
			// URR may not be called asynchronously in that case (for example in a render thread)
			NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
			submeshData.material = nrp->getResource(submeshData.materialID);
			submeshData.isMaterialPointerValid = true;
		}

		for (PxU32 i = 0; i < submeshData.renderResources.size(); ++i)
		{
			NxUserRenderResource* res = submeshData.renderResources[i].resource;
			if (res != NULL)
			{
				// LRR - poor workaround for http://nvbugs/534501, you'll crash here if you have more than 60 bones/material
				// and keepVisibleBonesPacked == false
				res->setMaterial(submeshData.material);
			}
		}
	}
	mBonePosesDirty = false;
	mPartVisibilityChanged = false;
}



void ApexRenderMeshActor::dispatchRenderResources(NxUserRenderer& renderer)
{
	dispatchRenderResources(renderer, PxMat44::createIdentity());
}



void ApexRenderMeshActor::dispatchRenderResources(NxUserRenderer& renderer, const PxMat44& globalPose)
{
	PX_PROFILER_PERF_SCOPE("ApexRenderMesh::dispatchRenderResources");

	NxApexRenderContext context;

	// Assign the transform to the context when there is 1 part and no instancing

	// if there are no parts to render, return early
	// else if using instancing and there are not instances, return early
	// else if not using instancing and there is just 1 part (no bone buffer), save the transform to the context
	// else (using instancing and/or multiple parts), just assign identity to context
	if (mRenderMeshAsset->getPartCount() == 0 && !mRenderMeshAsset->getOpaqueMesh())
	{
		return;
	}
	else if (mInstanceCount == 0 && mMaxInstanceCount > 0)
	{
		return;
	}
	else if (mMaxInstanceCount == 0 && mTransforms.size() == 1)
	{
		context.local2world = globalPose * mTransforms[0]; // provide context for non-instanced ARMs with a single bone
		context.world2local = context.local2world.inverseRT();
	}
	else
	{
		context.local2world = globalPose;
		context.world2local = globalPose.inverseRT();
	}
	if (mRenderMeshAsset->getOpaqueMesh())
	{
		if (mRenderResource)
		{
			context.renderResource = mRenderResource;
			renderer.renderResource(context);
		}
	}
	else
	{
		for (PxU32 submeshIndex = 0; submeshIndex < mSubmeshData.size(); ++submeshIndex)
		{
			for (PxU32 i = 0; i < mSubmeshData[submeshIndex].renderResources.size(); ++i)
			{
				context.renderResource = mSubmeshData[submeshIndex].renderResources[i].resource;

				// no reason to render if we don't have any indices
				if ((mSubmeshData[submeshIndex].indexBuffer && (mSubmeshData[submeshIndex].visibleTriangleCount == 0)) || (mSubmeshData[submeshIndex].renderResources[i].vertexCount == 0))
				{
					continue;
				}

				if (context.renderResource)
				{
                    context.renderMeshName = mRenderMeshAsset->getName();
					renderer.renderResource(context);
				}
			}
		}
	}
}



void ApexRenderMeshActor::addVertexBuffer(PxU32 submeshIndex, bool alwaysDirty, PxVec3* position, PxVec3* normal, PxVec4* tangents)
{
#if VERBOSE
	NiGetApexSDK()->getErrorHandler().reportError(PxErrorCode::eNO_ERROR, "addVertexBuffer\n", __FILE__, __LINE__);
	printf("addVertexBuffer(submeshIndex=%d)\n", submeshIndex);
#endif
	if (submeshIndex < mSubmeshData.size())
	{
		SubmeshData& submeshData = mSubmeshData[submeshIndex];

		if (submeshData.userSpecifiedData)
		{
			APEX_INVALID_PARAMETER("Cannot add user buffer to submesh %d, it's already assigned!", submeshIndex);
		}
		else
		{
			submeshData.userSpecifiedData = true;
			submeshData.userPositions = position;
			submeshData.userNormals = normal;
			submeshData.userTangents4 = tangents;

			submeshData.userVertexBufferAlwaysDirty = alwaysDirty;
			mOneUserVertexBufferChanged = true;
		}
	}
}



void ApexRenderMeshActor::removeVertexBuffer(PxU32 submeshIndex)
{
#if VERBOSE
	NiGetApexSDK()->getErrorHandler().reportError(PxErrorCode::eNO_ERROR, "removeVertexBuffer\n", __FILE__, __LINE__);
	printf("removeVertexBuffer(submeshIndex=%d)\n", submeshIndex);
#endif
	if (submeshIndex < mSubmeshData.size())
	{
		SubmeshData& submeshData = mSubmeshData[submeshIndex];

		if (!submeshData.userSpecifiedData)
		{
			APEX_INVALID_PARAMETER("Cannot remove user buffer to submesh %d, it's not assigned!", submeshIndex);
		}
		else
		{
			submeshData.userSpecifiedData = false;
			submeshData.userPositions = NULL;
			submeshData.userNormals = NULL;
			submeshData.userTangents4 = NULL;

			submeshData.userVertexBufferAlwaysDirty = false;
			mOneUserVertexBufferChanged = true;

			if (submeshData.fallbackSkinningMemory != NULL)
			{
				distributeFallbackData(submeshIndex);
			}
		}
	}
}



void ApexRenderMeshActor::setStaticPositionReplacement(PxU32 submeshIndex, const PxVec3* staticPositions)
{
	PX_ASSERT(staticPositions != NULL);

	PX_ASSERT(submeshIndex < mSubmeshData.size());
	if (submeshIndex < mSubmeshData.size())
	{
		SubmeshData& submeshData = mSubmeshData[submeshIndex];

		PX_ASSERT(submeshData.staticPositionReplacement == NULL);
		submeshData.staticPositionReplacement = staticPositions;

		PX_ASSERT(submeshData.staticBufferReplacement == NULL);
		PX_ASSERT(submeshData.dynamicBufferReplacement == NULL);
	}
}

void ApexRenderMeshActor::setStaticColorReplacement(PxU32 submeshIndex, const PxColorRGBA* staticColors)
{
	PX_ASSERT(staticColors != NULL);

	PX_ASSERT(submeshIndex < mSubmeshData.size());
	if (submeshIndex < mSubmeshData.size())
	{
		SubmeshData& submeshData = mSubmeshData[submeshIndex];

		submeshData.staticColorReplacement = staticColors;
		submeshData.staticColorReplacementDirty = true;
	}
}



void ApexRenderMeshActor::setInstanceBuffer(NxUserRenderInstanceBuffer* instBuf)
{
	NX_WRITE_ZONE();
	mInstanceBuffer = instBuf;

	for (shdfnd::Array<SubmeshData>::Iterator it = mSubmeshData.begin(), end = mSubmeshData.end(); it != end; ++it)
	{
		it->instanceBuffer = mInstanceBuffer;
		it->userIndexBufferChanged = true;
	}

	mOneUserVertexBufferChanged = true;
}

void ApexRenderMeshActor::setMaxInstanceCount(physx::PxU32 count)
{
	NX_WRITE_ZONE();
	mMaxInstanceCount = count;
}

void ApexRenderMeshActor::setInstanceBufferRange(physx::PxU32 from, physx::PxU32 count)
{
	NX_WRITE_ZONE();
	mInstanceOffset = from;
	mInstanceCountChanged = count != mInstanceCount;
	mInstanceCount = count < mMaxInstanceCount ? count : mMaxInstanceCount;
}



void ApexRenderMeshActor::getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const
{
	NX_READ_ZONE();
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("NxApexRenderMeshActor does not support this operation");
}



physx::PxF32 ApexRenderMeshActor::getActivePhysicalLod() const
{
	NX_READ_ZONE();
	APEX_INVALID_OPERATION("NxApexRenderMeshActor does not support this operation");
	return -1.0f;
}

void ApexRenderMeshActor::forcePhysicalLod(physx::PxF32 lod)
{
	NX_WRITE_ZONE();
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("NxApexRenderMeshActor does not support this operation");
}



void ApexRenderMeshActor::createRenderResources(bool useBones, void* userRenderData)
{
#if VERBOSE
	printf("createRenderResources(useBones=%s, userRenderData=0x%p)\n", useBones ? "true" : "false", userRenderData);
#endif

	PX_PROFILER_PERF_SCOPE("ApexRenderMesh::createRenderResources");

	if (mRenderMeshAsset->getOpaqueMesh())
	{
		if (mRenderResource == NULL || mRenderResource->getInstanceBuffer() != mInstanceBuffer)
		{
			NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();
			if (mRenderResource != NULL)
			{
				rrm->releaseResource(*mRenderResource);
				mRenderResource = NULL;
			}

			NxUserRenderResourceDesc desc;
			desc.instanceBuffer = mInstanceBuffer;
			desc.opaqueMesh = mRenderMeshAsset->getOpaqueMesh();
			desc.userRenderData = userRenderData;
			mRenderResource = rrm->createResource(desc);
		}
	}

	PX_ASSERT(mSubmeshData.size() == mRenderMeshAsset->getSubmeshCount());

	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();

	bool createAndFillSharedVertexBuffersAll = mOneUserVertexBufferChanged;
	if (mRenderMeshAsset->mRuntimeSubmeshData.empty())
	{
		mRenderMeshAsset->mRuntimeSubmeshData.resize(mRenderMeshAsset->getSubmeshCount());
		memset(mRenderMeshAsset->mRuntimeSubmeshData.begin(), 0, sizeof(ApexRenderMeshAsset::SubmeshData) * mRenderMeshAsset->mRuntimeSubmeshData.size());
		createAndFillSharedVertexBuffersAll = true;
	}

	bool fill2ndVertexBuffersAll = false;
	if (!mPerActorVertexBuffers.size())
	{
		// Create a separate (instanced) buffer for bone indices and/or colors
		fill2ndVertexBuffersAll = mKeepVisibleBonesPacked;
		for (PxU32 submeshIndex = 0; !fill2ndVertexBuffersAll && submeshIndex < mRenderMeshAsset->getSubmeshCount(); ++submeshIndex)
		{
			fill2ndVertexBuffersAll = (mSubmeshData[submeshIndex].staticColorReplacement != NULL);
		}
		if (fill2ndVertexBuffersAll)
		{
			mPerActorVertexBuffers.resize(mRenderMeshAsset->getSubmeshCount());
		}
	}

	PX_ASSERT(mRenderMeshAsset->mRuntimeSubmeshData.size() == mSubmeshData.size());
	PX_ASSERT(mRenderMeshAsset->getSubmeshCount() == mSubmeshData.size());
	for (PxU32 submeshIndex = 0; submeshIndex < mRenderMeshAsset->getSubmeshCount(); ++submeshIndex)
	{
		ApexRenderSubmesh& submesh = *mRenderMeshAsset->mSubmeshes[submeshIndex];
		SubmeshData& submeshData = mSubmeshData[submeshIndex];

		if (submesh.getVertexBuffer().getVertexCount() == 0 || !submeshHasVisibleTriangles(submeshIndex))
		{
			for (PxU32 i = 0; i < submeshData.renderResources.size(); ++i)
			{
				if (submeshData.renderResources[i].resource != NULL)
				{
					NxUserRenderBoneBuffer* boneBuffer = submeshData.renderResources[i].resource->getBoneBuffer();
					rrm->releaseResource(*submeshData.renderResources[i].resource);
					if (boneBuffer)
					{
						rrm->releaseBoneBuffer(*boneBuffer);
					}
					submeshData.renderResources[i].resource = NULL;
				}
			}
			continue;
		}

		bool fill2ndVertexBuffers = fill2ndVertexBuffersAll;
		// Handling color replacement through "2nd vertex buffer"
		if ((mKeepVisibleBonesPacked || submeshData.staticColorReplacement != NULL) && mPerActorVertexBuffers[submeshIndex] == NULL)
		{
			fill2ndVertexBuffers = true;
		}

		bool createAndFillSharedVertexBuffers = createAndFillSharedVertexBuffersAll;

		// create vertex buffers if some buffer replacements are present in the actor
		if (submeshData.staticPositionReplacement != NULL && submeshData.staticBufferReplacement == NULL && submeshData.dynamicBufferReplacement == NULL)
		{
			createAndFillSharedVertexBuffers = true;
		}

		{
			ApexRenderMeshAsset::SubmeshData& runtimeSubmeshData = mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex];
			if (runtimeSubmeshData.staticVertexBuffer == NULL && runtimeSubmeshData.dynamicVertexBuffer == NULL && runtimeSubmeshData.skinningVertexBuffer == NULL)
			{
				createAndFillSharedVertexBuffers = true;
			}

			// create vertex buffers if not all static vertex buffers have been created by the previous actors that were doing this
			if (runtimeSubmeshData.needsStaticData && runtimeSubmeshData.staticVertexBuffer == NULL)
			{
				createAndFillSharedVertexBuffers = true;
			}

			if (runtimeSubmeshData.needsDynamicData && runtimeSubmeshData.dynamicVertexBuffer == NULL)
			{
				createAndFillSharedVertexBuffers = true;
			}
		}


		NiApexVertexBuffer& srcVB = submesh.getVertexBufferWritable();
		const NxVertexFormat& vf = srcVB.getFormat();

		bool fillStaticSharedVertexBuffer = false;
		bool fillDynamicSharedVertexBuffer = false;
		bool fillSkinningSharedVertexBuffer = false;

		if (createAndFillSharedVertexBuffers)
		{
			NxUserRenderVertexBufferDesc staticBufDesc, dynamicBufDesc, boneBufDesc;
			staticBufDesc.moduleIdentifier = mRenderMeshAsset->mOwnerModuleID;
			staticBufDesc.maxVerts = srcVB.getVertexCount();
			staticBufDesc.hint = NxRenderBufferHint::STATIC;
			staticBufDesc.uvOrigin = mRenderMeshAsset->getTextureUVOrigin();
			staticBufDesc.numCustomBuffers = 0;
			staticBufDesc.canBeShared = true;

			dynamicBufDesc = staticBufDesc;
			boneBufDesc = dynamicBufDesc;
			bool useDynamicBuffer = false;
			bool replaceStaticBuffer = false;
			bool replaceDynamicBuffer = false;

			// extract all the buffers into one of the three descs
			for (PxU32 i = 0; i < vf.getBufferCount(); ++i)
			{
				NxRenderVertexSemantic::Enum semantic = vf.getBufferSemantic(i);
				if (semantic >= NxRenderVertexSemantic::POSITION && semantic <= NxRenderVertexSemantic::COLOR)
				{
					if (vf.getBufferAccess(i) == NxRenderDataAccess::STATIC)
					{
						staticBufDesc.buffersRequest[semantic]	= vf.getBufferFormat(i);

						if (semantic == NxRenderVertexSemantic::POSITION && submeshData.staticPositionReplacement != NULL)
						{
							replaceStaticBuffer = true;
						}
					}
					else
					{
						dynamicBufDesc.buffersRequest[semantic]	= vf.getBufferFormat(i);
						useDynamicBuffer = true;

						if (semantic == NxRenderVertexSemantic::POSITION && submeshData.staticPositionReplacement != NULL)
						{
							replaceDynamicBuffer = true;
						}
					}
				}
				else if (semantic == NxRenderVertexSemantic::CUSTOM)
				{
					++staticBufDesc.numCustomBuffers;
				}
			}

			if (staticBufDesc.numCustomBuffers)
			{
				staticBufDesc.customBuffersIdents = &mRenderMeshAsset->mRuntimeCustomSubmeshData[submeshIndex].customBufferVoidPtrs[0];
				staticBufDesc.customBuffersRequest = &mRenderMeshAsset->mRuntimeCustomSubmeshData[submeshIndex].customBufferFormats[0];
			}

			// PH: only create bone indices/weights if more than one bone is present. one bone just needs local2world
			if (mTransforms.size() > 1)
			{
				NxUserRenderVertexBufferDesc* boneDesc = vf.hasSeparateBoneBuffer() ? &boneBufDesc : &staticBufDesc;

				if (!fill2ndVertexBuffers)
				{
					PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::BONE_INDEX));
					boneDesc->buffersRequest[NxRenderVertexSemantic::BONE_INDEX] = vf.getBufferFormat(bufferIndex);
					bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::BONE_WEIGHT));
					boneDesc->buffersRequest[NxRenderVertexSemantic::BONE_WEIGHT] = vf.getBufferFormat(bufferIndex);
				}
			}
			else
			if (mForceBoneIndexChannel)
			{
				// Note, it is assumed here that this means there's an actor which will handle dynamic parts, and will require a shared bone index buffer
				NxUserRenderVertexBufferDesc* boneDesc = vf.hasSeparateBoneBuffer() ? &boneBufDesc : &staticBufDesc;

				if (!fill2ndVertexBuffers)
				{
					PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::BONE_INDEX));
					boneDesc->buffersRequest[NxRenderVertexSemantic::BONE_INDEX] = vf.getBufferFormat(bufferIndex);
				}
			}

			for (PxU32 semantic = NxRenderVertexSemantic::TEXCOORD0; semantic <= NxRenderVertexSemantic::TEXCOORD3; ++semantic)
			{
				PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID((NxRenderVertexSemantic::Enum)semantic));
				staticBufDesc.buffersRequest[ semantic ]	= vf.getBufferFormat(bufferIndex);
			}

			{
				PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::DISPLACEMENT_TEXCOORD));
				staticBufDesc.buffersRequest[ NxRenderVertexSemantic::DISPLACEMENT_TEXCOORD ] = vf.getBufferFormat(bufferIndex);
				bufferIndex       = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::DISPLACEMENT_FLAGS));
				staticBufDesc.buffersRequest[ NxRenderVertexSemantic::DISPLACEMENT_FLAGS ] = vf.getBufferFormat(bufferIndex);
			}

			// empty static buffer?
			PxU32 numEntries = staticBufDesc.numCustomBuffers;
			for (PxU32 i = 0; i < NxRenderVertexSemantic::NUM_SEMANTICS; i++)
			{
				numEntries += (staticBufDesc.buffersRequest[i] == NxRenderDataFormat::UNSPECIFIED) ? 0 : 1;

				PX_ASSERT(staticBufDesc.buffersRequest[i] == NxRenderDataFormat::UNSPECIFIED || vertexSemanticFormatValid((NxRenderVertexSemantic::Enum)i, staticBufDesc.buffersRequest[i]));
				PX_ASSERT(dynamicBufDesc.buffersRequest[i] == NxRenderDataFormat::UNSPECIFIED || vertexSemanticFormatValid((NxRenderVertexSemantic::Enum)i, dynamicBufDesc.buffersRequest[i]));
				PX_ASSERT(boneBufDesc.buffersRequest[i] == NxRenderDataFormat::UNSPECIFIED || vertexSemanticFormatValid((NxRenderVertexSemantic::Enum)i, boneBufDesc.buffersRequest[i]));
			}

			if (numEntries > 0)
			{
				if (replaceStaticBuffer)
				{
					submeshData.staticBufferReplacement = rrm->createVertexBuffer(staticBufDesc);
					fillStaticSharedVertexBuffer = true;
				}
				else if (mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].staticVertexBuffer == NULL)
				{
					mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].staticVertexBuffer = rrm->createVertexBuffer(staticBufDesc);
					fillStaticSharedVertexBuffer = true;
				}
				mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].needsStaticData = true;
			}

			if (useDynamicBuffer)
			{
				// only create this if we don't create a per-actor dynamic buffer
				if (submeshData.fallbackSkinningMemory == NULL && !submeshData.userSpecifiedData)
				{
					if (replaceDynamicBuffer)
					{
						submeshData.dynamicBufferReplacement = rrm->createVertexBuffer(dynamicBufDesc);
						fillDynamicSharedVertexBuffer = true;
					}
					else if (mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].dynamicVertexBuffer == NULL)
					{
						mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].dynamicVertexBuffer = rrm->createVertexBuffer(dynamicBufDesc);
						fillDynamicSharedVertexBuffer = true;
					}
				}
				mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].needsDynamicData = true;
			}

			PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::BONE_INDEX));
			const PxU32 bonesPerVertex = vertexSemanticFormatElementCount(NxRenderVertexSemantic::BONE_INDEX, vf.getBufferFormat(bufferIndex));
			if (vf.hasSeparateBoneBuffer() && bonesPerVertex > 0 && mTransforms.size() > 1 && useBones && mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].skinningVertexBuffer == NULL)
			{
				mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].skinningVertexBuffer = rrm->createVertexBuffer(boneBufDesc);
				fillSkinningSharedVertexBuffer = true;
			}
		}

		if ((submeshData.fallbackSkinningMemory != NULL || submeshData.userSpecifiedData) && submeshData.userDynamicVertexBuffer == NULL)
		{
			NxUserRenderVertexBufferDesc perActorDynamicBufDesc;
			perActorDynamicBufDesc.moduleIdentifier = mRenderMeshAsset->mOwnerModuleID;
			perActorDynamicBufDesc.maxVerts = srcVB.getVertexCount();
			perActorDynamicBufDesc.uvOrigin = mRenderMeshAsset->getTextureUVOrigin();
			perActorDynamicBufDesc.hint = NxRenderBufferHint::DYNAMIC;
			perActorDynamicBufDesc.canBeShared = false;

			if (submeshData.userPositions != NULL)
			{
				perActorDynamicBufDesc.buffersRequest[NxRenderVertexSemantic::POSITION] = NxRenderDataFormat::FLOAT3;
			}

			if (submeshData.userNormals != NULL)
			{
				perActorDynamicBufDesc.buffersRequest[NxRenderVertexSemantic::NORMAL] = NxRenderDataFormat::FLOAT3;
			}

			if (submeshData.userTangents4 != NULL)
			{
				perActorDynamicBufDesc.buffersRequest[NxRenderVertexSemantic::TANGENT] = NxRenderDataFormat::FLOAT4;
			}

			submeshData.userDynamicVertexBuffer = rrm->createVertexBuffer(perActorDynamicBufDesc);
		}

		if (fill2ndVertexBuffers)
		{
			NxUserRenderVertexBufferDesc bufDesc;
			bufDesc.moduleIdentifier = mRenderMeshAsset->mOwnerModuleID;
			bufDesc.maxVerts = srcVB.getVertexCount();
			bufDesc.hint = NxRenderBufferHint::DYNAMIC;
			if (mKeepVisibleBonesPacked)
			{
				bufDesc.buffersRequest[ NxRenderVertexSemantic::BONE_INDEX ] = NxRenderDataFormat::USHORT1;
			}
			if (submeshData.staticColorReplacement)
			{
				bufDesc.buffersRequest[ NxRenderVertexSemantic::COLOR ] = NxRenderDataFormat::R8G8B8A8;
			}
			bufDesc.uvOrigin = mRenderMeshAsset->getTextureUVOrigin();
			bufDesc.canBeShared = false;
			for (PxU32 i = 0; i < NxRenderVertexSemantic::NUM_SEMANTICS; i++)
			{
				PX_ASSERT(bufDesc.buffersRequest[i] == NxRenderDataFormat::UNSPECIFIED || vertexSemanticFormatValid((NxRenderVertexSemantic::Enum)i, bufDesc.buffersRequest[i]));
			}
			mPerActorVertexBuffers[submeshIndex] = rrm->createVertexBuffer(bufDesc);
		}

		// creates and/or fills index buffers
		updatePartVisibility(submeshIndex, useBones, userRenderData);

		if (fillStaticSharedVertexBuffer || fillDynamicSharedVertexBuffer || fillSkinningSharedVertexBuffer)
		{
			const NxVertexFormat& vf = srcVB.getFormat();

			NxApexRenderVertexBufferData    dynamicWriteData;
			NxApexRenderVertexBufferData    staticWriteData;
			NxApexRenderVertexBufferData    skinningWriteData;
			NxApexRenderVertexBufferData&	skinningWriteDataRef = vf.hasSeparateBoneBuffer() ? skinningWriteData : staticWriteData;

			NxUserRenderVertexBuffer* staticVb = mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].staticVertexBuffer;
			NxUserRenderVertexBuffer* dynamicVb = mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].dynamicVertexBuffer;
			NxUserRenderVertexBuffer* skinningVb = mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].skinningVertexBuffer;

			if (submeshData.staticBufferReplacement != NULL)
			{
				staticVb = submeshData.staticBufferReplacement;
			}

			if (submeshData.dynamicBufferReplacement != NULL)
			{
				dynamicVb = submeshData.dynamicBufferReplacement;
			}

			for (PxU32 semantic = NxRenderVertexSemantic::POSITION; semantic <= NxRenderVertexSemantic::COLOR; ++semantic)
			{
				if (semantic == NxRenderVertexSemantic::COLOR && submeshData.staticColorReplacement != NULL)
				{
					// Gets done in updatePartVisibility if submeshData.staticColorReplacement is used
					continue;
				}

				NxRenderDataFormat::Enum format;
				PxI32 bufferIndex = vf.getBufferIndexFromID(vf.getSemanticID((NxRenderVertexSemantic::Enum)semantic));
				if (bufferIndex < 0)
				{
					continue;
				}
				const void* src = srcVB.getBufferAndFormat(format, (physx::PxU32)bufferIndex);

				if (semantic == NxRenderVertexSemantic::POSITION && submeshData.staticPositionReplacement != NULL)
				{
					src = submeshData.staticPositionReplacement;
				}

				if (format != NxRenderDataFormat::UNSPECIFIED)
				{
					if (srcVB.getFormat().getBufferAccess((physx::PxU32)bufferIndex) == NxRenderDataAccess::STATIC)
					{
						staticWriteData.setSemanticData((NxRenderVertexSemantic::Enum)semantic, src, NxRenderDataFormat::getFormatDataSize(format), format);
					}
					else
					{
						dynamicWriteData.setSemanticData((NxRenderVertexSemantic::Enum)semantic, src, NxRenderDataFormat::getFormatDataSize(format), format);
					}
				}
			}

			for (PxU32 semantic = NxRenderVertexSemantic::TEXCOORD0; semantic <= NxRenderVertexSemantic::TEXCOORD3; ++semantic)
			{
				NxRenderDataFormat::Enum format;
				PxI32 bufferIndex = vf.getBufferIndexFromID(vf.getSemanticID((NxRenderVertexSemantic::Enum)semantic));
				if (bufferIndex < 0)
				{
					continue;
				}
				const void* src = srcVB.getBufferAndFormat(format, (physx::PxU32)bufferIndex);
				if (format != NxRenderDataFormat::UNSPECIFIED)
				{
					staticWriteData.setSemanticData((NxRenderVertexSemantic::Enum)semantic, src, NxRenderDataFormat::getFormatDataSize(format), format);
				}
			}

			for (PxU32 semantic = NxRenderVertexSemantic::DISPLACEMENT_TEXCOORD; semantic <= NxRenderVertexSemantic::DISPLACEMENT_FLAGS; ++semantic)
			{
				PxI32 bufferIndex = vf.getBufferIndexFromID(vf.getSemanticID((NxRenderVertexSemantic::Enum)semantic));
				if (bufferIndex >= 0)
				{
					NxRenderDataFormat::Enum format;
					const void* src = srcVB.getBufferAndFormat(format, (physx::PxU32)bufferIndex);
					if (format != NxRenderDataFormat::UNSPECIFIED)
					{
						staticWriteData.setSemanticData((NxRenderVertexSemantic::Enum)semantic, src, NxRenderDataFormat::getFormatDataSize(format), format);
					}
				}
			}

			shdfnd::Array<NxApexRenderSemanticData> semanticData;

			const PxU32 numCustom = vf.getCustomBufferCount();
			if (numCustom)
			{
				// NxParameterized::Handle custom vertex buffer semantics
				semanticData.resize(numCustom);

				physx::PxU32 writeIndex = 0;
				for (physx::PxU32 i = 0; i < vf.getBufferCount(); i++)
				{
					// Fill in a NxApexRenderSemanticData for each custom semantic
					if (vf.getBufferSemantic(i) != NxRenderVertexSemantic::CUSTOM)
					{
						continue;
					}
					semanticData[writeIndex].data = srcVB.getBuffer(i);
					NxRenderDataFormat::Enum fmt = mRenderMeshAsset->mRuntimeCustomSubmeshData[submeshIndex].customBufferFormats[writeIndex];
					semanticData[writeIndex].stride = NxRenderDataFormat::getFormatDataSize(fmt);
					semanticData[writeIndex].format = fmt;
					semanticData[writeIndex].ident = mRenderMeshAsset->mRuntimeCustomSubmeshData[submeshIndex].customBufferVoidPtrs[writeIndex];

					writeIndex++;
				}
				PX_ASSERT(writeIndex == numCustom);
				staticWriteData.setCustomSemanticData(&semanticData[0], numCustom);
			}

			physx::Array<physx::PxU16> boneIndicesModuloMaxBoneCount;

			if (mTransforms.size() > 1 || mForceBoneIndexChannel)
			{
				PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::BONE_INDEX));
				const PxU32 numBonesPerVertex = vertexSemanticFormatElementCount(NxRenderVertexSemantic::BONE_INDEX, vf.getBufferFormat(bufferIndex));
				if (numBonesPerVertex == 1)
				{
					// Gets done in updatePartVisibility if keepVisibleBonesPacked is true
					if (!mKeepVisibleBonesPacked)
					{
						NxRenderDataFormat::Enum format;
						const NxVertexFormat& vf = srcVB.getFormat();
						physx::PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::BONE_INDEX));
						const void* src = srcVB.getBufferAndFormat(format, bufferIndex);
						if (format != NxRenderDataFormat::UNSPECIFIED)
						{
							if (mForceBoneIndexChannel && format == NxRenderDataFormat::USHORT1 && submeshData.maxBonesPerMaterial > 0 && submeshData.maxBonesPerMaterial < mRenderMeshAsset->getBoneCount())
							{
								boneIndicesModuloMaxBoneCount.resize(srcVB.getVertexCount());
								physx::PxU16* srcBuf = (physx::PxU16*)src;
								for (physx::PxU32 vertexNum = 0; vertexNum < srcVB.getVertexCount(); ++vertexNum)
								{
									boneIndicesModuloMaxBoneCount[vertexNum] = *(srcBuf++)%submeshData.maxBonesPerMaterial;
								}
								src = &boneIndicesModuloMaxBoneCount[0];
							}
							skinningWriteDataRef.setSemanticData(NxRenderVertexSemantic::BONE_INDEX, src, NxRenderDataFormat::getFormatDataSize(format), format);
						}
					}
				}
				else
				{
					NxRenderDataFormat::Enum format;
					const void* src;
					const NxVertexFormat& vf = srcVB.getFormat();
					physx::PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(physx::NxRenderVertexSemantic::BONE_INDEX));
					src = srcVB.getBufferAndFormat(format, bufferIndex);
					if (format != NxRenderDataFormat::UNSPECIFIED)
					{
						skinningWriteDataRef.setSemanticData(NxRenderVertexSemantic::BONE_INDEX, src, NxRenderDataFormat::getFormatDataSize(format), format);
					}
					bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(physx::NxRenderVertexSemantic::BONE_WEIGHT));
					src = srcVB.getBufferAndFormat(format, bufferIndex);
					if (format != NxRenderDataFormat::UNSPECIFIED)
					{
						skinningWriteDataRef.setSemanticData(NxRenderVertexSemantic::BONE_WEIGHT, src, NxRenderDataFormat::getFormatDataSize(format), format);
					}
				}
			}

			if (staticVb != NULL && fillStaticSharedVertexBuffer)
			{
				staticVb->writeBuffer(staticWriteData,   0, srcVB.getVertexCount());
			}
			if (dynamicVb != NULL && fillDynamicSharedVertexBuffer)
			{
				dynamicVb->writeBuffer(dynamicWriteData,  0, srcVB.getVertexCount());
			}
			if (skinningVb != NULL && fillSkinningSharedVertexBuffer)
			{
				skinningVb->writeBuffer(skinningWriteData, 0, srcVB.getVertexCount());
			}

			// TODO - SJB - Beta2 - release submesh after updateRenderResources() returns. It requires acquiring the actor lock as game engine could delay these
			// writes so long as it holds the render lock.  Could be done in updateBounds(), which implictly has the lock.  Perhaps we need to catch lock release
			// so it happens immediately.
		}

		// Delete static vertex buffers after writing them
		if (mRenderMeshAsset->mParams->deleteStaticBuffersAfterUse)
		{
			ApexVertexFormat dynamicFormats;
			dynamicFormats.copy((const ApexVertexFormat&)vf);
			for (physx::PxU32 semantic = NxRenderVertexSemantic::POSITION; semantic < NxRenderVertexSemantic::NUM_SEMANTICS; ++semantic)
			{
				PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID((NxRenderVertexSemantic::Enum)semantic));
				if (dynamicFormats.getBufferAccess(bufferIndex) == NxRenderDataAccess::STATIC)
				{
					dynamicFormats.setBufferFormat(bufferIndex, NxRenderDataFormat::UNSPECIFIED);
				}
			}
			srcVB.build(dynamicFormats, srcVB.getVertexCount());
		}
	}
	mOneUserVertexBufferChanged = false;
	mPartVisibilityChanged = false;
	mBoneBufferInUse = useBones;
}

void ApexRenderMeshActor::updatePartVisibility(PxU32 submeshIndex, bool useBones, void* userRenderData)
{
#if VERBOSE
	printf("updatePartVisibility(submeshIndex=%d, useBones=%s, userRenderData=0x%p)\n", submeshIndex, useBones ? "true" : "false", userRenderData);
	printf("  mPartVisibilityChanged=%s\n", mPartVisibilityChanged ? "true" : "false");
#endif

	const ApexRenderSubmesh& submesh = *mRenderMeshAsset->mSubmeshes[submeshIndex];
	SubmeshData& submeshData = mSubmeshData[ submeshIndex ];

	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();
	PX_ASSERT(rrm != NULL);

	// we end up with a division by 0 otherwise :(
	PX_ASSERT(submeshData.maxBonesPerMaterial > 0);

	const PxU32 partCount = mKeepVisibleBonesPacked ? getRenderVisiblePartCount() : mRenderMeshAsset->getPartCount();

	// only use bones if there is no fallback skinning
	useBones &= submeshData.fallbackSkinningMemory == NULL;

	PxU32 resourceCount;
	if (!useBones)
	{
		// If we're not skinning, we only need one resource
		resourceCount = 1;
	}
	else
	{
		// LRR - poor workaround for http://nvbugs/534501
		if (mKeepVisibleBonesPacked)
		{
			resourceCount = partCount == 0 ? 0 : (partCount + submeshData.maxBonesPerMaterial - 1) / submeshData.maxBonesPerMaterial;
		}
		else
		{
			resourceCount = partCount == 0 ? 0 : (mRenderMeshAsset->getBoneCount() + submeshData.maxBonesPerMaterial - 1) / submeshData.maxBonesPerMaterial;
		}
	}

	// Eliminate unneeded resources:
	const PxU32 start = (submeshData.userIndexBufferChanged || useBones != mBoneBufferInUse) ? 0 : resourceCount;
	for (PxU32 i = start; i < submeshData.renderResources.size(); ++i)
	{
		if (submeshData.renderResources[i].resource != NULL)
		{
			NxUserRenderBoneBuffer* boneBuffer = submeshData.renderResources[i].resource->getBoneBuffer();
			rrm->releaseResource(*submeshData.renderResources[i].resource);

			if (boneBuffer)
			{
				rrm->releaseBoneBuffer(*boneBuffer);
			}

			submeshData.renderResources[i].resource = NULL;
		}
	}
	submeshData.userIndexBufferChanged = false;

	physx::PxU16 resourceBoneCount = 0;
	physx::PxU32 resourceNum = 0;
	physx::PxU32 startIndex = 0;

	if (mKeepVisibleBonesPacked)
	{
		if (mBoneIndexTempBuffer.size() < submesh.getVertexBuffer().getVertexCount())
		{
			mBoneIndexTempBuffer.resize(submesh.getVertexBuffer().getVertexCount());	// A new index buffer to remap the bone indices to the smaller buffer
		}
	}

	PxU32 boneIndexStart = PxU32(-1);
	PxU32 boneIndexEnd = 0;

	// Figure out how many indices we'll need
	physx::PxU32 totalIndexCount = submesh.getTotalIndexCount();	// Worst case

	const physx::PxU32* visiblePartIndexPtr = getRenderVisibleParts();

	if (mKeepVisibleBonesPacked)
	{
		// We can do better
		totalIndexCount = 0;
		for (physx::PxU32 partNum = 0; partNum < partCount; ++partNum)
		{
			totalIndexCount += submesh.getIndexCount(visiblePartIndexPtr[partNum]);
		}
	}

	physx::PxU32 newIndexBufferRequestSize = submeshData.indexBufferRequestedSize;

	// If there has not already been an index buffer request, set to exact size
	if (newIndexBufferRequestSize == 0 || totalIndexCount >= 0x80000000)	// special handling of potential overflow
	{
		newIndexBufferRequestSize = totalIndexCount;
	}
	else
	{
		// If the buffer has already been requested, see if we need to grow or shrink it
		while (totalIndexCount > newIndexBufferRequestSize)
		{
			newIndexBufferRequestSize *= 2;
		}
		while (2*totalIndexCount < newIndexBufferRequestSize)
		{
			newIndexBufferRequestSize /= 2;
		}
	}

	// In case our doubling schedule gave it a larger size than we'll ever need
	if (newIndexBufferRequestSize > submesh.getTotalIndexCount())
	{
		newIndexBufferRequestSize = submesh.getTotalIndexCount();
	}

	if (submeshData.indexBuffer != NULL && newIndexBufferRequestSize != submeshData.indexBufferRequestedSize)
	{
		// Release the old buffer
		rrm->releaseIndexBuffer(*submeshData.indexBuffer);
		submeshData.indexBuffer = NULL;
		releaseSubmeshRenderResources(submeshIndex);
	}

	// Create the index buffer now if needed
	if (submeshData.indexBuffer == NULL && newIndexBufferRequestSize > 0)
	{
		NxUserRenderIndexBufferDesc indexDesc;
		indexDesc.maxIndices = newIndexBufferRequestSize;
		indexDesc.hint       = mIndexBufferHint;
		indexDesc.format     = NxRenderDataFormat::UINT1;
		submeshData.indexBuffer = rrm->createIndexBuffer(indexDesc);
		submeshData.indexBufferRequestedSize = newIndexBufferRequestSize;
	}

	submeshData.renderResources.resize(resourceCount, ResourceData());

	submeshData.visibleTriangleCount = 0;
	// KHA - batch writes to temporary buffer so that index buffer is only locked once per frame
	if(mPartIndexTempBuffer.size() < totalIndexCount)
	{
		mPartIndexTempBuffer.resize(totalIndexCount);
	}
	for (physx::PxU32 partNum = 0; partNum < partCount;)
	{
		physx::PxU32 partIndex;
		bool partIsVisible;
		if (mKeepVisibleBonesPacked)
		{
			partIndex = visiblePartIndexPtr[partNum++];
			partIsVisible = true;

			const PxU32 indexStart = submesh.getFirstVertexIndex(partIndex);
			const PxU32 vertexCount = submesh.getVertexCount(partIndex);

			PxU16* boneIndex = mBoneIndexTempBuffer.begin() + indexStart;
			const PxU16* boneIndexStop = boneIndex + vertexCount;

			boneIndexStart = PxMin(boneIndexStart, indexStart);
			boneIndexEnd = PxMax(boneIndexEnd, indexStart + vertexCount);

			while (boneIndex < boneIndexStop)
			{
				*boneIndex++ = resourceBoneCount;
			}
		}
		else
		{
			partIndex = partNum++;
			partIsVisible = isVisible((physx::PxU16)partIndex);
		}

		if (partIsVisible)
		{
			const physx::PxU32 indexCount = submesh.getIndexCount(partIndex);
			const physx::PxU32* indices = submesh.getIndexBuffer(partIndex);
			const physx::PxU32 currentIndexNum = submeshData.visibleTriangleCount * 3;
			if (indexCount > 0 && mPartVisibilityChanged)
			{
				memcpy(mPartIndexTempBuffer.begin() + currentIndexNum, indices, indexCount * sizeof(PxU32));
			}
			submeshData.visibleTriangleCount += indexCount / 3;
		}

		// LRR - poor workaround for http://nvbugs/534501
		bool generateNewRenderResource = false;

		const bool oneBonePerPart = mSkinningMode != NxRenderMeshActorSkinningMode::AllBonesPerPart;

		const physx::PxU16 bonesToAdd = oneBonePerPart ? 1u : (physx::PxU16)mRenderMeshAsset->getBoneCount();
		resourceBoneCount = physx::PxMin<physx::PxU16>((physx::PxU16)(resourceBoneCount + bonesToAdd), (physx::PxU16)submeshData.maxBonesPerMaterial);

		// Check if we exceed max bones limit or if this is the last part
		if ((useBones && resourceBoneCount == submeshData.maxBonesPerMaterial) || partNum == partCount)
		{
			generateNewRenderResource = true;
		}
		if (generateNewRenderResource)
		{
			submeshData.renderResources[resourceNum].boneCount = resourceBoneCount;
			submeshData.renderResources[resourceNum].vertexCount = submeshData.visibleTriangleCount * 3 - startIndex;
			NxUserRenderResource*& renderResource = submeshData.renderResources[resourceNum].resource;	// Next resource
			++resourceNum;
			if (renderResource == NULL)	// Create if needed
			{
				NxUserRenderVertexBuffer* vertexBuffers[5] = { NULL };
				PxU32 numVertexBuffers = 0;

				if (submeshData.staticBufferReplacement != NULL)
				{
					vertexBuffers[numVertexBuffers++] = submeshData.staticBufferReplacement;
				}
				else if (mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].staticVertexBuffer != NULL)
				{
					vertexBuffers[numVertexBuffers++] = mRenderMeshAsset->mRuntimeSubmeshData[ submeshIndex ].staticVertexBuffer;
				}

				if (submeshData.userDynamicVertexBuffer != NULL && (submeshData.userSpecifiedData || submeshData.fallbackSkinningMemory != NULL))
				{
					vertexBuffers[numVertexBuffers++] = submeshData.userDynamicVertexBuffer;
				}
				else if (submeshData.dynamicBufferReplacement != NULL)
				{
					vertexBuffers[numVertexBuffers++] = submeshData.dynamicBufferReplacement;
				}
				else if (mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].dynamicVertexBuffer != NULL)
				{
					if (submeshData.userDynamicVertexBuffer != NULL && mReleaseResourcesIfNothingToRender)
					{
						rrm->releaseVertexBuffer(*submeshData.userDynamicVertexBuffer);
						submeshData.userDynamicVertexBuffer = NULL;
					}

					vertexBuffers[numVertexBuffers++] = mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].dynamicVertexBuffer;
				}

				if (useBones && mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].skinningVertexBuffer != NULL)
				{
					vertexBuffers[numVertexBuffers++] = mRenderMeshAsset->mRuntimeSubmeshData[submeshIndex].skinningVertexBuffer;
				}

				// Separate (instanced) buffer for bone indices
				if (mPerActorVertexBuffers.size())
				{
					vertexBuffers[numVertexBuffers++] = mPerActorVertexBuffers[ submeshIndex ];
				}

				PX_ASSERT(numVertexBuffers <= 5);

				NxUserRenderResourceDesc resourceDesc;
				resourceDesc.primitives = NxRenderPrimitiveType::TRIANGLES;

				resourceDesc.vertexBuffers = vertexBuffers;
				resourceDesc.numVertexBuffers = numVertexBuffers;

				resourceDesc.numVerts = submesh.getVertexBuffer().getVertexCount();

				resourceDesc.indexBuffer = submeshData.indexBuffer;
				resourceDesc.firstIndex = startIndex;
				resourceDesc.numIndices = submeshData.visibleTriangleCount * 3 - startIndex;

				// not assuming partcount == bonecount anymore
				//if (mRenderMeshAsset->getPartCount() > 1)
				const PxU32 numBones = mRenderMeshAsset->getBoneCount();
				if (numBones > 1 && useBones)
				{
					NxUserRenderBoneBufferDesc boneDesc;
					// we don't need to use the minimum of numBones and max bones because the 
					// bone buffer update contains the proper range
					boneDesc.maxBones = submeshData.maxBonesPerMaterial;
					boneDesc.hint = NxRenderBufferHint::DYNAMIC;
					boneDesc.buffersRequest[ NxRenderBoneSemantic::POSE ] = NxRenderDataFormat::FLOAT3x4;
					if (mKeepPreviousFrameBoneBuffer)
					{
						boneDesc.buffersRequest[ NxRenderBoneSemantic::PREVIOUS_POSE ] = NxRenderDataFormat::FLOAT3x4;
					}
					resourceDesc.boneBuffer = rrm->createBoneBuffer(boneDesc);
					PX_ASSERT(resourceDesc.boneBuffer);
					if (resourceDesc.boneBuffer)
					{
						resourceDesc.numBones = numBones;
					}
				}

				resourceDesc.instanceBuffer = submeshData.instanceBuffer;
				resourceDesc.numInstances = 0;

				if (!submeshData.isMaterialPointerValid)
				{
					// this should only be reached, when renderMeshActorLoadMaterialsLazily is true.
					// URR may not be called asynchronously in that case (for example in a render thread)
					NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
					if (nrp != NULL)
					{
						submeshData.material = nrp->getResource(submeshData.materialID);
						submeshData.isMaterialPointerValid = true;
					}
				}

				resourceDesc.material = submeshData.material;

				resourceDesc.submeshIndex = submeshIndex;

				resourceDesc.userRenderData = userRenderData;

				resourceDesc.cullMode = submesh.getVertexBuffer().getFormat().getWinding();

				if (resourceDesc.isValid()) // TODO: should probably make this an if-statement... -jgd // I did, -poh
				{
					renderResource = rrm->createResource(resourceDesc);
				}
			}

			if (renderResource != NULL)
			{
				renderResource->setIndexBufferRange(startIndex, submeshData.visibleTriangleCount * 3 - startIndex);
				startIndex = submeshData.visibleTriangleCount * 3;

				if (renderResource->getBoneBuffer() != NULL)
				{
					renderResource->setBoneBufferRange(0, resourceBoneCount);
					// TODO - LRR - make useBoneVisibilitySemantic work with >1 bone/part
					// if visible bone optimization enabled (as set in the actor desc)
					// {
					//		if (renderMesh->getBoneBuffer() != NULL)
					//		if we have a 1:1 bone:part mapping, as determined when asset is loaded (or authored)
					//			renderMesh->getBoneBuffer()->writeBuffer(NxRenderBoneSemantic::VISIBLE_INDEX, visibleParts.usedIndices(), sizeof(physx::PxU32), 0, visibleParts.usedCount());
					//		else
					//		{
					//			// run through index buffer, and find all bones referenced by visible verts and store in visibleBones
					//			renderMesh->getBoneBuffer()->writeBuffer(NxRenderBoneSemantic::VISIBLE_INDEX, visibleBones.usedIndices(), sizeof(physx::PxU32), 0, visibleBones.usedCount());
					//		}
					// }
				}
			}

			resourceBoneCount = 0;
		}
	}

	if (boneIndexStart == PxU32(-1))
	{
		boneIndexStart = 0;
	}

	// KHA - Write temporary buffer to index buffer
	if(submeshData.indexBuffer != NULL && mPartVisibilityChanged)
	{
		submeshData.indexBuffer->writeBuffer(mPartIndexTempBuffer.begin(), sizeof(PxU32), 0, submeshData.visibleTriangleCount*3);
	}

	// Write re-mapped bone indices
	if (mPerActorVertexBuffers.size())
	{
		if (mTransforms.size() > 1 && mKeepVisibleBonesPacked)
		{
			NxApexRenderVertexBufferData skinningWriteData;
			skinningWriteData.setSemanticData(NxRenderVertexSemantic::BONE_INDEX, mBoneIndexTempBuffer.begin() + boneIndexStart, sizeof(PxU16), NxRenderDataFormat::USHORT1);
			if (submeshData.staticColorReplacement != NULL)
			{
				skinningWriteData.setSemanticData(NxRenderVertexSemantic::COLOR, submeshData.staticColorReplacement + boneIndexStart, sizeof(PxColorRGBA), NxRenderDataFormat::R8G8B8A8);
			}
			mPerActorVertexBuffers[submeshIndex]->writeBuffer(skinningWriteData, boneIndexStart, boneIndexEnd - boneIndexStart);
		}
		else
		if (submeshData.staticColorReplacement != NULL)
		{
			NxApexRenderVertexBufferData skinningWriteData;
			skinningWriteData.setSemanticData(NxRenderVertexSemantic::COLOR, submeshData.staticColorReplacement, sizeof(PxColorRGBA), NxRenderDataFormat::R8G8B8A8);
			mPerActorVertexBuffers[submeshIndex]->writeBuffer(skinningWriteData, 0, submesh.getVertexBuffer().getVertexCount());
		}
	}

	mBonePosesDirty = true;

#if VERBOSE
	printf("-updatePartVisibility(submeshIndex=%d, useBones=%s, userRenderData=0x%p)\n", submeshIndex, useBones ? "true" : "false", userRenderData);
#endif
}

void ApexRenderMeshActor::updateBonePoses(PxU32 submeshIndex)
{
// There can now be >1 bones per part
//	if (mRenderMeshAsset->getPartCount() > 1)
	if (mRenderMeshAsset->getBoneCount() > 1)
	{
		SubmeshData& submeshData = mSubmeshData[ submeshIndex ];
		PxMat34Legacy* boneTMs = mTransforms.begin();
		const physx::PxU32 tmBufferSize = mKeepVisibleBonesPacked ? getRenderVisiblePartCount() : mTransforms.size();

		// Set up the previous bone buffer, if requested and available
		PxMat34Legacy* previousBoneTMs = NULL;
		if (!mPreviousFrameBoneBufferValid || mTransformsLastFrame.size() != mTransforms.size())
		{
			previousBoneTMs = boneTMs;
		}
		else
		if (!mKeepVisibleBonesPacked || mRemappedPreviousBoneTMs.size() == 0)
		{
			previousBoneTMs = mTransformsLastFrame.begin();
		}
		else
		{
			previousBoneTMs = mRemappedPreviousBoneTMs.begin();
		}

		PxU32 tmsRemaining = tmBufferSize;
		for (physx::PxU32 i = 0; i < submeshData.renderResources.size(); ++i)
		{
			NxUserRenderResource* renderResource = submeshData.renderResources[i].resource;
			const PxU32 resourceBoneCount = submeshData.renderResources[i].boneCount;
			if (renderResource && renderResource->getBoneBuffer() != NULL)
			{
				NxApexRenderBoneBufferData boneWriteData;
				boneWriteData.setSemanticData(NxRenderBoneSemantic::POSE, boneTMs, sizeof(physx::PxMat34Legacy), NxRenderDataFormat::FLOAT3x4);
				if (mKeepPreviousFrameBoneBuffer)
				{
					boneWriteData.setSemanticData(NxRenderBoneSemantic::PREVIOUS_POSE, previousBoneTMs, sizeof(physx::PxMat34Legacy), NxRenderDataFormat::FLOAT3x4);
				}
				renderResource->getBoneBuffer()->writeBuffer(boneWriteData, 0, physx::PxMin(tmsRemaining, resourceBoneCount));
				tmsRemaining -= resourceBoneCount;
				boneTMs += resourceBoneCount;
				previousBoneTMs += resourceBoneCount;
			}
		}
	}
}

void ApexRenderMeshActor::releaseSubmeshRenderResources(PxU32 submeshIndex)
{
#if VERBOSE
	printf("releaseSubmeshRenderResources()\n");
#endif

	if (submeshIndex >= mSubmeshData.size())
	{
		return;
	}

	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();

	SubmeshData& submeshData = mSubmeshData[submeshIndex];
	for (PxU32 j = submeshData.renderResources.size(); j--;)
	{
		if (submeshData.renderResources[j].resource != NULL)
		{
			if (submeshData.renderResources[j].resource->getBoneBuffer() != NULL)
			{
				rrm->releaseBoneBuffer(*submeshData.renderResources[j].resource->getBoneBuffer());
			}
			rrm->releaseResource(*submeshData.renderResources[j].resource);
			submeshData.renderResources[j].resource = NULL;
		}
	}
	submeshData.renderResources.reset();
}


void ApexRenderMeshActor::releaseRenderResources()
{
#if VERBOSE
	printf("releaseRenderResources()\n");
#endif

	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();

	for (PxU32 i = mSubmeshData.size(); i--;)
	{
		releaseSubmeshRenderResources(i);

		SubmeshData& submeshData = mSubmeshData[i];

		if (submeshData.indexBuffer != NULL)
		{
			rrm->releaseIndexBuffer(*submeshData.indexBuffer);
			submeshData.indexBuffer = NULL;
		}
		submeshData.instanceBuffer = NULL;

		if (submeshData.staticBufferReplacement != NULL)
		{
			rrm->releaseVertexBuffer(*submeshData.staticBufferReplacement);
			submeshData.staticBufferReplacement = NULL;
		}

		if (submeshData.dynamicBufferReplacement != NULL)
		{
			rrm->releaseVertexBuffer(*submeshData.dynamicBufferReplacement);
			submeshData.dynamicBufferReplacement = NULL;
		}

		if (submeshData.userDynamicVertexBuffer != NULL)
		{
			rrm->releaseVertexBuffer(*submeshData.userDynamicVertexBuffer);
			submeshData.userDynamicVertexBuffer = NULL;
		}
		submeshData.userIndexBufferChanged = false;
	}

	for (PxU32 i = mPerActorVertexBuffers.size(); i--;)
	{
		if (mPerActorVertexBuffers[i] != NULL)
		{
			rrm->releaseVertexBuffer(*mPerActorVertexBuffers[i]);
			mPerActorVertexBuffers[i] = NULL;
		}
	}
	mPerActorVertexBuffers.reset();

	if (mRenderResource)
	{
		rrm->releaseResource(*mRenderResource);
		mRenderResource = NULL;
	}

	mBoneBufferInUse = false;
}



bool ApexRenderMeshActor::submeshHasVisibleTriangles(PxU32 submeshIndex) const
{
	const ApexRenderSubmesh& submesh = *mRenderMeshAsset->mSubmeshes[submeshIndex];

	const PxU32 partCount = getRenderVisiblePartCount();
	const PxU32* visiblePartIndexPtr = getRenderVisibleParts();

	for (PxU32 partNum = 0; partNum < partCount;)
	{
		const PxU32 partIndex = visiblePartIndexPtr[partNum++];
		const PxU32 indexCount = submesh.getIndexCount(partIndex);

		if (indexCount > 0)
		{
			return true;
		}
	}

	return false;
}



void ApexRenderMeshActor::createFallbackSkinning(PxU32 submeshIndex)
{
	if (mTransforms.size() == 1)
	{
		return;
	}

#if VERBOSE
	printf("createFallbackSkinning(submeshIndex=%d)\n", submeshIndex);
#endif
	const NxVertexBuffer& vertexBuffer = mRenderMeshAsset->getSubmesh(submeshIndex).getVertexBuffer();
	const NxVertexFormat& format = vertexBuffer.getFormat();

	const PxU32 bufferCount = format.getBufferCount();

	PxU32 bufferSize = 0;
	for (PxU32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
	{
		if (format.getBufferAccess(bufferIndex) == NxRenderDataAccess::DYNAMIC)
		{
			NxRenderDataFormat::Enum bufferFormat = format.getBufferFormat(bufferIndex);
			NxRenderVertexSemantic::Enum bufferSemantic = format.getBufferSemantic(bufferIndex);

			if (bufferSemantic == NxRenderVertexSemantic::POSITION ||
			        bufferSemantic == NxRenderVertexSemantic::NORMAL ||
			        bufferSemantic == NxRenderVertexSemantic::TANGENT)
			{
				if (bufferFormat == NxRenderDataFormat::FLOAT3)
				{
					bufferSize += sizeof(PxVec3);
				}
				else if (bufferFormat == NxRenderDataFormat::FLOAT4)
				{
					bufferSize += sizeof(PxVec4);
				}
			}
		}
	}

	if (bufferSize > 0)
	{
		PX_ASSERT(mSubmeshData[submeshIndex].fallbackSkinningMemory == NULL);
		mSubmeshData[submeshIndex].fallbackSkinningMemorySize = bufferSize * vertexBuffer.getVertexCount();
		mSubmeshData[submeshIndex].fallbackSkinningMemory = PX_ALLOC(mSubmeshData[submeshIndex].fallbackSkinningMemorySize, "fallbackSkinnningMemory");

		PX_ASSERT(mSubmeshData[submeshIndex].fallbackSkinningDirty == false);

		if (!mSubmeshData[submeshIndex].userSpecifiedData)
		{
			distributeFallbackData(submeshIndex);
			mOneUserVertexBufferChanged = true;
		}
	}
}



void ApexRenderMeshActor::distributeFallbackData(PxU32 submeshIndex)
{
	const NxVertexBuffer& vertexBuffer = mRenderMeshAsset->getSubmesh(submeshIndex).getVertexBuffer();
	const NxVertexFormat& format = vertexBuffer.getFormat();
	const PxU32 bufferCount = format.getBufferCount();
	const PxU32 vertexCount = vertexBuffer.getVertexCount();

	unsigned char* memoryIterator = (unsigned char*)mSubmeshData[submeshIndex].fallbackSkinningMemory;

	PxU32 sizeUsed = 0;
	for (PxU32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
	{
		if (format.getBufferAccess(bufferIndex) == NxRenderDataAccess::DYNAMIC)
		{
			NxRenderDataFormat::Enum bufferFormat = format.getBufferFormat(bufferIndex);
			NxRenderVertexSemantic::Enum bufferSemantic = format.getBufferSemantic(bufferIndex);

			if (bufferSemantic == NxRenderVertexSemantic::POSITION && bufferFormat == NxRenderDataFormat::FLOAT3)
			{
				mSubmeshData[submeshIndex].userPositions = (PxVec3*)memoryIterator;
				memoryIterator += sizeof(PxVec3) * vertexCount;
				sizeUsed += sizeof(PxVec3);
			}
			else if (bufferSemantic == NxRenderVertexSemantic::NORMAL && bufferFormat == NxRenderDataFormat::FLOAT3)
			{
				mSubmeshData[submeshIndex].userNormals = (PxVec3*)memoryIterator;
				memoryIterator += sizeof(PxVec3) * vertexCount;
				sizeUsed += sizeof(PxVec3);
			}
			else if (bufferSemantic == NxRenderVertexSemantic::TANGENT && bufferFormat == NxRenderDataFormat::FLOAT4)
			{
				mSubmeshData[submeshIndex].userTangents4 = (PxVec4*)memoryIterator;
				memoryIterator += sizeof(PxVec4) * vertexCount;
				sizeUsed += sizeof(PxVec4);
			}
		}
	}

	PX_ASSERT(sizeUsed * vertexCount == mSubmeshData[submeshIndex].fallbackSkinningMemorySize);
}



void ApexRenderMeshActor::updateFallbackSkinning(PxU32 submeshIndex)
{
	if (mSubmeshData[submeshIndex].fallbackSkinningMemory == NULL || mSubmeshData[submeshIndex].userSpecifiedData)
	{
		return;
	}

#if VERBOSE
	printf("updateFallbackSkinning(submeshIndex=%d)\n", submeshIndex);
#endif
	PX_PROFILER_PERF_SCOPE("ApexRenderMesh::updateFallbackSkinning");

	const NxVertexBuffer& vertexBuffer = mRenderMeshAsset->getSubmesh(submeshIndex).getVertexBuffer();
	const NxVertexFormat& format = vertexBuffer.getFormat();

	PxVec3* outPositions = mSubmeshData[submeshIndex].userPositions;
	PxVec3* outNormals = mSubmeshData[submeshIndex].userNormals;
	PxVec4* outTangents = mSubmeshData[submeshIndex].userTangents4;

	if (outPositions == NULL && outNormals == NULL && outTangents == NULL)
	{
		return;
	}

	NxRenderDataFormat::Enum inFormat;
	const PxU32 positionIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::POSITION));
	const PxVec3* inPositions = (const PxVec3*)vertexBuffer.getBufferAndFormat(inFormat, positionIndex);
	PX_ASSERT(inPositions == NULL || inFormat == NxRenderDataFormat::FLOAT3);

	if (inPositions != NULL && mSubmeshData[submeshIndex].staticPositionReplacement != NULL)
	{
		inPositions = mSubmeshData[submeshIndex].staticPositionReplacement;
	}

	const PxU32 normalIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::NORMAL));
	const PxVec3* inNormals = (const PxVec3*)vertexBuffer.getBufferAndFormat(inFormat, normalIndex);
	PX_ASSERT(inNormals == NULL || inFormat == NxRenderDataFormat::FLOAT3);

	const PxU32 tangentIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::TANGENT));
	const PxVec4* inTangents = (const PxVec4*)vertexBuffer.getBufferAndFormat(inFormat, tangentIndex);
	PX_ASSERT(inTangents == NULL || inFormat == NxRenderDataFormat::FLOAT4);

	const PxU32 boneIndexIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::BONE_INDEX));
	const PxU16* inBoneIndices = (const PxU16*)vertexBuffer.getBufferAndFormat(inFormat, boneIndexIndex);
	PX_ASSERT(inBoneIndices == NULL || inFormat == NxRenderDataFormat::USHORT1 || inFormat == NxRenderDataFormat::USHORT2 || inFormat == NxRenderDataFormat::USHORT3 || inFormat == NxRenderDataFormat::USHORT4);

	const PxU32 boneWeightIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::BONE_WEIGHT));
	const PxF32* inBoneWeights = (const PxF32*)vertexBuffer.getBufferAndFormat(inFormat, boneWeightIndex);
	PX_ASSERT(inBoneWeights == NULL || inFormat == NxRenderDataFormat::FLOAT1 || inFormat == NxRenderDataFormat::FLOAT2 || inFormat == NxRenderDataFormat::FLOAT3 || inFormat == NxRenderDataFormat::FLOAT4);

	PxU32 numBonesPerVertex = 0;
	switch (inFormat)
	{
	case NxRenderDataFormat::FLOAT1:
		numBonesPerVertex = 1;
		break;
	case NxRenderDataFormat::FLOAT2:
		numBonesPerVertex = 2;
		break;
	case NxRenderDataFormat::FLOAT3:
		numBonesPerVertex = 3;
		break;
	case NxRenderDataFormat::FLOAT4:
		numBonesPerVertex = 4;
		break;
	default:
		break;
	}

	PX_ASSERT((inPositions != NULL) == (outPositions != NULL));
	PX_ASSERT((inNormals != NULL) == (outNormals != NULL));
	PX_ASSERT((inTangents != NULL) == (outTangents != NULL));

	if (inBoneWeights == NULL || inBoneIndices == NULL || numBonesPerVertex == 0)
	{
		return;
	}

	// clear all data
	physx::PxMemSet(mSubmeshData[submeshIndex].fallbackSkinningMemory, 0, mSubmeshData[submeshIndex].fallbackSkinningMemorySize);

	const PxU32 vertexCount = vertexBuffer.getVertexCount();
	for (PxU32 i = 0; i < vertexCount; i++)
	{
		for (PxU32 k = 0; k < numBonesPerVertex; k++)
		{
			const PxU32 boneIndex = inBoneIndices[i * numBonesPerVertex + k];
			PX_ASSERT(boneIndex < mTransforms.size());
			PxMat34Legacy& transform = mTransforms[boneIndex];

			const PxF32 boneWeight = inBoneWeights[i * numBonesPerVertex + k];
			if (boneWeight > 0.0f)
			{
				if (outPositions != NULL)
				{
					outPositions[i] += transform * inPositions[i] * boneWeight;
				}

				if (outNormals != NULL)
				{
					outNormals[i] += transform.M * inNormals[i] * boneWeight;
				}

				if (outTangents != NULL)
				{
					outTangents[i] += PxVec4(transform.M * inTangents[i].getXYZ() * boneWeight, 0.0f);
				}
			}
		}
		if (outTangents != NULL)
		{
			outTangents[i].w = inTangents[i].w;
		}
	}

	mSubmeshData[submeshIndex].fallbackSkinningDirty = true;
}



void ApexRenderMeshActor::writeUserBuffers(PxU32 submeshIndex)
{
	PxVec3* outPositions = mSubmeshData[submeshIndex].userPositions;
	PxVec3* outNormals = mSubmeshData[submeshIndex].userNormals;
	PxVec4* outTangents4 = mSubmeshData[submeshIndex].userTangents4;

	if (outPositions == NULL && outNormals == NULL && outTangents4 == NULL)
	{
		return;
	}

	NxApexRenderVertexBufferData dynamicWriteData;
	if (outPositions != NULL)
	{
		dynamicWriteData.setSemanticData(NxRenderVertexSemantic::POSITION, outPositions, sizeof(PxVec3), NxRenderDataFormat::FLOAT3);
	}

	if (outNormals != NULL)
	{
		dynamicWriteData.setSemanticData(NxRenderVertexSemantic::NORMAL, outNormals, sizeof(PxVec3), NxRenderDataFormat::FLOAT3);
	}

	if (outTangents4)
	{
		dynamicWriteData.setSemanticData(NxRenderVertexSemantic::TANGENT, outTangents4, sizeof(PxVec4), NxRenderDataFormat::FLOAT4);
	}

	const PxU32 vertexCount = mRenderMeshAsset->mSubmeshes[submeshIndex]->getVertexBuffer().getVertexCount();

	mSubmeshData[submeshIndex].userDynamicVertexBuffer->writeBuffer(dynamicWriteData, 0, vertexCount);
}



void ApexRenderMeshActor::visualizeTangentSpace(NiApexRenderDebug& batcher, PxF32 normalScale, PxF32 tangentScale, PxF32 bitangentScale, PxMat33* scaledRotations, PxVec3* translations, PxU32 stride, PxU32 numberOfTransforms) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(batcher);
	PX_UNUSED(normalScale);
	PX_UNUSED(tangentScale);
	PX_UNUSED(bitangentScale);
	PX_UNUSED(scaledRotations);
	PX_UNUSED(translations);
	PX_UNUSED(stride);
	PX_UNUSED(numberOfTransforms);
#else

	if (normalScale <= 0.0f && tangentScale <= 0.0f && bitangentScale <= 0.0f)
	{
		return;
	}

	PxU32 debugColorRed = batcher.getDebugColor(DebugColors::Red);
	PxU32 debugColorGreen = batcher.getDebugColor(DebugColors::Green);
	PxU32 debugColorBlue = batcher.getDebugColor(DebugColors::Blue);

	batcher.pushRenderState();

	const PxU32 submeshCount = mRenderMeshAsset->getSubmeshCount();
	PX_ASSERT(mSubmeshData.size() == submeshCount);
	for (PxU32 submeshIndex = 0; submeshIndex < submeshCount; ++submeshIndex)
	{
		const PxVec3* positions = NULL;
		const PxVec3* normals = NULL;
		const PxVec3* tangents = NULL;
		const PxVec4* tangents4 = NULL;

		const PxU16* boneIndices = NULL;
		const PxF32* boneWeights = NULL;

		PxU32 numBonesPerVertex = 0;

		if (mSubmeshData[submeshIndex].userSpecifiedData || mSubmeshData[submeshIndex].fallbackSkinningMemory != NULL)
		{
			positions = mSubmeshData[submeshIndex].userPositions;
			normals = mSubmeshData[submeshIndex].userNormals;
			tangents4 = mSubmeshData[submeshIndex].userTangents4;
		}
		else
		{
			const NxVertexBuffer& vertexBuffer = mRenderMeshAsset->getSubmesh(submeshIndex).getVertexBuffer();
			const NxVertexFormat& format = vertexBuffer.getFormat();

			NxRenderDataFormat::Enum inFormat;
			const PxU32 positionIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::POSITION));
			positions = (const PxVec3*)vertexBuffer.getBufferAndFormat(inFormat, positionIndex);
			PX_ASSERT(positions == NULL || inFormat == NxRenderDataFormat::FLOAT3);

			if (positions != NULL && mSubmeshData[submeshIndex].staticPositionReplacement != NULL)
			{
				positions = mSubmeshData[submeshIndex].staticPositionReplacement;
			}

			const PxU32 normalIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::NORMAL));
			normals = (const PxVec3*)vertexBuffer.getBufferAndFormat(inFormat, normalIndex);
			PX_ASSERT(normals == NULL || inFormat == NxRenderDataFormat::FLOAT3);

			const PxU32 tangentIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::TANGENT));
			tangents = (const PxVec3*)vertexBuffer.getBufferAndFormat(inFormat, tangentIndex);
			PX_ASSERT(tangents == NULL || inFormat == NxRenderDataFormat::FLOAT3 || inFormat == NxRenderDataFormat::FLOAT4);
			if (inFormat == NxRenderDataFormat::FLOAT4)
			{
				tangents4 = (const PxVec4*)tangents;
				tangents = NULL;
			}

			const PxU32 boneIndexIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::BONE_INDEX));
			boneIndices = (const PxU16*)vertexBuffer.getBufferAndFormat(inFormat, boneIndexIndex);
			PX_ASSERT(boneIndices == NULL || inFormat == NxRenderDataFormat::USHORT1 || inFormat == NxRenderDataFormat::USHORT2 || inFormat == NxRenderDataFormat::USHORT3 || inFormat == NxRenderDataFormat::USHORT4);

			switch (inFormat)
			{
			case NxRenderDataFormat::USHORT1:
				numBonesPerVertex = 1;
				break;
			case NxRenderDataFormat::USHORT2:
				numBonesPerVertex = 2;
				break;
			case NxRenderDataFormat::USHORT3:
				numBonesPerVertex = 3;
				break;
			case NxRenderDataFormat::USHORT4:
				numBonesPerVertex = 4;
				break;
			default:
				break;
			}

			const PxU32 boneWeightIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::BONE_WEIGHT));
			boneWeights = (const PxF32*)vertexBuffer.getBufferAndFormat(inFormat, boneWeightIndex);
			PX_ASSERT(boneWeights == NULL || inFormat == NxRenderDataFormat::FLOAT1 || inFormat == NxRenderDataFormat::FLOAT2 || inFormat == NxRenderDataFormat::FLOAT3 || inFormat == NxRenderDataFormat::FLOAT4);
		}

		const PxU32 partCount = visiblePartCount();
		const PxU32* visibleParts = getVisibleParts();
		for (PxU32 visiblePartIndex = 0; visiblePartIndex < partCount; visiblePartIndex++)
		{
			const PxU32 partIndex = visibleParts[visiblePartIndex];

			const NxRenderSubmesh& submesh = mRenderMeshAsset->getSubmesh(submeshIndex);
			const PxU32 vertexStart = submesh.getFirstVertexIndex(partIndex);
			const PxU32 vertexEnd = vertexStart + submesh.getVertexCount(partIndex);

			for (PxU32 i = vertexStart; i < vertexEnd; i++)
			{
				PxVec3 position(0.0f), tangent(0.0f), bitangent(0.0f), normal(0.0f);
				if (numBonesPerVertex == 0)
				{
					position = positions[i];
					if (normals != NULL)
					{
						normal = normals[i].getNormalized();
					}
					if (tangents4 != NULL)
					{
						tangent = tangents4[i].getXYZ().getNormalized();
						bitangent = normal.cross(tangent) * tangents4[i].w;
					}
					else if (tangents != NULL)
					{
						tangent = tangents[i].getNormalized();
						bitangent = normal.cross(tangent);
					}

				}
				else if (numBonesPerVertex == 1)
				{
					PX_ASSERT(boneIndices != NULL);
					PxU32 boneIndex = 0;
					if (mRenderWithoutSkinning)
					{
						boneIndex = 0;
					}
					else if (mKeepVisibleBonesPacked)
					{
						boneIndex = visiblePartIndex;
					}
					else
					{
						boneIndex = boneIndices[i];
					}

					const PxMat34Legacy& tm = mTransforms[boneIndex];
					position = tm * positions[i];
					if (normals != NULL)
					{
						normal = tm.M * normals[i].getNormalized();
					}
					if (tangents4 != NULL)
					{
						tangent = tm.M * tangents4[i].getXYZ().getNormalized();
						bitangent = normal.cross(tangent) * tangents4[i].w;
					}
					else if (tangents != NULL)
					{
						tangent = tm.M * tangents[i].getNormalized();
						bitangent = normal.cross(tangent);
					}
				}
				else
				{
					position = tangent = bitangent = normal = PxVec3(0.0f);
					for (PxU32 k = 0; k < numBonesPerVertex; k++)
					{
						const PxF32 weight = boneWeights[i * numBonesPerVertex + k];
						if (weight > 0.0f)
						{
							const PxMat34Legacy& tm = mTransforms[boneIndices[i * numBonesPerVertex + k]];
							position += tm * positions[i] * weight;
							if (normals != NULL)
							{
								normal += tm.M * normals[i] * weight;
							}
							if (tangents4 != NULL)
							{
								tangent += tm.M * tangents4[i].getXYZ() * weight;
							}
							else if (tangents != NULL)
							{
								tangent += tm.M * tangents[i] * weight;
							}
						}
					}
					normal.normalize();
					tangent.normalize();
					if (tangents4 != NULL)
					{
						bitangent = normal.cross(tangent) * tangents4[i].w;
					}
					else if (tangents != NULL)
					{
						bitangent = normal.cross(tangent);
					}
				}

				if (numberOfTransforms == 0 || scaledRotations == NULL || translations == NULL)
				{
					if (!tangent.isZero() && tangentScale > 0.0f)
					{
						batcher.setCurrentColor(debugColorRed);
						batcher.debugLine(position, position + tangent * tangentScale);
					}

					if (!bitangent.isZero() && bitangentScale > 0.0f)
					{
						batcher.setCurrentColor(debugColorGreen);
						batcher.debugLine(position, position + bitangent * bitangentScale);
					}

					if (!normal.isZero() && normalScale > 0.0f)
					{
						batcher.setCurrentColor(debugColorBlue);
						batcher.debugLine(position, position + normal * normalScale);
					}
				}
				else	//instancing
				{
					for (PxU32 k = 0; k < numberOfTransforms; k++)
					{
						PxMat33& scaledRotation = *(PxMat33*)((PxU8*)scaledRotations + k*stride);
						PxVec3& translation = *(PxVec3*)((PxU8*)translations + k*stride);

						PxVec3 newPos = scaledRotation.transform(position) + translation;	//full transform

						PxVec3 newTangent = scaledRotation.transform(tangent);	//without translation
						PxVec3 newBitangent = scaledRotation.transform(bitangent);

						PxVec3 newNormal = (scaledRotation.getInverse()).getTranspose().transform(normal);

						if (!tangent.isZero() && tangentScale > 0.0f)
						{
							batcher.setCurrentColor(debugColorRed);
							batcher.debugLine(newPos, newPos + newTangent * tangentScale);
						}

						if (!bitangent.isZero() && bitangentScale > 0.0f)
						{
							batcher.setCurrentColor(debugColorGreen);
							batcher.debugLine(newPos, newPos + newBitangent * bitangentScale);
						}

						if (!normal.isZero() && normalScale > 0.0f)
						{
							batcher.setCurrentColor(debugColorBlue);
							batcher.debugLine(newPos, newPos + newNormal * normalScale);
						}
					}
				}
			}
		}

	}

	batcher.popRenderState();
#endif
}




ApexRenderMeshActor::SubmeshData::SubmeshData() :
	indexBuffer(NULL),
	fallbackSkinningMemory(NULL),
	userDynamicVertexBuffer(NULL),
	instanceBuffer(NULL),
	userPositions(NULL),
	userNormals(NULL),
	userTangents4(NULL),
	staticColorReplacement(NULL),
	staticColorReplacementDirty(false),
	staticPositionReplacement(NULL),
	staticBufferReplacement(NULL),
	dynamicBufferReplacement(NULL),
	fallbackSkinningMemorySize(0),
	visibleTriangleCount(0),
	materialID(INVALID_RESOURCE_ID),
	material(NULL),
	isMaterialPointerValid(false),
	maxBonesPerMaterial(0),
	indexBufferRequestedSize(0),
	userSpecifiedData(false),
	userVertexBufferAlwaysDirty(false),
	userIndexBufferChanged(false),
	fallbackSkinningDirty(false)
{
}



ApexRenderMeshActor::SubmeshData::~SubmeshData()
{
	if (fallbackSkinningMemory != NULL)
	{
		PX_FREE(fallbackSkinningMemory);
		fallbackSkinningMemory = NULL;
	}
	fallbackSkinningMemorySize = 0;
}



void ApexRenderMeshActor::setTM(const physx::PxMat44& tm, physx::PxU32 boneIndex /* = 0 */)
{
	NX_WRITE_ZONE();
	PX_ASSERT(boneIndex < mRenderMeshAsset->getBoneCount());
	mBonePosesDirty = true;
	physx::PxMat34Legacy& boneTM = accessTM(boneIndex);
	boneTM.t = tm.getPosition();
	boneTM.M.setColumn(0, tm.column0.getXYZ());
	boneTM.M.setColumn(1, tm.column1.getXYZ());
	boneTM.M.setColumn(2, tm.column2.getXYZ());
}



void ApexRenderMeshActor::setTM(const physx::PxMat44& tm, const physx::PxVec3& scale, physx::PxU32 boneIndex /* = 0 */)
{
	// Assumes tm is pure rotation.  This can allow some optimization.
	NX_WRITE_ZONE();
	PX_ASSERT(boneIndex < mRenderMeshAsset->getBoneCount());
	mBonePosesDirty = true;
	physx::PxMat34Legacy& boneTM = accessTM(boneIndex);
	boneTM.t = tm.getPosition();
	boneTM.M.setColumn(0, tm.column0.getXYZ()*scale.x);
	boneTM.M.setColumn(1, tm.column1.getXYZ()*scale.y);
	boneTM.M.setColumn(2, tm.column2.getXYZ()*scale.z);
}


void ApexRenderMeshActor::setLastFrameTM(const physx::PxMat44& tm, physx::PxU32 boneIndex /* = 0 */)
{
	if (!mPreviousFrameBoneBufferValid)
	{
		return;
	}

	PX_ASSERT(boneIndex < mRenderMeshAsset->getBoneCount());
	physx::PxMat34Legacy& boneTM = accessLastFrameTM(boneIndex);
	boneTM.t = tm.getPosition();
	boneTM.M.setColumn(0, tm.column0.getXYZ());
	boneTM.M.setColumn(1, tm.column1.getXYZ());
	boneTM.M.setColumn(2, tm.column2.getXYZ());
}



void ApexRenderMeshActor::setLastFrameTM(const physx::PxMat44& tm, const physx::PxVec3& scale, physx::PxU32 boneIndex /* = 0 */)
{
	if (!mPreviousFrameBoneBufferValid)
	{
		return;
	}

	// Assumes tm is pure rotation.  This can allow some optimization.

	PX_ASSERT(boneIndex < mRenderMeshAsset->getBoneCount());
	physx::PxMat34Legacy& boneTM = accessLastFrameTM(boneIndex);
	boneTM.t = tm.getPosition();
	boneTM.M.setColumn(0, tm.column0.getXYZ()*scale.x);
	boneTM.M.setColumn(1, tm.column1.getXYZ()*scale.y);
	boneTM.M.setColumn(2, tm.column2.getXYZ()*scale.z);
}


void ApexRenderMeshActor::setSkinningMode(NxRenderMeshActorSkinningMode::Enum mode)
{
	if (mode >= NxRenderMeshActorSkinningMode::Default && mode < NxRenderMeshActorSkinningMode::Count)
	{
		mSkinningMode = mode;
	}
}

NxRenderMeshActorSkinningMode::Enum ApexRenderMeshActor::getSkinningMode() const
{
	return mSkinningMode;
}


void ApexRenderMeshActor::syncVisibility(bool useLock)
{
	NX_WRITE_ZONE();
	if (mApiVisibilityChanged && mBufferVisibility)
	{
		if (useLock)
		{
			lockRenderResources();
		}
		mVisiblePartsForRendering.resize(mVisiblePartsForAPI.usedCount());
		memcpy(mVisiblePartsForRendering.begin(), mVisiblePartsForAPI.usedIndices(), mVisiblePartsForAPI.usedCount()*sizeof(PxU32));
		const PxU32 swapBufferSize = mTMSwapBuffer.size();
		for (PxU32 i = 0; i < swapBufferSize; ++i)
		{
			const PxU32 swapIndices = mTMSwapBuffer[i];
			physx::swap(mTransforms[swapIndices >> 16], mTransforms[swapIndices & 0xFFFF]);
		}
		mTMSwapBuffer.reset();
		mPartVisibilityChanged = true;
		mApiVisibilityChanged = false;
		if (useLock)
		{
			unlockRenderResources();
		}
	}
}

// TODO - LRR - update part bounds actor bounds to work with >1 bones per part
void ApexRenderMeshActor::updateBounds()
{
	mRenderBounds.setEmpty();
	const PxU32* visiblePartIndexPtr = mVisiblePartsForAPI.usedIndices();
	const PxU32* visiblePartIndexPtrStop = visiblePartIndexPtr + mVisiblePartsForAPI.usedCount();
	if (mTransforms.size() < mRenderMeshAsset->getPartCount())
	{
		// BRG - for static meshes.  We should create a mapping for more generality.
		PX_ASSERT(mTransforms.size() == 1);
		physx::PxMat34Legacy& tm = accessTM();
		while (visiblePartIndexPtr < visiblePartIndexPtrStop)
		{
			const PxU32 partIndex = *visiblePartIndexPtr++;
			PxBounds3 partBounds = mRenderMeshAsset->getBounds(partIndex);
			PxBounds3Transform(partBounds, tm);
			mRenderBounds.include(partBounds);
		}
	}
	else
	{
		while (visiblePartIndexPtr < visiblePartIndexPtrStop)
		{
			const PxU32 partIndex = *visiblePartIndexPtr++;
			PxBounds3 partBounds = mRenderMeshAsset->getBounds(partIndex);
			PxMat34Legacy& tm = accessTM(partIndex);
			PxBounds3Transform(partBounds, tm);
			mRenderBounds.include(partBounds);
		}
	}
}

void ApexRenderMeshActor::updateInstances(PxU32 submeshIndex)
{
	PX_PROFILER_PERF_SCOPE("ApexRenderMesh::updateInstances");

	for (PxU32 i = 0; i < mSubmeshData[submeshIndex].renderResources.size(); ++i)
	{
		NxUserRenderResource* renderResource = mSubmeshData[submeshIndex].renderResources[i].resource;
		renderResource->setInstanceBufferRange(mInstanceOffset, mInstanceCount);
	}
}

void ApexRenderMeshActor::setReleaseResourcesIfNothingToRender(bool value)
{
	NX_WRITE_ZONE();
	mReleaseResourcesIfNothingToRender = value;
}

void ApexRenderMeshActor::setBufferVisibility(bool bufferVisibility)
{
	NX_WRITE_ZONE();
	mBufferVisibility = bufferVisibility;
	mPartVisibilityChanged = true;
}

void ApexRenderMeshActor::setOverrideMaterial(PxU32 index, const char* overrideMaterialName)
{
	NX_WRITE_ZONE();
	NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
	if (nrp != NULL && index < mSubmeshData.size())
	{
		// do create before release, so we don't release the resource if the newID is the same as the old
		NxResID materialNS = NiGetApexSDK()->getMaterialNameSpace();

		NxResID newID = nrp->createResource(materialNS, overrideMaterialName);
		nrp->releaseResource(mSubmeshData[index].materialID);

		mSubmeshData[index].materialID = newID;
		mSubmeshData[index].material = NULL;
		mSubmeshData[index].isMaterialPointerValid = false;
		mSubmeshData[index].maxBonesPerMaterial = 0;

		if (!NiGetApexSDK()->getRMALoadMaterialsLazily())
		{
			loadMaterial(mSubmeshData[index]);
		}
	}
}

// Need an inverse
PX_INLINE physx::PxMat44 inverse(const physx::PxMat44& m)
{
	const physx::PxMat33 invM33 = physx::PxMat33(m.getBasis(0), m.getBasis(1), m.getBasis(2)).getInverse();
	return PxMat44(invM33, -(invM33.transform(m.getPosition())));
}

bool ApexRenderMeshActor::rayCast(NxRenderMeshActorRaycastHitData& hitData,
                                  const physx::PxVec3& worldOrig, const physx::PxVec3& worldDisp,
                                  NxRenderMeshActorRaycastFlags::Enum flags,
                                  NxRenderCullMode::Enum winding,
                                  physx::PxI32 partIndex) const
{
	NX_READ_ZONE();
	PX_ASSERT(mRenderMeshAsset != NULL);
	PX_ASSERT(worldOrig.isFinite() && worldDisp.isFinite()  && !worldDisp.isZero());

	// Come up with a part range which matches the flags, and if partIndex > 0, ensure it lies within the part range
	PxU32 rankStart = (flags & NxRenderMeshActorRaycastFlags::VISIBLE_PARTS) != 0 ? 0 : mVisiblePartsForAPI.usedCount();
	PxU32 rankStop = (flags & NxRenderMeshActorRaycastFlags::INVISIBLE_PARTS) != 0 ? mRenderMeshAsset->getPartCount() : mVisiblePartsForAPI.usedCount();
	// We use the visibility index bank, since it holds visible and invisible parts contiguously
	if (rankStart >= rankStop)
	{
		return false;	// No parts selected for raycast
	}
	if (partIndex >= 0)
	{
		const PxU32 partRank = mVisiblePartsForAPI.getRank((PxU32)partIndex);
		if (partRank < rankStart || partRank >= rankStop)
		{
			return false;
		}
		rankStart = partRank;
		rankStop = partRank + 1;
	}
	const PxU32* partIndices = mVisiblePartsForAPI.usedIndices();

	// Allocate an inverse transform and local ray for each part and calculate them
	const PxU32 tmCount = mRenderWithoutSkinning ? 1 : rankStop - rankStart;	// Only need one transform if not skinning

	PX_ALLOCA(invTMs, PxMat44, tmCount);
	PX_ALLOCA(localOrigs, PxVec3, tmCount);
	PX_ALLOCA(localDisps, PxVec3, tmCount);

	if (mRenderWithoutSkinning)
	{
		invTMs[0] = inverse(mTransforms[0]);
		localOrigs[0] = invTMs[0].transform(worldOrig);
		localDisps[0] = invTMs[0].rotate(worldDisp);
	}
	else
	{
		for (physx::PxU32 partRank = rankStart; partRank < rankStop; ++partRank)
		{
			invTMs[partRank - rankStart] = inverse(mTransforms[partRank - rankStart]);
			localOrigs[partRank - rankStart] = invTMs[partRank - rankStart].transform(worldOrig);
			localDisps[partRank - rankStart] = invTMs[partRank - rankStart].rotate(worldDisp);
		}
	}

	// Side "discriminant" - used to reduce branches in inner loops
	const physx::PxF32 disc = winding == NxRenderCullMode::CLOCKWISE ? 1.0f : (winding == NxRenderCullMode::COUNTER_CLOCKWISE ? -1.0f : 0.0f);

	// Keeping hit time as a fraction
	physx::PxF32 tNum = -1.0f;
	physx::PxF32 tDen = 0.0f;

	// To do: handle multiple-weighted vertices, and other cases where the number of parts does not equal the number of bones (besides non-skinned, which we do handle)
//	if (single-weighted vertices)
	{
		// Traverse the selected parts:
		const PxU32 submeshCount = mRenderMeshAsset->getSubmeshCount();
		for (PxU32 submeshIndex = 0; submeshIndex < submeshCount; ++submeshIndex)
		{
			const NxRenderSubmesh& submesh = mRenderMeshAsset->getSubmesh(submeshIndex);
			const NxVertexBuffer& vertexBuffer = submesh.getVertexBuffer();
			const NxVertexFormat& vertexFormat = vertexBuffer.getFormat();
			NxRenderDataFormat::Enum positionFormat;
			const PxVec3* vertexPositions = (const physx::PxVec3*)vertexBuffer.getBufferAndFormat(positionFormat, (physx::PxU32)vertexFormat.getBufferIndexFromID(vertexFormat.getSemanticID(physx::NxRenderVertexSemantic::POSITION)));
			if (positionFormat != NxRenderDataFormat::FLOAT3)
			{
				continue;	// Not handling any position format other than FLOAT3
			}
			for (physx::PxU32 partRank = rankStart; partRank < rankStop; ++partRank)
			{
				const physx::PxU32 cachedLocalIndex = mRenderWithoutSkinning ? 0 : partRank - rankStart;
				const physx::PxVec3& localOrig = localOrigs[cachedLocalIndex];
				const physx::PxVec3& localDisp = localDisps[cachedLocalIndex];
				const physx::PxU32 partIndex = partIndices[partRank];
				const physx::PxU32* ib = submesh.getIndexBuffer(partIndex);
				const physx::PxU32* ibStop = ib + submesh.getIndexCount(partIndex);
				PX_ASSERT(submesh.getIndexCount(partIndex) % 3 == 0);
				for (; ib < ibStop; ib += 3)
				{
					const physx::PxVec3 offsetVertices[3] = { vertexPositions[ib[0]] - localOrig, vertexPositions[ib[1]] - localOrig, vertexPositions[ib[2]] - localOrig };
					const physx::PxVec3 triangleNormal = (offsetVertices[1] - offsetVertices[0]).cross(offsetVertices[2] - offsetVertices[0]);
					const physx::PxF32 den = triangleNormal.dot(localDisp);
					if (den > -PX_EPS_F32 * PX_EPS_F32)
					{
						// Ray misses plane (or is too near parallel)
						continue;
					}
					const physx::PxF32 sides[3] = { (offsetVertices[0].cross(offsetVertices[1])).dot(localDisp), (offsetVertices[1].cross(offsetVertices[2])).dot(localDisp), (offsetVertices[2].cross(offsetVertices[0])).dot(localDisp) };
					if ((int)(sides[0]*disc > 0.0f) | (int)(sides[1]*disc > 0.0f) | (int)(sides[2]*disc > 0.0f))
					{
						// Ray misses triangle
						continue;
					}
					// Ray has hit the triangle; calculate time of intersection
					const physx::PxF32 num = offsetVertices[0].dot(triangleNormal);
					// Since den and tDen both have the same (negative) sign, this is equivalent to : if (num/den < tNum/tDen)
					if (num * tDen < tNum * den)
					{
						// This intersection is earliest
						tNum = num;
						tDen = den;
						hitData.partIndex = partIndex;
						hitData.submeshIndex = submeshIndex;
						hitData.vertexIndices[0] = ib[0];
						hitData.vertexIndices[1] = ib[1];
						hitData.vertexIndices[2] = ib[2];
					}
				}
			}
		}

		if (tDen == 0.0f)
		{
			// No intersection found
			return false;
		}

		// Found a triangle.  Fill in hit data
		hitData.time = tNum / tDen;

		// See if normal, tangent, or binormal can be found
		const NxRenderSubmesh& submesh = mRenderMeshAsset->getSubmesh(hitData.submeshIndex);
		const NxVertexBuffer& vertexBuffer = submesh.getVertexBuffer();
		const NxVertexFormat& vertexFormat = vertexBuffer.getFormat();

		const PxI32 normalBufferIndex = vertexFormat.getBufferIndexFromID(vertexFormat.getSemanticID(physx::NxRenderVertexSemantic::NORMAL));
		const PxI32 tangentBufferIndex = vertexFormat.getBufferIndexFromID(vertexFormat.getSemanticID(physx::NxRenderVertexSemantic::TANGENT));
		const PxI32 binormalBufferIndex = vertexFormat.getBufferIndexFromID(vertexFormat.getSemanticID(physx::NxRenderVertexSemantic::BINORMAL));

		NxExplicitRenderTriangle triangle;
		const bool haveNormal = vertexBuffer.getBufferData(&triangle.vertices[0].normal, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)normalBufferIndex, hitData.vertexIndices[0], 1);
		const bool haveTangent = vertexBuffer.getBufferData(&triangle.vertices[0].tangent, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)tangentBufferIndex, hitData.vertexIndices[0], 1);
		const bool haveBinormal = vertexBuffer.getBufferData(&triangle.vertices[0].binormal, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)binormalBufferIndex, hitData.vertexIndices[0], 1);

		physx::PxU32 fieldMask = 0;

		if (haveNormal)
		{
			vertexBuffer.getBufferData(&triangle.vertices[1].normal, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)normalBufferIndex, hitData.vertexIndices[1], 1);
			vertexBuffer.getBufferData(&triangle.vertices[2].normal, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)normalBufferIndex, hitData.vertexIndices[2], 1);
			fieldMask |= 1 << TriangleFrame::Normal_x | 1 << TriangleFrame::Normal_y | 1 << TriangleFrame::Normal_z;
		}
		else
		{
			hitData.normal = physx::PxVec3(0.0f);
		}

		if (haveTangent)
		{
			vertexBuffer.getBufferData(&triangle.vertices[1].tangent, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)tangentBufferIndex, hitData.vertexIndices[1], 1);
			vertexBuffer.getBufferData(&triangle.vertices[2].tangent, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)tangentBufferIndex, hitData.vertexIndices[2], 1);
			fieldMask |= 1 << TriangleFrame::Tangent_x | 1 << TriangleFrame::Tangent_y | 1 << TriangleFrame::Tangent_z;
		}
		else
		{
			hitData.tangent = physx::PxVec3(0.0f);
		}

		if (haveBinormal)
		{
			vertexBuffer.getBufferData(&triangle.vertices[1].binormal, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)binormalBufferIndex, hitData.vertexIndices[1], 1);
			vertexBuffer.getBufferData(&triangle.vertices[2].binormal, physx::NxRenderDataFormat::FLOAT3, 0, (physx::PxU32)binormalBufferIndex, hitData.vertexIndices[2], 1);
			fieldMask |= 1 << TriangleFrame::Binormal_x | 1 << TriangleFrame::Binormal_y | 1 << TriangleFrame::Binormal_z;
		}
		else
		{
			hitData.binormal = physx::PxVec3(0.0f);
		}

		if (fieldMask != 0)
		{
			// We know the positions are in the correct format from the check in the raycast
			const physx::PxVec3* vertexPositions = (const physx::PxVec3*)vertexBuffer.getBuffer(
				(physx::PxU32)vertexFormat.getBufferIndexFromID(vertexFormat.getSemanticID(physx::NxRenderVertexSemantic::POSITION)));
			triangle.vertices[0].position = vertexPositions[hitData.vertexIndices[0]];
			triangle.vertices[1].position = vertexPositions[hitData.vertexIndices[1]];
			triangle.vertices[2].position = vertexPositions[hitData.vertexIndices[2]];
			TriangleFrame frame(triangle, fieldMask);

			// Find the local hit position
			const PxU32 partRank = mVisiblePartsForAPI.getRank(hitData.partIndex);
			const PxU32 cachedLocalIndex = mRenderWithoutSkinning ? 0 : partRank - rankStart;
			const PxMat44& tm = mTransforms[mRenderWithoutSkinning ? 0 : hitData.partIndex];

			NxVertex v;
			v.position = localOrigs[cachedLocalIndex] + hitData.time * localDisps[cachedLocalIndex];
			frame.interpolateVertexData(v);
			if (haveNormal)
			{
				hitData.normal = invTMs[cachedLocalIndex].getTranspose().rotate(v.normal);
				hitData.normal.normalize();
			}

			if (haveTangent)
			{
				hitData.tangent = tm.rotate(v.tangent);
				hitData.tangent.normalize();
			}

			if (haveBinormal)
			{
				hitData.binormal = tm.rotate(v.binormal);
				hitData.binormal.normalize();
			}
			else
			{
				if (haveNormal && haveTangent)
				{
					hitData.binormal = hitData.normal.cross(hitData.tangent);
					hitData.binormal.normalize();
				}
			}
		}

		return true;
	}
}

void ApexRenderMeshActor::visualize(NiApexRenderDebug& batcher, physx::apex::DebugRenderParams* debugParams, PxMat33* scaledRotations, PxVec3* translations, PxU32 stride, PxU32 numberOfTransforms) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(batcher);
	PX_UNUSED(debugParams);
	PX_UNUSED(scaledRotations);
	PX_UNUSED(translations);
	PX_UNUSED(stride);
	PX_UNUSED(numberOfTransforms);
#else
	PX_ASSERT(&batcher != NULL);
	if ( !mEnableDebugVisualization ) return;

	// This implementation seems to work for destruction and clothing!
	const PxF32 scale = debugParams->Scale;
	visualizeTangentSpace(batcher, debugParams->RenderNormals * scale, debugParams->RenderTangents * scale, debugParams->RenderBitangents * scale, scaledRotations, translations, stride, numberOfTransforms);
#endif
}

} // namespace apex
} // namespace physx
