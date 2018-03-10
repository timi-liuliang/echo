/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RENDER_RESOURCE_MANAGER_WRAPPER_H
#define RENDER_RESOURCE_MANAGER_WRAPPER_H

#include "ApexSDK.h"

namespace physx
{
namespace apex
{


class RenderResourceManagerWrapper : public NxUserRenderResourceManager, public UserAllocated
{
	PX_NOCOPY(RenderResourceManagerWrapper);
public:
	RenderResourceManagerWrapper(NxUserRenderResourceManager& rrm) :
	  mRrm(rrm)
	{
	}


	virtual NxUserRenderVertexBuffer*   createVertexBuffer(const NxUserRenderVertexBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createVertexBuffer(desc);
	}

	virtual void                        releaseVertexBuffer(NxUserRenderVertexBuffer& buffer)
	{
		mRrm.releaseVertexBuffer(buffer);
	}

	virtual NxUserRenderIndexBuffer*    createIndexBuffer(const NxUserRenderIndexBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createIndexBuffer(desc);
	}

	virtual void                        releaseIndexBuffer(NxUserRenderIndexBuffer& buffer) 
	{
		mRrm.releaseIndexBuffer(buffer);
	}

	virtual NxUserRenderBoneBuffer*     createBoneBuffer(const NxUserRenderBoneBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createBoneBuffer(desc);
	}

	virtual void                        releaseBoneBuffer(NxUserRenderBoneBuffer& buffer)
	{
		mRrm.releaseBoneBuffer(buffer);
	}

	virtual NxUserRenderInstanceBuffer* createInstanceBuffer(const NxUserRenderInstanceBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createInstanceBuffer(desc);
	}

	virtual void                        releaseInstanceBuffer(NxUserRenderInstanceBuffer& buffer)
	{
		mRrm.releaseInstanceBuffer(buffer);
	}

	virtual NxUserRenderSpriteBuffer*   createSpriteBuffer(const NxUserRenderSpriteBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createSpriteBuffer(desc);
	}

	virtual void                        releaseSpriteBuffer(NxUserRenderSpriteBuffer& buffer)
	{
		mRrm.releaseSpriteBuffer(buffer);
	}

	virtual NxUserRenderSurfaceBuffer*  createSurfaceBuffer(const NxUserRenderSurfaceBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createSurfaceBuffer(desc);
	}

	virtual void                        releaseSurfaceBuffer(NxUserRenderSurfaceBuffer& buffer)
	{
		mRrm.releaseSurfaceBuffer(buffer);
	}

	virtual NxUserRenderResource*       createResource(const NxUserRenderResourceDesc& desc)
	{
		URR_CHECK;
		return mRrm.createResource(desc);
	}

	virtual void                        releaseResource(NxUserRenderResource& resource)
	{
		mRrm.releaseResource(resource);
	}

	virtual physx::PxU32                       getMaxBonesForMaterial(void* material)
	{
		return mRrm.getMaxBonesForMaterial(material);
	}

	virtual bool getSpriteLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, NxUserRenderSpriteBufferDesc* textureDescArray)
	{
		return mRrm.getSpriteLayoutData(spriteCount, spriteSemanticsBitmap, textureDescArray);
	}

	virtual bool getInstanceLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, NxUserRenderInstanceBufferDesc* instanceDescArray)
	{
		return mRrm.getInstanceLayoutData(spriteCount, spriteSemanticsBitmap, instanceDescArray);
	}

private:
	NxUserRenderResourceManager& mRrm;
};


}
} // end namespace physx::apex


#endif // RENDER_RESOURCE_MANAGER_WRAPPER_H
