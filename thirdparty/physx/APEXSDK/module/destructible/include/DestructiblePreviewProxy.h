/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEPREVIEW_PROXY_H__
#define __DESTRUCTIBLEPREVIEW_PROXY_H__

#include "NxApex.h"
#include "NxDestructiblePreview.h"
#include "DestructiblePreview.h"
#include "PsUserAllocated.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{
namespace destructible
{

class DestructiblePreviewProxy : public NxDestructiblePreview, public NxApexResource, public physx::UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructiblePreview impl;

#pragma warning(disable : 4355) // disable warning about this pointer in argument list
	DestructiblePreviewProxy(DestructibleAsset& asset, NxResourceList& list, const NxParameterized::Interface* params)
		: impl(this, asset, params)
	{
		list.add(*this);
	}

	virtual ~DestructiblePreviewProxy()
	{
	}

	// NxApexAssetPreview methods
	virtual void setPose(const physx::PxMat44& pose)
	{
		NX_WRITE_ZONE();
		impl.setPose(pose);
	}

	virtual const physx::PxMat44 getPose() const
	{
		NX_READ_ZONE();
		return impl.getPose();
	}

	// NxDestructiblePreview methods

	virtual const NxRenderMeshActor* getRenderMeshActor() const
	{
		NX_READ_ZONE();
		return const_cast<const NxRenderMeshActor*>(impl.getRenderMeshActor());
	}

	virtual void setExplodeView(physx::PxU32 depth, physx::PxF32 explode)
	{
		NX_WRITE_ZONE();
		return impl.setExplodeView(depth, explode);
	}

	// NxApexInterface methods
	virtual void release()
	{
		NX_WRITE_ZONE();
		impl.release();
		delete this;
	}
	virtual void destroy()
	{
		impl.destroy();
	}

	// NxApexRenderable methods
	virtual void updateRenderResources(bool rewriteBuffers, void* userRenderData)
	{
		URR_SCOPE;
		impl.updateRenderResources(rewriteBuffers, userRenderData);
	}
	virtual void dispatchRenderResources(NxUserRenderer& api)
	{
		impl.dispatchRenderResources(api);
	}
	virtual physx::PxBounds3 getBounds() const
	{
		return impl.getBounds();
	}
	virtual void lockRenderResources()
	{
		impl.ApexRenderable::renderDataLock();
	}
	virtual void unlockRenderResources()
	{
		impl.ApexRenderable::renderDataUnLock();
	}

	// NxApexResource methods
	virtual void	setListIndex(NxResourceList& list, physx::PxU32 index)
	{
		impl.m_listIndex = index;
		impl.m_list = &list;
	}
	virtual physx::PxU32	getListIndex() const
	{
		return impl.m_listIndex;
	}
};

}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLEPREVIEW_PROXY_H__
