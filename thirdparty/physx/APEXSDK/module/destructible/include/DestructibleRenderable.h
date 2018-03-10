/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLERENDERABLE_H__
#define __DESTRUCTIBLERENDERABLE_H__

#include "NxApex.h"
#include "NxDestructibleActor.h"
#include "NxDestructibleRenderable.h"
#include "ApexInterface.h"
#include "ApexRWLockable.h"
#include "ApexActor.h"
#if APEX_RUNTIME_FRACTURE
#include "Renderable.h"
#endif

#include "ReadCheck.h"

namespace physx
{
namespace apex
{

class NxRenderMeshActor;

namespace destructible
{

class DestructibleActor;

class DestructibleRenderable : public NxDestructibleRenderable, public ApexRenderable, public physx::UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructibleRenderable(NxRenderMeshActor* renderMeshActors[NxDestructibleActorMeshType::Count], DestructibleAsset* asset, PxI32 listIndex);
	~DestructibleRenderable();

	// Begin NxDestructibleRenderable methods
	virtual NxRenderMeshActor*	getRenderMeshActor(NxDestructibleActorMeshType::Enum type = NxDestructibleActorMeshType::Skinned) const
	{
		NX_READ_ZONE();
		return (physx::PxU32)type < NxDestructibleActorMeshType::Count ? mRenderMeshActors[type] : NULL;
	}

	virtual void				release();
	// End NxDestructibleRenderable methods

	// Begin NxApexRenderable methods
	virtual	void				updateRenderResources(bool rewriteBuffers, void* userRenderData);

	virtual	void				dispatchRenderResources(NxUserRenderer& api);

	virtual	PxBounds3			getBounds() const
	{
		PxBounds3 bounds = ApexRenderable::getBounds();
#if APEX_RUNTIME_FRACTURE
		bounds.include(mRTrenderable.getBounds());
#endif
		return bounds;
	}

	virtual	void				lockRenderResources()
	{
		ApexRenderable::renderDataLock();
	}

	virtual	void				unlockRenderResources()
	{
		ApexRenderable::renderDataUnLock();
	}
	// End NxApexRenderable methods

	// Begin DestructibleRenderable methods
	// Returns this if successful, NULL otherwise
	DestructibleRenderable*		incrementReferenceCount();

	PxI32						getReferenceCount()
	{
		return mRefCount;
	}

	void						setBounds(const physx::PxBounds3& bounds)
	{
		mRenderBounds = bounds;
	}
	// End DestructibleRenderable methods

#if APEX_RUNTIME_FRACTURE
	::physx::fracture::Renderable& getRTrenderable() { return mRTrenderable; }
#endif

private:

	NxRenderMeshActor*	mRenderMeshActors[NxDestructibleActorMeshType::Count];	// Indexed by NxDestructibleActorMeshType::Enum
	DestructibleAsset*	mAsset;
	PxI32				mListIndex;
	volatile PxI32		mRefCount;
#if APEX_RUNTIME_FRACTURE
	::physx::fracture::Renderable mRTrenderable;
#endif
};

}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLERENDERABLE_H__
