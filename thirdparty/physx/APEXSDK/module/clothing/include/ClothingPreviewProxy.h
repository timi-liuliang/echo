/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_PREVIEW_PROXY_H
#define CLOTHING_PREVIEW_PROXY_H

#include "NxClothingPreview.h"
#include "ClothingActor.h"
#include "NxApexRenderable.h"
#include "ApexRWLockable.h"
#include "WriteCheck.h"
#include "ReadCheck.h"

namespace physx
{
namespace apex
{
namespace clothing
{

class ClothingPreviewProxy : public NxClothingPreview, public physx::UserAllocated, public NxApexResource, public ApexRWLockable
{
	ClothingActor impl;

public:
	APEX_RW_LOCKABLE_BOILERPLATE
#pragma warning( disable : 4355 ) // disable warning about this pointer in argument list
	ClothingPreviewProxy(const NxParameterized::Interface& desc, ClothingAsset* asset, NxResourceList* list) :
		impl(desc, NULL, this, asset, NULL)
	{
		list->add(*this);
	}

	virtual void release()
	{
		impl.release();
	}

	virtual physx::PxU32 getListIndex() const
	{
		return impl.m_listIndex;
	}

	virtual void setListIndex(class NxResourceList& list, physx::PxU32 index)
	{
		impl.m_list = &list;
		impl.m_listIndex = index;
	}

	virtual void setPose(const physx::PxMat44& pose)
	{
		NX_WRITE_ZONE();
		impl.updateState(pose, NULL, 0, 0, ClothingTeleportMode::Continuous);
	}

	virtual const physx::PxMat44 getPose() const
	{
		NX_READ_ZONE();
		return impl.getGlobalPose();
	}

	virtual void lockRenderResources()
	{
		impl.lockRenderResources();
	}

	virtual void unlockRenderResources()
	{
		impl.unlockRenderResources();
	}

	virtual void updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0)
	{
		NxApexRenderable* renderable = impl.getRenderable();
		if (renderable != NULL)
		{
			renderable->updateRenderResources(rewriteBuffers, userRenderData);
		}
	}

	virtual void dispatchRenderResources(NxUserRenderer& renderer)
	{
		NX_WRITE_ZONE();
		NxApexRenderable* renderable = impl.getRenderable();
		if (renderable != NULL)
		{
			renderable->dispatchRenderResources(renderer);
		}
	}

	virtual physx::PxBounds3 getBounds() const
	{
		NX_READ_ZONE();
		return impl.getBounds();
	}


	virtual void updateState(const physx::PxMat44& globalPose, const physx::PxMat44* newBoneMatrices, physx::PxU32 boneMatricesByteStride, physx::PxU32 numBoneMatrices)
	{
		NX_WRITE_ZONE();
		impl.updateState(globalPose, newBoneMatrices, boneMatricesByteStride, numBoneMatrices, ClothingTeleportMode::Continuous);
	}

	void destroy()
	{
		impl.destroy();
		delete this;
	}

	virtual ~ClothingPreviewProxy() {}
};

}
} // namespace apex
} // namespace physx

#endif // CLOTHING_PREVIEW_PROXY_H
